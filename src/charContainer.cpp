#include <SFML/Graphics.hpp>
#include <vector>
#include "charContainer.h"

void charContainer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform(); // Apply the transform of the container

    for (const auto& letter_shape : letter_shapes) {
        target.draw(letter_shape, states);
    }
}

// Custom getLocalBounds method for custom Transformable class. Used for determining spacing between letters to prevent overlap
sf::FloatRect charContainer::getLocalBounds() const {
    if (letter_shapes.empty()) {
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
    }

    float min_x = std::numeric_limits<float>::max();
    float max_x = -std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float max_y = -std::numeric_limits<float>::max();

    bool hasVertices = false;

    // Get min and max values to build bounding box
    for (const auto& letter_shape : letter_shapes) {
        for (std::size_t i = 0; i < letter_shape.getVertexCount(); i++) {
            sf::Vector2f vertexPos = letter_shape[i].position;
            if (vertexPos.x < min_x) min_x = vertexPos.x;
            if (vertexPos.x > max_x) max_x = vertexPos.x;
            if (vertexPos.y < min_y) min_y = vertexPos.y;
            if (vertexPos.y > max_y) max_y = vertexPos.y;
            hasVertices = true;
        }
    }

    if (!hasVertices) {
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
    }

    return sf::FloatRect({min_x, min_y}, {max_x - min_x, max_y - min_y});
}

// Parse text from glyph file to get exact instructions to build glyph
std::vector<std::vector<std::string>> charContainer::textParser(std::string font) {
    std::ifstream file(font);
    
    // Checks if file exists
    if (!file.is_open()) {
        std::cerr << "Failed to open " << font << std::endl;
        return {};
    }

    // Will store all tokens in a 2D vector to keep track of lines
    std::vector<std::vector<std::string>> all_tokens;
    std::string line;

    // Parse file line by line
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> line_tokens;
        std::string token;

        // Separate all letters and numbers in the line by spaces. Push each one to the vector
        while (ss >> token) {
            line_tokens.push_back(token);
        }

        // Push the tokens from the current line into the master vector
        if (!line_tokens.empty()) {
            all_tokens.push_back(line_tokens);
        }
    }

    // Prevent memory leaks!
    file.close();

    return all_tokens;
}

// Calculate cubic bezier curve across 4 points
// B(t) = (1 - t)^3 * p0 + 3t(1 - t)^2 * p1 + 3t^2 * (1 - t) * p2 + t^3 * p3
sf::Vector2f charContainer::getBezierPoint(float t, sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3) {
    sf::Vector2f curve = ((1 - t) * (1 - t) * (1 - t)) * p0 + 3 * t * ((1 - t) * (1 - t)) * p1 + 3 * t * t * (1 - t) * p2 + t * t * t * p3;

    return curve;
}

// Use the tokens given by textParser and the formula from getBezierPoint to build the letter
charContainer charContainer::buildGlyph(std::string font, std::string glyph) {
    std::filesystem::path current_dir = std::filesystem::current_path();
    std::filesystem::path src_path;

    // Get out of exe's build folder to find the fonts directory
    while (current_dir.has_relative_path()) {
        if (std::filesystem::exists(current_dir / "fonts" / font)) {
            src_path = current_dir / "fonts" / font / glyph;
            break;
        }
        current_dir = current_dir.parent_path(); // Go up one directory level
    }

    // Parse file once found
    auto instructions = charContainer::textParser(src_path.string());

    charContainer letter_container;

    // Used to keep track of current position while drawing the curves
    sf::Vector2f start_point;
    sf::Vector2f current_point;

    for (const auto& line : instructions) {
        if (line.front() == "M") {

            // M determines a starting point
            start_point = sf::Vector2f(std::stof(line[1]), std::stof(line[2]));
            current_point = start_point;

            // Since this is a starting point, we create a new LineStrip to be a specific shape in the letter. Important for letters with more than one contour
            sf::VertexArray letter_shape(sf::PrimitiveType::LineStrip);
            letter_container.letter_shapes.push_back(letter_shape);
        } else if (line.front() == "C") {

            // C is for drawing a cubic bezier curve
            sf::Vector2f p1 = sf::Vector2f(std::stof(line[1]), std::stof(line[2]));
            sf::Vector2f p2 = sf::Vector2f(std::stof(line[3]), std::stof(line[4]));
            sf::Vector2f p3 = sf::Vector2f(std::stof(line[5]), std::stof(line[6]));

            // Draw cubic bezier curve from start_point to p3 using p1 and p2 as control points
            int segmentCount = 25;

            // Use a high number of line segments to make smoother curves
            // Calculate t and use it to place every point on the bezier in the right place
            sf::VertexArray cubic_bezier_curve(sf::PrimitiveType::LineStrip, segmentCount + 1);
            for (int i = 0; i <= segmentCount; i++) {
                float t = static_cast<float>(i) / static_cast<float>(segmentCount);
                sf::Vector2f point = getBezierPoint(t, current_point, p1, p2, p3);
                cubic_bezier_curve[i] = sf::Vertex{point, sf::Color::White};
            }

            for (int i = 0; i < cubic_bezier_curve.getVertexCount(); i++) {
                letter_container.letter_shapes.back().append(cubic_bezier_curve[i]);
            }

            // Update start_point for the next segment
            current_point = p3;

        } else if (line.front() == "L") {
            sf::Vector2f p1 = sf::Vector2f(std::stof(line[1]), std::stof(line[2]));

            // Draw single line from current to p1
            sf::VertexArray line_segment(sf::PrimitiveType::Lines, 2);
            line_segment[0] = sf::Vertex{current_point, sf::Color::White};
            line_segment[1] = sf::Vertex{p1, sf::Color::White};

            for (int i = 0; i < line_segment.getVertexCount(); i++) {
                letter_container.letter_shapes.back().append(line_segment[i]);
            }

            // Update start_point for the next segment
            current_point = p1;

        } else if (line.front() == "Z") {

            // Close the path by drawing a line back to the starting point
            sf::VertexArray line_segment(sf::PrimitiveType::Lines, 2);
            line_segment[0] = sf::Vertex{current_point, sf::Color::White};
            line_segment[1] = sf::Vertex{start_point, sf::Color::White};

            for (int i = 0; i < line_segment.getVertexCount(); i++) {
                letter_container.letter_shapes.back().append(line_segment[i]);
            }

        }
    }

    return letter_container;
}
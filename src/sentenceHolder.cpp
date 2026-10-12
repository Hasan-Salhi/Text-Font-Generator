#include <SFML/Graphics.hpp>
#include <vector>
#include "sentenceHolder.h"

void sentenceHolder::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform(); // Apply the transform of the container

    for (const auto& letter_shape : sentence_letters) {
        target.draw(letter_shape, states);
    }
}

// Custom implementation of getLocalBounds for this custom Transformable class
sf::FloatRect sentenceHolder::getLocalBounds() const {
    if (sentence_letters.empty()) {
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
    }

    float min_x = std::numeric_limits<float>::max();
    float max_x = -std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float max_y = -std::numeric_limits<float>::max();

    bool hasVertices = false;

    // Get min and max values to determine bounding box
    for (const auto& letter_shape : sentence_letters) {
        for (const auto& shape : letter_shape.letter_shapes) {
            for (int i = 0; i < shape.getVertexCount(); i++) {
                sf::Vector2f vertexPos = shape[i].position;
                if (vertexPos.x < min_x) min_x = vertexPos.x;
                if (vertexPos.x > max_x) max_x = vertexPos.x;
                if (vertexPos.y < min_y) min_y = vertexPos.y;
                if (vertexPos.y > max_y) max_y = vertexPos.y;

                hasVertices = true;
            }
        }
    }

    if (!hasVertices) {
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
    }

    return sf::FloatRect({min_x, min_y}, {max_x - min_x, max_y - min_y});
}

// Build sentence structure by applying the right transformations to each letter, determining how it will be displayed
void sentenceHolder::buildSentenceStructure(sf::Vector2f position, float scale_factor) {
    float cursor_x = -1.f;
    float space_gap = 0.45f;

    // Get sentence holder's letters
    std::vector<charContainer> letters = this->sentence_letters;

    // Need to make each character well spaced out relative to its bounds
    for (int i = 0; i < letters.size(); ++i) {
        if (letters[i].letter_shapes.empty()) {
            // If it's a space, just move the cursor by the space gap
            cursor_x += space_gap;
        } else {
            sf::FloatRect bounds = letters[i].getLocalBounds();

             // Scale down the letter
            letters[i].setScale(sf::Vector2f(scale_factor, scale_factor));
        
            // Adjust for the letter's local bounds
            float adjusted_x = cursor_x + position.x - (bounds.position.x * scale_factor);

            // Adjust position based on width and scale
            letters[i].setPosition(sf::Vector2f(adjusted_x, position.y));

            // Move cursor to the right by the width of the letter. Add slight padding so letters aren't touching
            cursor_x += bounds.size.x * scale_factor + 0.05f;
        }
    }

    // Store the letters in the sentence holder
    this->sentence_letters = letters;
}

// Take string and get ASCII codes. Used to determine which glyphs we need to build
// NOTE: Every glyph file uses the character's ASCII code
std::vector<int> sentenceHolder::getASCIIValues(const std::string& str) {
    std::vector<int> asciiValues;
    for (char c : str) {
        asciiValues.push_back(static_cast<int>(c));
    }
    return asciiValues;
}

// Takes input sentence, gets ASCII codes, uses it to find corresponding glyph files and build all the needed glyphs for the sentence
void sentenceHolder::buildText(const std::string& text, const std::string& font) {
    std::vector<charContainer> text_containers;
    std::vector<int> asciiValues = this->getASCIIValues(text);

    for (int ascii : asciiValues) {

        // Get file for corresponding character
        std::string glyph = "gly_" + std::to_string(ascii);
        
        // ASCII for space
        if (ascii == 32) {
            // Create an empty charContainer for space. Has no vertices
            charContainer space_container;
            text_containers.push_back(space_container);
        } else {

            // Build character and push it to vector
            charContainer letter_container = charContainer::buildGlyph(font, glyph);
            text_containers.push_back(letter_container);
        }
    }

    // Store the letters in the sentence holder
    this->sentence_letters = text_containers;
}
#pragma once
#include <SFML/Graphics.hpp>
#include <glad/gl.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

class charContainer : public sf::Drawable, public sf::Transformable {
    public:
        std::vector<sf::VertexArray> letter_shapes;

        sf::FloatRect getLocalBounds() const;

        static charContainer buildGlyph(std::string font, std::string glyph);
    
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        static std::vector<std::vector<std::string>> textParser(std::string font);

        // B(t) = (1 - t)^3 * p0 + 3t(1 - t)^2 * p1 + 3t^2 * (1 - t) * p2 + t^3 * p3
        static sf::Vector2f getBezierPoint(float t, sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3);

};
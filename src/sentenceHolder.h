#pragma once
#include <SFML/Graphics.hpp>
#include <glad/gl.h>
#include <vector>
#include <iostream>

#include "charContainer.h"

class sentenceHolder : public sf::Drawable, public sf::Transformable {
    public:    
        sf::FloatRect getLocalBounds() const;

        void buildSentenceStructure(sf::Vector2f position, float scale_factor);

        // Changes position by adding to current position instead of overwriting like setPosition()
        void addToPosition(sf::Vector2f new_position) {
            this->setPosition(this->getPosition() + new_position);
        }

        void changeScale(float scale_factor) {
            this->setScale(sf::Vector2f(scale_factor, scale_factor));
        }
    
        void buildText(const std::string& text, const std::string& font);

    protected:
        std::vector<charContainer> sentence_letters;

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        std::vector<int> getASCIIValues(const std::string& str);

};
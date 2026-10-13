#include <glad/gl.h> // Must be first!
#include <SFML/Graphics.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "charContainer.h"
#include "sentenceHolder.h"

char* DEFAULT_FONT = "cmunrm";
char* FONT;

// Important variables initialized here to be used in the rendering thread and main thread
sentenceHolder sentence_holder;

std::string message;

std::string test_msg = "The quick brown fox jumps over the lazy dog.";

void renderingThread(sf::RenderWindow& window, std::atomic<bool>& running, std::atomic<bool>& contextReady)
{
    if (!window.setActive(true)) {
        std::cerr << "CRITICAL: Thread failed to claim context!" << std::endl;
        contextReady = true;
        return;
    }
    contextReady = true;

    // Initialize GLAD with SFML's function loader
    if (!gladLoadGL(sf::Context::getFunction)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return;
    }

    // Hard verification check:
    if (sf::Shader::isAvailable()) {
        std::cout << "SUCCESS: Shaders are fully available on this thread!" << std::endl;
    } else {
        std::cerr << "ERROR: Context is active on thread, but driver reports NO SHADERS." << std::endl;
    }

    
    // match OpenGL's -1 to 1 canonical view with the Y axis pointing up.
	// Passing a negative height (-2.f) flips the Y-axis upside down!
	// Now: Top-Left is (-1.0, 1.0) and Bottom-Right is (1.0, -1.0)
	sf::View view(sf::Vector2f(0.f, 0.f), sf::Vector2f(2.f, -2.f));
	window.setView(view);

    // Background and alpha blending (for transparency)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build the characters and compile them into a sentence holder
    sentence_holder.buildText(message, FONT);

    float scale_factor = 1.0f;
    sf::Vector2f pos(0.f, 0.0f);

    // Apply all transformations to each letter so it's ready to be displayed
    sentence_holder.buildSentenceStructure(pos, scale_factor);

    // Fits the message on the whole window
    sentence_holder.setScale(sf::Vector2f(0.09f,0.09f));
    sentence_holder.setPosition(sf::Vector2f(-0.9f,0.9f));


    while(running) {
            // Is Key press events check continuously to see if a key is being held down. Makes the movement and scaling smoother and easier
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Left)) {
                sentence_holder.addToPosition(sf::Vector2f(0.0005f, 0.f)); // Move right
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Right)) {
                sentence_holder.addToPosition(sf::Vector2f(-0.0005f, 0.f)); // Move left
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Down)) {
                sentence_holder.addToPosition(sf::Vector2f(0.f, 0.0005f)); // Move up
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up)) {
                sentence_holder.addToPosition(sf::Vector2f(0.f, -0.0005f)); // Move down
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) {
                sentence_holder.changeScale(sentence_holder.getScale().x * 1.0001f); // Increase scale
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) {
                sentence_holder.changeScale(sentence_holder.getScale().x * 0.9999f); // Decrease scale
            }

        
        // draw...
		
        glClear(GL_COLOR_BUFFER_BIT);

        window.draw(sentence_holder);

		window.pushGLStates(); // Save OpenGL states before drawing SFML shapes
		window.popGLStates(); // Restore OpenGL states after drawing SFML shapes
		window.display();
    }

	window.setActive(false);
}

// SPICY ITALIAN SAUSAGE!

int main(int argc, char* argv[]) {
    std::cout << "Starting SFML + GLAD (OpenGL 2.1) application..." << std::endl;

    // Use default message and/or font if none provided
    message = test_msg; 
    FONT = DEFAULT_FONT;

    // Check for command line arguments to override defaults
    if (argc > 1) {
        message = argv[1];
        if (argc > 2) {
            FONT = argv[2];
        }
    }

	sf::ContextSettings settings;
    settings.majorVersion = 2;
    settings.minorVersion = 1;
    settings.attributeFlags = sf::ContextSettings::Default;
	
	sf::RenderWindow window(
    	sf::VideoMode({800, 800}), 
    	"SFML + GLAD (OpenGL 2.1)",
        sf::Style::Default, 
    	sf::State::Windowed,  
    	settings
	);



    // set additional window properties
    window.setKeyRepeatEnabled(false);
    
    // DO NOT LIMIT FRAME RATE! Use elapsed time to control updates.

    // deactivate its OpenGL context
    if( !window.setActive(false) ) {
        std::cerr << "Error setting up window, could not deactivate context" << std::endl;
        return -1;
    }

	std::atomic<bool> running(true);
    std::atomic<bool> contextReady(false);

    // Launch the rendering thread
    std::thread thread(renderingThread, std::ref(window), std::ref(running), std::ref(contextReady));
 
    // CRUCIAL: Pause the main thread until the rendering thread has successfully claimed the context.
    // This prevents pollEvent() from clashing with setActive(true) during initialization.
    while (!contextReady) {
        std::this_thread::yield(); 
    }

    // the event/logic/whatever loop
    while (window.isOpen()) {
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() ) {
				running = false; 
                
                // Wait for the rendering thread to safely deactivate and exit
                if (thread.joinable()) {
                    thread.join();
                }

                // Do additional cleanup here before closing the window
                window.close();
			}

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            
            }

            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                
				// key release events are used to trigger actions, not to hold them down.

			}
	
		}
    }

}
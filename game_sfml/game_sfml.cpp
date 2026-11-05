#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
    // Create the game window
    sf::RenderWindow window(
        sf::VideoMode({ 800, 600 }),
        "My First Aircraft Game"
    );

    window.setFramerateLimit(60);

    // Create the aircraft
    sf::RectangleShape aircraft({ 50.f, 35.f });
    aircraft.setFillColor(sf::Color::Cyan);
    aircraft.setPosition({ 375.f, 500.f });

    // Store the obstacles
    vector<sf::RectangleShape> obstacles;

    srand(static_cast<unsigned int>(time(nullptr)));

    sf::Clock gameClock;
    sf::Clock spawnClock;

    int score = 0;
    bool gameOver = false;

    cout << "Aircraft Game Started!" << endl;
    cout << "Use arrow keys or WASD to move." << endl;
    cout << "Avoid the red obstacles." << endl;

    while (window.isOpen())
    {
        // Handle window events
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        float deltaTime = gameClock.restart().asSeconds();

        if (!gameOver)
        {
            // Move the aircraft
            float speed = 300.f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            {
                aircraft.move({ -speed * deltaTime, 0.f });
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                aircraft.move({ speed * deltaTime, 0.f });
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            {
                aircraft.move({ 0.f, -speed * deltaTime });
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                aircraft.move({ 0.f, speed * deltaTime });
            }

            // Keep the aircraft inside the window
            sf::Vector2f pos = aircraft.getPosition();

            if (pos.x < 0.f)
                aircraft.setPosition({ 0.f, pos.y });

            if (pos.x > 750.f)
                aircraft.setPosition({ 750.f, pos.y });

            if (pos.y < 0.f)
                aircraft.setPosition({ pos.x, 0.f });

            if (pos.y > 565.f)
                aircraft.setPosition({ pos.x, 565.f });

            // Spawn a new obstacle every 0.8 seconds
            if (spawnClock.getElapsedTime().asSeconds() >= 0.8f)
            {
                sf::RectangleShape obstacle({ 35.f, 35.f });

                float x = static_cast<float>(rand() % 766);

                obstacle.setPosition({ x, -35.f });
                obstacle.setFillColor(sf::Color::Red);

                obstacles.push_back(obstacle);

                spawnClock.restart();
            }

            // Move obstacles and check for collisions
            for (auto& obstacle : obstacles)
            {
                obstacle.move({ 0.f, 220.f * deltaTime });

                if (aircraft.getGlobalBounds().findIntersection(
                    obstacle.getGlobalBounds()))
                {
                    gameOver = true;

                    cout << "\nGAME OVER!" << endl;
                    cout << "Final score: " << score << endl;
                    cout << "Press R to restart." << endl;
                }
            }

            // Remove obstacles that leave the screen
            for (int i = static_cast<int>(obstacles.size()) - 1;
                i >= 0; i--)
            {
                if (obstacles[i].getPosition().y > 600.f)
                {
                    obstacles.erase(obstacles.begin() + i);
                    score++;

                    cout << "Score: " << score << endl;
                }
            }
        }
        else
        {
            // Restart the game
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            {
                aircraft.setPosition({ 375.f, 500.f });

                obstacles.clear();

                score = 0;
                gameOver = false;

                gameClock.restart();
                spawnClock.restart();

                cout << "\nGame restarted!" << endl;
                cout << "Score: 0" << endl;
            }
        }

        // Draw the game
        window.clear(sf::Color(15, 20, 40));

        for (const auto& obstacle : obstacles)
        {
            window.draw(obstacle);
        }

        window.draw(aircraft);

        window.display();
    }

    return 0;
}
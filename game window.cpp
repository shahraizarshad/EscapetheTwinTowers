#include <iostream>
#include <SFML\Graphics.hpp>
int main() {
	//opening a window and keep it open until 'x' is pressed
	sf::RenderWindow Window(sf::VideoMode({ 800, 600 }), "Escape the twin towers");
	//Drawing a rectangle
	sf::RectangleShape rectangle({ 100,200 });
	rectangle.setFillColor(sf::Color::Green);  //fill colour in the rectangle
	rectangle.setPosition({ 200,200});         //position on window

	while (Window.isOpen()) {    //keep the window open until
		while (const std::optional event = Window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {  //if x is pressed close the window
				Window.close();
			}
		}   

		if ((rectangle.getPosition().x < 700) && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))) {

			rectangle.move({ 1,0 });
		}
		if ((rectangle.getPosition().x > 0) && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))) {
			rectangle.move({ -1, 0 });
		}
		if ((rectangle.getPosition().y<400) && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key:: Down))) {
			rectangle.move({ 0, 1 });
		}
		if ((rectangle.getPosition().y > 0) && (sf::Keyboard::isKeyPressed(sf ::Keyboard::Key::Up))) {

			rectangle.move({0, -1});
		}

		Window.clear();          // wipe the old picture
		Window.draw(rectangle);  //draw rectangle
		Window.display();        //show the new picture
	}
	return 0;
}
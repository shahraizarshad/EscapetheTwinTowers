#include <iostream>
#include <SFML\Graphics.hpp>
int main() {
	//opening a window and keep it open until 'x' is pressed
	sf::RenderWindow Window(sf::VideoMode({ 800, 600 }), "Escape the twin towers");
	//Drawing a rectangle
	sf::RectangleShape rectangle({ 100,200 });     //rectangleShape:name of data type,rectangle:name,50=radius        
	rectangle.setFillColor(sf::Color::Green);  //fill colour in the rectangle
	rectangle.setPosition({ 200,200 });         //position on window

	while (Window.isOpen()) {    //keep the window open until
		while (const std::optional event = Window.pollEvent()) {
			if (event->is<sf::Event::Closed>())   //if event x is pressed close the window
				Window.close();            //condition true = window close

		}
		Window.clear(); // wipe the old picture
		Window.draw(rectangle);  //draw rectangle
		Window.display();      //show the new picture

	}
	return 0;
}

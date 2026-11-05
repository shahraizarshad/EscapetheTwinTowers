#include <iostream>
#include <SFML\Graphics.hpp>
int main() {
	//opening a window and keep it open until 'x' is pressed
	sf::RenderWindow Window(sf::VideoMode({ 800, 600 }), "Escape the twin towers");
	while (Window.isOpen()) {    //keep the window open until
		while (const std::optional event = Window.pollEvent()) {
			if (event->is<sf::Event::Closed>())   //if event x is pressed close the window
				Window.close();            //condition true = window close
			
		}
		Window.clear();            //wipe the old picture
			Window.display();      //show the new picture

	}
	return 0;
} 
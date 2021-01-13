#include <SFML/Graphics.hpp>

int main() {

  sf::RenderWindow window(sf::VideoMode(840, 600), "SFML works!", sf::Style::Titlebar | sf::Style::Close);

  sf::CircleShape shape(100.f);
  shape.setFillColor(sf::Color::Green);

  while(window.isOpen()) {

    sf::Event event;
    
    while(window.pollEvent(event)) {
      switch (event.type) {
      case sf::Event::Closed:
        window.close();
        break;
      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape) {
          window.close();
        }
        break;
      }
    }

    window.clear();
    window.draw(shape);
    window.display();
  }

  return 0;
}
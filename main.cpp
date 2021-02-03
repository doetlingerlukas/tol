#include <sstream>
#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <game.hpp>

int main(int argc, char **argv) {
  try {
    const auto& settings = Settings(argv[0]);

    Game game(settings);
    game.run();

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}

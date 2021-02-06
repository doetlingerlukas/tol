#include <sstream>
#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <game.hpp>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  try {
    const auto executeable_path = fs::canonical(argv[0]);
    const auto executeable_dir = executeable_path.parent_path();

    const auto settings = Settings(executeable_path);

    Game game(executeable_dir, settings);
    game.run();

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}

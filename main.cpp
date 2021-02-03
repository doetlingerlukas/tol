#include <game.hpp>
#include <settings.hpp>


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

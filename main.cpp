#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <game.hpp>
#include <settings.hpp>

#define NK_IMPLEMENTATION
#define NK_SFML_GL2_IMPLEMENTATION

#include <nuklear.h>
#include <nuklear_sfml_gl2.h>

int main(int argc, char** argv) {
  try {
    spdlog::cfg::load_env_levels();

    const auto executeable_path = fs::canonical(argv[0]);
    const auto executeable_dir = executeable_path.parent_path();

    spdlog::basic_logger_mt("file_logger", fs::path(executeable_dir / "logs" / "basic-log.txt").string());
    auto logger = spdlog::get("file_logger");
    logger->set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");

    auto settings = Settings(executeable_path);

    Game game(executeable_dir, settings);
    game.run();

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}

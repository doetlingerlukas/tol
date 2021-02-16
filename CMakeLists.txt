cmake_minimum_required(VERSION 3.17)

project(tol VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(BuildType)
include(CxxOptions)
include(ClangTidy)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

add_executable(tol)
target_sources(tol
  PRIVATE
    main.cpp
    map.cpp
    play_state.cpp
    tile.cpp
    object.cpp
    asset_cache.cpp
    nuklear.cpp
    dialog_state.cpp
    game_instance.cpp
    settings.cpp
    stats.cpp
    character.cpp
    npc.cpp
    protagonist.cpp
)

target_include_directories(tol SYSTEM PRIVATE external)
target_include_directories(tol PRIVATE include)

set(LINUX FALSE)
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(${LINUX})
  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL)
  target_link_libraries(tol PRIVATE OpenGL::GL)

  find_package(OpenAL CONFIG REQUIRED)
  target_link_libraries(tol PRIVATE OpenAL::OpenAL)
endif()

if(WIN32)
  target_link_libraries(tol PRIVATE opengl32)
endif()

find_path(NUKLEAR_INCLUDE_DIRS "nuklear.h")
target_include_directories(tol PRIVATE ${NUKLEAR_INCLUDE_DIRS})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(tol PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
target_link_libraries(tol PRIVATE sfml-graphics sfml-window sfml-audio sfml-system)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(tol PRIVATE fmt::fmt)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(tol PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

add_custom_target(assets ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_BINARY_DIR}/assets"
)
add_dependencies(tol assets)

# Don't compile map on Linux CI since it needs a GUI.
if(NOT DEFINED ENV{CI} OR NOT ${LINUX})
  file(GLOB_RECURSE MAP_DEPENDENCIES "${CMAKE_CURRENT_SOURCE_DIR}/assets" "*.tmx")
  set(MAP_TMX "${CMAKE_CURRENT_SOURCE_DIR}/assets/map.tmx")
  set(MAP_JSON "${CMAKE_CURRENT_BINARY_DIR}/assets/map.json")
  add_custom_command(
    DEPENDS "${MAP_DEPENDENCIES}"
    OUTPUT "${MAP_JSON}"
    COMMAND tiled --minimize --embed-tilesets --export-map "${MAP_TMX}" "${MAP_JSON}"
    COMMAND ruby "${CMAKE_CURRENT_SOURCE_DIR}/fix-map-indices.rb" "${MAP_JSON}"
  )
  add_custom_target(map ALL
    DEPENDS "${MAP_JSON}"
  )
  add_dependencies(tol map)
endif()

cmake_minimum_required(VERSION 3.3)

project(tol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(tol)
target_sources(tol
  PRIVATE
    main.cpp
)

target_include_directories(tol PRIVATE include)
target_include_directories(tol PRIVATE external)

if(UNIX AND NOT APPLE)
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

add_custom_target(assets ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_BINARY_DIR}/assets"
)
add_dependencies(tol assets)

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

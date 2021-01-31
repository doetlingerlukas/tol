cmake_minimum_required(VERSION 3.0)

project(tol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

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
endif()

if(WIN32)
  target_link_libraries(tol PRIVATE opengl32)
endif()

find_path(NUKLEAR_INCLUDE_DIRS "nuklear.h")
target_include_directories(tol PRIVATE ${NUKLEAR_INCLUDE_DIRS})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(tol PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(tol PRIVATE sfml-graphics sfml-window sfml-system)


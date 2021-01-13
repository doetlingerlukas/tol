cmake_minimum_required(VERSION 3.0)

project(tol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_program(XXD xxd)


find_program(GLSLC glslc)


set(shader_path ${CMAKE_HOME_DIRECTORY}/shaders)

file(GLOB shaders RELATIVE ${CMAKE_SOURCE_DIR} "${shader_path}/*.vert" "${shader_path}/*.frag")
foreach(shader ${shaders})
  set(input_glsl "${CMAKE_HOME_DIRECTORY}/${shader}")
  set(output_spv "${input_glsl}.spv")
  execute_process(COMMAND "glslc" "${input_glsl}" "-o" "${output_spv}")
  configure_file(${output_spv} "${CMAKE_BINARY_DIR}/${shader}.spv" COPYONLY)
endforeach()

execute_process(COMMAND "mkdir" "-p" "nuklear")

set(NUKLEAR_SHADERS
  nuklear.frag
  nuklear.vert
)

foreach (SHADER ${NUKLEAR_SHADERS})
  execute_process(COMMAND "xxd" "-i" "shaders/${SHADER}.spv" "${CMAKE_HOME_DIRECTORY}/nuklear/${SHADER}.h")
endforeach()

add_executable(tol)
target_sources(tol
  PRIVATE
    main.cpp
    overlay.c
)

find_package(glad CONFIG REQUIRED)
target_link_libraries(tol PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(tol PRIVATE glfw)

find_path(NUKLEAR_INCLUDE_DIRS "nuklear.h")
target_include_directories(tol PRIVATE ${NUKLEAR_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)

target_include_directories(tol PRIVATE
  "$ENV{VULKAN_SDK}/include"
  include
  nuklear
)

target_link_directories(tol PUBLIC "$ENV{VULKAN_SDK}/lib")
target_link_libraries(tol PRIVATE Vulkan::Vulkan)

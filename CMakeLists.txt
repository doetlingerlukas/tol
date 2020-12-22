cmake_minimum_required(VERSION 3.0)

project(tol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(tol)
target_sources(tol
  PRIVATE
    main.cpp
)

find_package(glad CONFIG REQUIRED)
target_link_libraries(tol PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(tol PRIVATE glfw)

find_package(Vulkan REQUIRED)
target_include_directories(tol PUBLIC "$ENV{VULKAN_SDK}/include")
target_link_directories(tol PUBLIC "$ENV{VULKAN_SDK}/lib")
target_link_libraries(tol PRIVATE Vulkan::Vulkan)

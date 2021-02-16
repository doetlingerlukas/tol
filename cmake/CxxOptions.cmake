set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0.0")
    message(FATAL_ERROR "GCC 10 or higher is required")
  endif()
endif()

add_compile_options(-pedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-Wall -Werror)
endif()

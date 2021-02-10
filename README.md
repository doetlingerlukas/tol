# tol

## Dependencies

- CMake (https://cmake.org)
- Ruby (https://www.ruby-lang.org/)
- Tiled (https://www.mapeditor.org)
- Ninja (https://ninja-build.org)
- vcpkg (https://docs.microsoft.com/cpp/build/vcpkg)

To install these, run `./deps.sh`.

## Building

Build using

```sh
cmake -G Ninja -B ./build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build ./build -v
```

or alternatively, simply run

```sh
rake build
```

## Running

Start the application using

```sh
./build/tol
```

or alternatively, run

```sh
rake run
```

# tol

A 2D game written in C++.

View the provided [specification](specification.md) for more detail.

## Dependencies

- CMake (https://cmake.org)
- Ruby (https://www.ruby-lang.org/)
- Tiled (https://www.mapeditor.org)
- Ninja (https://ninja-build.org)
- vcpkg (https://docs.microsoft.com/cpp/build/vcpkg)

To install these, run `./deps.sh`.

## Building

When building inside a Git repository, the build defaults to `-DCMAKE_BUILD_TYPE=Debug`,
otherwise it will use `-DCMAKE_BUILD_TYPE=Release`. Specify either of these explicitly
to override the default behaviour.

Build using

```sh
cmake -G Ninja -B ./build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
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

## Controls

| Action            | Control                                                                   |
|-------------------|---------------------------------------------------------------------------|
| Player Movement   | <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd>, <kbd>D</kbd>                    |
| Attack Selection  | <kbd>↑</kbd> (Up Arrow), <kbd>↓</kbd> (Down Arrow), <kbd>⏎</kbd> (Return) |
| Talk to Character | <kbd>E</kbd>, Mouse                                                       |
| Show Inventory    | <kbd>I</kbd>                                                              |
| Show Quests       | <kbd>⇥</kbd> (Tab)                                                        |
| Debug Mode        | <kbd>H</kbd>                                                              |
| Open Menu         | <kbd>Esc</kbd>                                                            |

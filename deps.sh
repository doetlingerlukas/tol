#!/usr/bin/env bash

set -euo pipefail

uname="$(uname -s)"
if [[ "${uname}" == 'Darwin' ]]; then
  brew install ruby
  brew install cmake
  brew install llvm
  brew install vcpkg
  brew install ninja
  brew install tiled

  ln -sfn "$(brew --prefix vcpkg)/libexec" vcpkg
elif [[ "${uname}" == 'Linux' ]]; then
  sudo apt-get update
  sudo apt-get install -y build-essential tar curl cmake zip unzip gcc ruby ninja-build \
    libx11-dev libxrandr-dev libxi-dev libudev-dev libgl1-mesa-dev pkg-config \
    clang-format clang-tidy

  if ! [[ -d vcpkg ]]; then
    git clone https://github.com/microsoft/vcpkg
    pushd vcpkg
    ./bootstrap-vcpkg.sh
    popd
  fi

  export PATH="$(pwd)/vcpkg:${PATH}"

  if ! which tiled; then
    sudo curl -L https://github.com/mapeditor/tiled/releases/download/v1.4.3/Tiled-1.4.3-x86_64.AppImage -o /usr/local/bin/tiled
    sudo chmod +x /usr/local/bin/tiled
  fi
else
  :
fi

which vcpkg

vcpkg install fmt
vcpkg install nlohmann-json
vcpkg install nuklear
vcpkg install sfml
vcpkg install spdlog
vcpkg install openal-soft

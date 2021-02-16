# frozen_string_literal: true

require 'json'

BUILD_DIR = './build'

task :deps do
  sh './deps.sh'
end

task :build do
  sh 'cmake', '-G', 'Ninja', '-B', BUILD_DIR, '-DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake'
  sh 'cmake', '--build', BUILD_DIR, '-v'
end

task run: :build do
  sh "#{BUILD_DIR}/tol"
end

task :format do
  sh 'clang-format', '-i', *Dir.glob('**/*.{c,h}pp')
end

task :clean do
  rm_rf BUILD_DIR
end

task default: :build

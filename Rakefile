# frozen_string_literal: true

require 'json'

BUILD_DIR = 'build'

def mac?
  RUBY_PLATFORM.include?('darwin')
end

task :deps do
  sh 'brew', 'install', 'vcpkg' if mac?
  sh 'vcpkg', 'install', 'glad'
  sh 'vcpkg', 'install', 'glfw3'
  sh 'vcpkg', 'install', 'sfml'
end

task :map do
  sh 'tiled', '--minimize', '--embed-tilesets', '--export-map', 'assets/map.tmx', 'assets/map.json'

  map = JSON.parse(File.read('assets/map.json'))

  map['tilesets'].each_with_index do |tileset, i|
    first_gid = tileset['firstgid']

    tileset['tiles']&.each_with_index do |tile, i|
      tile['id'] = first_gid + i
    end
  end

  File.write('assets/map.json', JSON.pretty_generate(map))
end

task :build => :map do
  vcpkg_prefix = if mac?
    `brew --prefix vcpkg`.chomp
  else
    '/usr/share/vcpkg'
  end
  toolchain_file = "#{vcpkg_prefix}/libexec/scripts/buildsystems/vcpkg.cmake"

  sh 'cmake', '-G', 'Ninja', '-B', BUILD_DIR, '-DCMAKE_BUILD_TYPE=Debug', "-DCMAKE_TOOLCHAIN_FILE=#{toolchain_file}"
  sh 'cmake', '--build', BUILD_DIR, '-v'
end

task run: :build do
  sh "#{BUILD_DIR}/tol"
end

task :clean do
  rm_rf BUILD_DIR
end

task default: :build

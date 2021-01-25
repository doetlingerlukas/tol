# frozen_string_literal: true

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

task :build do
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

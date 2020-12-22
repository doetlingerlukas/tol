# frozen_string_literal: true

BUILD_DIR = 'build'

def mac?
  RUBY_PLATFORM.include?('darwin')
end

task :build do
  vulkan_sdk = if mac?
    '/usr/local'
  end

  vcpkg_prefix = if mac?
    `brew --prefix vcpkg`.chomp
  else
    '/usr/share/vcpkg'
  end
  toolchain_file = "#{vcpkg_prefix}/libexec/scripts/buildsystems/vcpkg.cmake"

  env = { "VULKAN_SDK" => vulkan_sdk }
  sh env, 'cmake', '-G', 'Ninja', '-B', BUILD_DIR, '-DCMAKE_BUILD_TYPE=Debug', "-DCMAKE_TOOLCHAIN_FILE=#{toolchain_file}"
  sh 'cmake', '--build', BUILD_DIR, '-v'
end

task run: :build do
  sh "#{BUILD_DIR}/tol"
end

task :clean do
  rm_rf BUILD_DIR
end

task default: :build

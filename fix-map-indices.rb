#!/usr/bin/env ruby

require 'pathname'
require 'json'

path = Pathname(ARGV.first)

map = JSON.parse(path.read)

map['tilesets'].each do |tileset|
  first_gid = tileset['firstgid']

  tileset['tiles']&.each do |tile|
    tile['id'] = first_gid - 1 + tile['id']

    tile['animation']&.each do |animation|
      animation['tileid'] = first_gid + animation['tileid']
    end
  end
end

path.write(JSON.pretty_generate(map))

#pragma once

class Fight: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  const int TILE_SIZE = 64;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::RectangleShape background;
    background.setSize({ (float)target.getSize().x, (float)target.getSize().y });
    background.setFillColor(sf::Color(132, 94, 28, 200));
    target.draw(background);

    const int scale_factor = 5;

    const int resize_x = ((float)target.getSize().x / ((float)target.getSize().x / target.getSize().y)) * 1.77777777778;

    sf::Sprite player;
    player.setTexture(*asset_cache->loadTexture("tilesets/character-whitebeard.png"));
    player.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 3, TILE_SIZE, TILE_SIZE });
    player.setPosition({ target.getSize().x - resize_x - 100.0f, target.getSize().y - (100.0f * scale_factor) });
    player.setScale({ scale_factor, scale_factor });
    target.draw(player);

    sf::Sprite enemy;

    enemy.setTexture(*asset_cache->loadTexture("tilesets/character-whitebeard.png"));
    enemy.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 5, TILE_SIZE, TILE_SIZE });
    enemy.setPosition({ resize_x - 100.0f, 100.0f });
    enemy.setScale({ scale_factor, scale_factor });
    target.draw(enemy);
  }

public:
  Fight(const std::shared_ptr<AssetCache> asset_cache_): asset_cache(asset_cache_) {}
};

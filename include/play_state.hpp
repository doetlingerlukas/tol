#pragma once

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <character.hpp>
#include <input.hpp>

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

class PlayState: public sf::Drawable {
  std::shared_ptr<AssetCache> asset_cache;

  sf::View map_view;
  sf::Font font;
  TiledMap* map;
  Character* player;

  sf::Vector2f scale;
  sf::Vector2f direction = { 0.0f, 0.0f };

  std::vector<sf::RectangleShape> collision_rects;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    target.setView(map_view);
    target.draw(*map);

    for (auto shape : collision_rects) {
      shape.setScale(scale);
      auto position = shape.getPosition();
      shape.setPosition({ position.x * scale.x, position.y * scale.y });
      target.draw(shape);
    }

    auto center = map_view.getCenter();

    std::stringstream ss;
    ss << "Center Coords: " << center.x << ", " << center.y << "\n";
    ss << "Player: " << player->getPosition().x << ", " << player->getPosition().y << "\n";

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16 * scale.y);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(1);
    text.setString(ss.str());
    
    target.setView(target.getDefaultView());
    target.draw(text);
  }

public:
  PlayState(TiledMap* map_, Character* player_, std::shared_ptr<AssetCache> asset_cache_, const sf::Vector2f& scale_, const sf::Vector2u& window_size):
    map(map_), player(player_), asset_cache(asset_cache_), scale(scale_) {
    
    font.loadFromFile((asset_cache_->dir() / "fonts/Gaegu-Regular.ttf").string());

    map_view.reset({ 0, (map->getSize().y - window_size.y) * scale_.y, (float) window_size.x, (float) window_size.y });

    const auto spawn = map->getSpawn();
    if (spawn) {
      map_view.setCenter({ spawn->x * scale_.x, spawn->y * scale_.y });
      player_->setPosition(*spawn);
    }
  }

  void update(KeyInput& key_input, const sf::RenderWindow& window, const std::chrono::milliseconds& now, float dt) {
    collision_rects = map->collisionTiles(*player);

    player->move(
      (key_input.a && !key_input.d) ? std::optional(LEFT) : ((key_input.d && !key_input.a) ? std::optional(RIGHT) : std::nullopt),
      (key_input.w && !key_input.s) ? std::optional(UP) : ((key_input.s && !key_input.w) ? std::optional(DOWN) : std::nullopt),
      dt * CHARACTER_MOVE_SPEED, now, collision_rects, map->getSize()
    );


    if (key_input.up && !key_input.down) {
      direction.y = std::clamp(direction.y + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
    }
    else if (key_input.down && !key_input.up) {
      direction.y = std::clamp(direction.y - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
    }
    else {
      if (direction.y >= 0.5) {
        direction.y -= 1.0 * dt * VIEW_MOVE_DECEL;
      }
      else if (direction.y <= -0.5) {
        direction.y += 1.0 * dt * VIEW_MOVE_DECEL;
      }
      else {
        direction.y = 0;
      }
    }

    if (key_input.right && !key_input.left) {
      direction.x = std::clamp(direction.x - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
    }
    else if (key_input.left && !key_input.right) {
      direction.x = std::clamp(direction.x + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
    }
    else {
      if (direction.x >= 0.5) {
        direction.x -= 1.0 * dt * VIEW_MOVE_DECEL;
      }
      else if (direction.x <= -0.5) {
        direction.x += 1.0 * dt * VIEW_MOVE_DECEL;
      }
      else {
        direction.x = 0;
      }
    }

    map_view.setCenter(map->getView(window.getSize().x, window.getSize().y));
    map->update(map_view, window, now);
  }

};
#pragma once

#include <optional>

#include <SFML/Audio.hpp>

#include <animation.hpp>
#include <stats.hpp>
#include <z_indexable.hpp>
#include <inventory.hpp>

enum CharacterDirection {
  UP = 0,
  LEFT,
  DOWN,
  RIGHT,
};

enum CharacterAnimation {
  SPELLCAST = 0,
  THRUST,
  WALK,
  SLASH,
  SHOOT,
  HURT,
};

const int CHARACTER_ANIMATION_FRAMES[6] {
  7,
  8,
  9,
  6,
  13,
  6,
};

const int TILE_SIZE = 64;
const int EFFECT_TILE_SIZE = 32;

class Character: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  Inventory inventory;

  mutable sf::Sprite sprite;
  std::optional<Animation> animation;
  std::chrono::milliseconds now;
  std::chrono::milliseconds last_collision = std::chrono::milliseconds(0);

  mutable sf::Sprite effect;

  std::optional<sf::IntRect> current_effect;

  std::string name;
  std::function<void(const std::string&)> pickup_callback;

protected:
  void registerPickup(std::function<void(const std::string&)> callback) {
    pickup_callback = callback;
  }

  std::shared_ptr<Stats> stats;

public:
  Character(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache_,
      const std::shared_ptr<Stats> stats_, const std::string& name_) : asset_cache(asset_cache_), stats(stats_), name(name_), inventory(Inventory(32)) {
    sprite.setTexture(*asset_cache->loadTexture(path));
    sprite.setTextureRect({ 0, 0, TILE_SIZE, TILE_SIZE });
    sprite.setOrigin({ TILE_SIZE / 2.f, TILE_SIZE - 6.f });

    effect.setTexture(*asset_cache->loadTexture("tilesets/effects.png"));
    effect.setOrigin({ EFFECT_TILE_SIZE / 2.f, EFFECT_TILE_SIZE / 2.f });
  }

  sf::FloatRect bounding_box_rect;

  mutable CharacterDirection last_direction = DOWN;

  std::string getName() const {
    return name;
  }

  std::vector<std::pair<std::string, Object>> getInventoryElements() const {
    return inventory.getElements();
  }
  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    if (animation) {
      sprite.setTextureRect(animation->getDrawingRect(now));
    } else {
      sprite.setTextureRect({ 0 * TILE_SIZE, last_direction * TILE_SIZE, TILE_SIZE, TILE_SIZE });
    }

    auto scale = getScale();
    sprite.setPosition({ getPosition().x * scale.x, getPosition().y * scale.y });
    sprite.setScale(scale);

    const auto shadow_ratio = 2.6f;

    sf::CircleShape shadow;
    shadow.setRadius(TILE_SIZE / 5.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    shadow.setPosition(sprite.getPosition());
    shadow.setOrigin({ shadow.getRadius(), shadow.getRadius() });
    shadow.setScale({ scale.x, scale.y / shadow_ratio });

    auto bounding_box_rect = getBoundingRect();
    sf::RectangleShape bounding_box;
    bounding_box.setSize({bounding_box_rect.width, bounding_box_rect.height});
    bounding_box.setOutlineThickness(0.5f);
    bounding_box.setOutlineColor(sf::Color::Red);
    bounding_box.setFillColor(sf::Color::Transparent);
    bounding_box.setPosition({bounding_box_rect.left * scale.x, bounding_box_rect.top * scale.y});
    bounding_box.setScale(scale);

    auto texture_bounding_box_rect = getTextureBoundingRect();
    sf::RectangleShape texture_bounding_box;
    texture_bounding_box.setSize({texture_bounding_box_rect.width, texture_bounding_box_rect.height});
    texture_bounding_box.setOutlineThickness(0.5f);
    texture_bounding_box.setOutlineColor(sf::Color::Green);
    texture_bounding_box.setFillColor(sf::Color::Transparent);
    texture_bounding_box.setPosition({texture_bounding_box_rect.left * scale.x, texture_bounding_box_rect.top * scale.y});
    texture_bounding_box.setScale(scale);

    if (current_effect) {
      effect.setTextureRect(*current_effect);
      effect.setPosition({ getPosition().x * scale.x, (getPosition().y - EFFECT_TILE_SIZE * 1.5f) * scale.y });
      effect.setScale({ scale.x, scale.y });
    }

    target.draw(shadow);
    target.draw(sprite);
    target.draw(bounding_box);
    target.draw(texture_bounding_box);
    target.draw(effect);
  }

  sf::FloatRect getBoundingRect() const {
    const auto& position = getPosition();
    const auto width = 16.f;
    const auto height = 8.f;

    return {
      (position.x - width / 2.f),
      (position.y - height / 2.f),
      width,
      height,
    };
  }

  sf::FloatRect getTextureBoundingRect() const {
    const auto& position = getPosition();
    const auto& origin = sprite.getOrigin();
    const auto& texture_rect = sprite.getTextureRect();

    return {
      static_cast<float>(position.x - origin.x),
      static_cast<float>(position.y - origin.y),
      static_cast<float>(texture_rect.width),
      static_cast<float>(texture_rect.height),
    };
  }

  void move(std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction,
    float speed, std::chrono::milliseconds now, std::vector<sf::RectangleShape>& collision_rects, std::map<int, Object>& collectibles, const sf::Vector2f& map_size) {
    auto position = getPosition();

    auto speed_adjusted = speed * (stats->speed().get() / 10.0f);

    sf::Vector2f velocity = { 0.f, 0.f };

    if (x_direction == RIGHT) {
      velocity.x += speed_adjusted;
    }

    if (x_direction == LEFT) {
      velocity.x -= speed_adjusted;
    }

    if (y_direction == UP) {
      velocity.y -= speed_adjusted;
    }

    if (y_direction == DOWN) {
      velocity.y += speed_adjusted;
    }

    auto stop_movement = [this, &x_direction, &y_direction, now](CharacterDirection direction) {
      if (direction == LEFT || direction == RIGHT) {
        if (x_direction == direction) {
          x_direction = std::nullopt;

          if (!y_direction) {
            last_direction = direction;
          }
        }
      } else if (direction == UP || direction == DOWN) {
        if (y_direction == direction) {
          y_direction = std::nullopt;

          if (!x_direction) {
            last_direction = direction;
          }
        }
      }

      const auto td = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_collision);

      if(td.count() > 250)
        stats->health().decrease(10);

      this->last_collision = now;
    };

    const auto player_bounds = getBoundingRect();

    auto next_bounds = player_bounds;
    next_bounds.left += velocity.x;
    next_bounds.top += velocity.y;

    const auto player_width = player_bounds.width;
    const auto player_height = player_bounds.height;
    const auto player_left = player_bounds.left;
    const auto player_right = player_left + player_width;
    const auto player_top = player_bounds.top;
    const auto player_bottom = player_top + player_height;

    const auto td2 = now - this->now;

    for (auto& rect : collision_rects) {
      auto obstacle_bounds = rect.getGlobalBounds();
      if (obstacle_bounds.intersects(next_bounds)) {
        rect.setFillColor(sf::Color(255, 0, 0, 100));

        const auto obstacle_left = obstacle_bounds.left;
        const auto obstacle_right = obstacle_left + obstacle_bounds.width;
        const auto obstacle_top = obstacle_bounds.top;
        const auto obstacle_bottom = obstacle_top + obstacle_bounds.height;

        // Left collision
        if (
          player_left    > obstacle_left  &&
          player_right  > obstacle_right  &&
          player_top    < obstacle_bottom &&
          player_bottom > obstacle_top
        ) {
          next_bounds.left = obstacle_right;
          stop_movement(LEFT);
        }

        // Right collision
        if (
          player_left   < obstacle_left   &&
          player_right  < obstacle_right  &&
          player_top    < obstacle_bottom &&
          player_bottom > obstacle_top
        ) {
          next_bounds.left = obstacle_left - player_width;
          stop_movement(RIGHT);
        }

        // Top collision
        if (
          player_top    > obstacle_top    &&
          player_bottom > obstacle_bottom &&
          player_left   < obstacle_right  &&
          player_right  > obstacle_left
        ) {
          next_bounds.top = obstacle_bottom;
          stop_movement(UP);
        }

        // Bottom collision
        if (
          player_top    < obstacle_top    &&
          player_bottom < obstacle_bottom &&
          player_left   < obstacle_right  &&
          player_right  > obstacle_left
        ) {
          next_bounds.top = obstacle_top - player_height;
          stop_movement(DOWN);
        }
      }
    }

    // Restrict movement outside the map
    if (next_bounds.left < 0) {
      next_bounds.left = 0.f;
      stop_movement(LEFT);
    }

    if (next_bounds.left + player_width > map_size.x) {
      next_bounds.left = map_size.x - player_width;
      stop_movement(RIGHT);
    }

    if (next_bounds.top < 0) {
      next_bounds.top = 0.f;
      stop_movement(UP);
    }

    if (next_bounds.top + player_height > map_size.y) {
      next_bounds.top = map_size.y - player_height;
      stop_movement(DOWN);
    }

    for (auto it = collectibles.cbegin(); it != collectibles.cend();) {
      auto& [id, collectible] = *it;

      if (collectible.collides_with(next_bounds)) {
        std::cout << "Item collected: " << collectible.getName() << std::endl;
        inventory.add(make_pair(collectible.getName(), collectible));

        if (pickup_callback != nullptr)
          pickup_callback(collectible.getName());

        it = collectibles.erase(it);
      } else {
        it++;
      }
    }

    setPosition({ next_bounds.left + player_width / 2.f, next_bounds.top + player_height / 2.f });

    const auto direction = x_direction ? x_direction : y_direction;

    if (direction) {
      if (direction != last_direction) {
        animation = std::nullopt;
        last_direction = *direction;
      }

      if (!animation) {
        std::vector<std::tuple<std::chrono::milliseconds, sf::IntRect>> frames;

        auto animation_type = WALK;
        for (int i = 1; i < CHARACTER_ANIMATION_FRAMES[animation_type]; i++) {
          frames.push_back(std::make_tuple(std::chrono::milliseconds(100), sf::IntRect{ i * TILE_SIZE, (animation_type * 4 + *direction) * TILE_SIZE, TILE_SIZE, TILE_SIZE }));
        }

        animation = Animation(std::move(frames));
      }
    } else {
      animation = std::nullopt;
    }

    this->now = now;
  }

  virtual std::optional<float> zIndex() const {
    const auto& texture_bounding_rect = getTextureBoundingRect();
    return texture_bounding_rect.top + texture_bounding_rect.height;
  }

  float distanceTo(const Character& other) {
    const auto a = getPosition();
    const auto b = other.getPosition();

    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
  }

  void setEffectRect(sf::IntRect effect) {
    current_effect = effect;
  }

  void resetEffect() {
    current_effect = std::nullopt;
  }

  void lookToward(const sf::Vector2f point) {
    const auto x_dist = std::abs(point.x - getPosition().x);
    const auto y_dist = std::abs(point.y - getPosition().y);

    if (x_dist >= y_dist) {
      if (point.x < getPosition().x) {
        last_direction = LEFT;
      } else {
        last_direction = RIGHT;
      }
    } else {
      if (point.y < getPosition().y) {
        last_direction = UP;
      } else {
        last_direction = DOWN;
      }
    }
  }
};

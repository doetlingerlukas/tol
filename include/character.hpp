#pragma once

#include <animation.hpp>

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

class Character: public sf::Drawable, public sf::Transformable {
  sf::Texture texture;
  mutable sf::Sprite sprite;
  mutable std::optional<Animation> animation;

  std::optional<CharacterDirection> x_direction;
  std::optional<CharacterDirection> y_direction;

public:
  Character(const fs::path& path) {
    texture.loadFromFile(path.string());
    sprite.setTexture(texture);
    sprite.setOrigin({ TILE_SIZE / 2.f, TILE_SIZE / 8.f * 7.f });
  }

  mutable CharacterDirection last_direction = DOWN;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

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
          frames.push_back(std::make_tuple(std::chrono::milliseconds(125), sf::IntRect{ i * TILE_SIZE, (animation_type * 4 + *direction) * TILE_SIZE, TILE_SIZE, TILE_SIZE }));
        }

        animation = Animation(std::move(frames));
      }
    } else {
      animation = std::nullopt;
    }

    if (animation) {
      sprite.setTextureRect(animation->getDrawingRect(now));
    } else {
      sprite.setTextureRect({ 0 * TILE_SIZE, last_direction * TILE_SIZE, TILE_SIZE, TILE_SIZE });
    }

    auto scale = getScale();
    sprite.setPosition({ getPosition().x * scale.x, getPosition().y * scale.y });
    sprite.setScale(scale);

    target.draw(sprite);
  }

  void move(std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed) {
    this->x_direction = x_direction;
    this->y_direction = y_direction;

    auto position = getPosition();

    if (x_direction == RIGHT) {
      position.x += 1.0 * speed;
    }

    if (x_direction == LEFT) {
      position.x -= 1.0 * speed;
    }

    if (y_direction == UP) {
      position.y -= 1.0 * speed;
    }

    if (y_direction == DOWN) {
      position.y += 1.0 * speed;
    }

    setPosition(position);
  }
};

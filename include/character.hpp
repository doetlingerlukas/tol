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

class Character: public sf::Drawable {
  sf::Texture texture;
  sf::Vector2f position;
  sf::Vector2f scale;
  mutable sf::Sprite sprite;
  mutable std::optional<Animation> animation;


public:
  Character(const fs::path& path) {
    texture.loadFromFile(path.string());
    sprite.setTexture(texture);
  }

  int direction = 0;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    if (animation) {
      sprite.setTextureRect(animation->getDrawingRect(now));
    } else {
      sprite.setTextureRect({ DOWN * TILE_SIZE, direction * TILE_SIZE, TILE_SIZE, TILE_SIZE });
    }

    sprite.setPosition(position);
    sprite.setScale(scale);

    target.draw(sprite);
  }

  void move(CharacterDirection direction) {
    if (this->direction != direction) {
      std::vector<std::tuple<std::chrono::milliseconds, sf::IntRect>> frames;

      auto animation_type = WALK;
      for (int i = 1; i < CHARACTER_ANIMATION_FRAMES[animation_type]; i++) {
        frames.push_back(std::make_tuple(std::chrono::milliseconds(125), sf::IntRect{ i * TILE_SIZE, (animation_type * 4 + direction) * TILE_SIZE, TILE_SIZE, TILE_SIZE }));
      }

      this->animation = Animation(std::move(frames));
    }

    this->direction = direction;
  }

  void setScale(sf::Vector2f scale) {
    this->scale = scale;
  }
};

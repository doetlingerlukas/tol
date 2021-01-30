#pragma once

#include <animation.hpp>
#include <optional>

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

  sf::FloatRect bounding_box_rect;

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

    const auto shadow_ratio = 2.f;

    sf::CircleShape shadow;
    shadow.setRadius(TILE_SIZE / 4.f);
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

    target.draw(shadow);
    target.draw(sprite);
    target.draw(bounding_box);
  }

  sf::FloatRect getBoundingRect() const {
    const auto width = (TILE_SIZE / 8.f * 3.f);
    const auto height = (TILE_SIZE / 8.f * 2.f);

    return {
      (getPosition().x - width / 2.f),
      (getPosition().y - height / 2.f),
      width,
      height,
    };
  }

  void move(std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed, const std::vector<sf::RectangleShape>& collision_rects) {
    this->x_direction = x_direction;
    this->y_direction = y_direction;

    auto position = getPosition();
    auto prev_position = position;

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

    auto player_bounds = getBoundingRect();
    for (auto& rect : collision_rects) {

      auto obstacle_bounds = rect.getGlobalBounds();
      if (player_bounds.intersects(obstacle_bounds)) {
        std::cout << "Collision" << std::endl;

        // Right collision
        if (player_bounds.left < obstacle_bounds.left
          && player_bounds.left + player_bounds.width < obstacle_bounds.left + obstacle_bounds.width
          && player_bounds.top  < obstacle_bounds.top + obstacle_bounds.height
          && player_bounds.top + player_bounds.height > obstacle_bounds.top) {

          position.x = prev_position.x;
          //position.x = obstacle_bounds.left - player_bounds.width;
          //position.y = player_bounds.top;
        }

        // Left collision
        if (player_bounds.left > obstacle_bounds.left
          && player_bounds.left + player_bounds.width > obstacle_bounds.left + obstacle_bounds.width
          && player_bounds.top < obstacle_bounds.top + obstacle_bounds.height
          && player_bounds.top + player_bounds.height > obstacle_bounds.top) {

          position.x = prev_position.x;
          //position.x = obstacle_bounds.left + player_bounds.width;
          //position.y = player_bounds.top;
        }

        // Bottom collision
        if (player_bounds.top < obstacle_bounds.top
          && player_bounds.top + player_bounds.height < obstacle_bounds.top + obstacle_bounds.height
          && player_bounds.left  < obstacle_bounds.left + obstacle_bounds.width
          && player_bounds.left + player_bounds.width > obstacle_bounds.left) {

          position.y = prev_position.y;
          //position.x = play_bounds.left;
          //position.y = obstacle_bounds.top - player_bounds.height;
        }

        // Top collision
        if (player_bounds.top > obstacle_bounds.top
          && player_bounds.top + player_bounds.height > obstacle_bounds.top + obstacle_bounds.height
          && player_bounds.left  < obstacle_bounds.left + obstacle_bounds.width
          && player_bounds.left + player_bounds.width > obstacle_bounds.left) {

          position.y = prev_position.y;
          //position.x = play_bounds.left;
          //position.y = obstacle_bounds.top + player_bounds.height;
        }
      }
    }

    setPosition(position);
  }
};

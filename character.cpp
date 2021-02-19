#include "character.hpp"
#include "play_state.hpp"

namespace tol {

Character::Character(
  const fs::path& path, const std::shared_ptr<AssetCache> asset_cache_, const std::shared_ptr<Stats> stats,
  const std::string& name, std::vector<Attack>&& attacks):
  asset_cache(asset_cache_),
  _name(name), character_texture(path), _attacks(attacks), _stats(stats) {
  sprite.setTexture(*asset_cache->load_texture(path));
  sprite.setTextureRect({ 0, 0, TILE_SIZE, TILE_SIZE });
  sprite.setOrigin({ TILE_SIZE / 2.f, TILE_SIZE - 6.f });

  effect.setTexture(*asset_cache->load_texture("tilesets/effects.png"));
  effect.setOrigin({ EFFECT_TILE_SIZE / 2.f, EFFECT_TILE_SIZE / 2.f });
}

const std::string& Character::name() const {
  return _name;
}

const sf::Texture& Character::texture() const {
  return *sprite.getTexture();
}

const std::vector<Attack>& Character::attacks() const {
  return _attacks;
}

void Character::clear_attacks() {
  _attacks.clear();
}

void Character::add_attack(Attack&& attack) {
  _attacks.emplace_back(attack);
}

void Character::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  if (animation) {
    sprite.setTextureRect(animation->drawing_rect(now));
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

  auto bounding_box_rect = bounds();
  sf::RectangleShape bounding_box;
  bounding_box.setSize({ bounding_box_rect.width, bounding_box_rect.height });
  bounding_box.setOutlineThickness(0.5f);
  bounding_box.setOutlineColor(sf::Color::Red);
  bounding_box.setFillColor(sf::Color::Transparent);
  bounding_box.setPosition({ bounding_box_rect.left * scale.x, bounding_box_rect.top * scale.y });
  bounding_box.setScale(scale);

  auto texture_bounding_box_rect = texture_bounds();
  sf::RectangleShape texture_bounding_box;
  texture_bounding_box.setSize({ texture_bounding_box_rect.width, texture_bounding_box_rect.height });
  texture_bounding_box.setOutlineThickness(0.5f);
  texture_bounding_box.setOutlineColor(sf::Color::Green);
  texture_bounding_box.setFillColor(sf::Color::Transparent);
  texture_bounding_box.setPosition(
    { texture_bounding_box_rect.left * scale.x, texture_bounding_box_rect.top * scale.y });
  texture_bounding_box.setScale(scale);

  target.draw(shadow);
  target.draw(sprite);
  target.draw(bounding_box);
  target.draw(texture_bounding_box);

  if (current_effect) {
    effect.setTextureRect(*current_effect);
    effect.setPosition({ getPosition().x * scale.x, (getPosition().y - EFFECT_TILE_SIZE * 1.5f) * scale.y });
    effect.setScale({ scale.x, scale.y });
    target.draw(effect);
  }
}

sf::FloatRect Character::bounds() const {
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

sf::FloatRect Character::texture_bounds() const {
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

std::vector<sf::RectangleShape> Character::move(
  std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed,
  std::chrono::milliseconds now, PlayState& play_state, const sf::Vector2f& map_size, Info& info) {
  const auto speed_adjusted = speed * (stats().speed().get() / 10.0f);

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

  const auto stop_movement = [this, &x_direction, &y_direction, now](CharacterDirection direction) {
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

    this->last_collision = now;
  };

  const auto player_bounds = bounds();

  auto next_bounds = player_bounds;
  next_bounds.left += velocity.x;
  next_bounds.top += velocity.y;

  const auto player_width = player_bounds.width;
  const auto player_height = player_bounds.height;
  const auto player_left = player_bounds.left;
  const auto player_right = player_left + player_width;
  const auto player_top = player_bounds.top;
  const auto player_bottom = player_top + player_height;

  const auto collisions = play_state.map().collisions_around(next_bounds);

  std::vector<sf::RectangleShape> shapes;

  auto create_collision_shape = [](const sf::FloatRect& rect) {
    sf::RectangleShape shape({ rect.width, rect.height });
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Red);

    shape.setOutlineThickness(0.5f);
    shape.setPosition({ rect.left, rect.top });
    return shape;
  };

  auto in_city = false;
  for (auto& collision: collisions) {
    auto shape = create_collision_shape(collision.bounds);

    auto collided = collision.bounds.intersects(next_bounds);

    if (collision.city && collided) {
      in_city = true;
    }

    if (collision.unlock_condition && play_state.check_unlock_condition(*collision.unlock_condition, collided)) {
      shape.setOutlineColor(sf::Color::Green);
    } else if (collided && !collision.city) {
      if (collision.unlock_hint) {
        info.display_info(*collision.unlock_hint, std::chrono::seconds(10));
      }

      shape.setFillColor(sf::Color(255, 0, 0, 100));

      const auto obstacle_left = collision.bounds.left;
      const auto obstacle_right = obstacle_left + collision.bounds.width;
      const auto obstacle_top = collision.bounds.top;
      const auto obstacle_bottom = obstacle_top + collision.bounds.height;

      // Left collision
      if (
        player_left > obstacle_left && player_right > obstacle_right && player_top < obstacle_bottom &&
        player_bottom > obstacle_top) {
        next_bounds.left = obstacle_right;
        stop_movement(LEFT);
      }

      // Right collision
      if (
        player_left < obstacle_left && player_right < obstacle_right && player_top < obstacle_bottom &&
        player_bottom > obstacle_top) {
        next_bounds.left = obstacle_left - player_width;
        stop_movement(RIGHT);
      }

      // Top collision
      if (
        player_top > obstacle_top && player_bottom > obstacle_bottom && player_left < obstacle_right &&
        player_right > obstacle_left) {
        next_bounds.top = obstacle_bottom;
        stop_movement(UP);
      }

      // Bottom collision
      if (
        player_top < obstacle_top && player_bottom < obstacle_bottom && player_left < obstacle_right &&
        player_right > obstacle_left) {
        next_bounds.top = obstacle_top - player_height;
        stop_movement(DOWN);
      }
    }

    shapes.emplace_back(std::move(shape));
  }

  if (in_city) {
    play_state.getMusic().play_city();
  } else {
    play_state.getMusic().play_default();
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
        frames.push_back(std::make_tuple(
          std::chrono::milliseconds(100),
          sf::IntRect{ i * TILE_SIZE, (animation_type * 4 + *direction) * TILE_SIZE, TILE_SIZE, TILE_SIZE }));
      }

      animation = Animation(std::move(frames));
    }
  } else {
    animation = std::nullopt;
  }

  this->now = now;

  return shapes;
}

std::optional<float> Character::z_index() const {
  const auto& texture_bounding_rect = texture_bounds();
  return texture_bounding_rect.top + texture_bounding_rect.height;
}

float Character::distance_to(const Character& other) {
  const auto a = getPosition();
  const auto b = other.getPosition();

  return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

void Character::set_effect_rect(sf::IntRect effect) {
  current_effect = effect;
}

void Character::reset_effect() {
  current_effect = std::nullopt;
}

void Character::lookToward(const sf::Vector2f point) {
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

} // namespace tol

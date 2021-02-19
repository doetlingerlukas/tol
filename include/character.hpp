#pragma once
#define TOL_CHARACTER_HPP

#include <optional>

#include <SFML/Audio.hpp>

#include "animation.hpp"
#include "attack.hpp"
#include "overlay/info.hpp"
#include "stats.hpp"
#include "z_indexable.hpp"

namespace tol {

class PlayState;

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

const std::array<int, 6> CHARACTER_ANIMATION_FRAMES{
  7, 8, 9, 6, 13, 6,
};

const int TILE_SIZE = 64;
const int EFFECT_TILE_SIZE = 32;

class Character: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  mutable sf::Sprite sprite;
  std::optional<Animation> animation;
  std::chrono::milliseconds last_collision = std::chrono::milliseconds(0);

  mutable sf::Sprite effect;

  std::optional<sf::IntRect> current_effect;

  std::string _name;
  fs::path character_texture;

  std::vector<Attack> _attacks;

  mutable CharacterDirection last_direction = DOWN;

  protected:
  std::chrono::milliseconds now = std::chrono::milliseconds(0);
  std::shared_ptr<Stats> _stats;

  public:
  Character(
    const fs::path& path, std::shared_ptr<AssetCache> asset_cache_, std::shared_ptr<Stats> stats,
    const std::string& name, std::vector<Attack>&& attacks);

  inline const Stats& stats() const {
    return *_stats;
  }

  inline Stats& stats() {
    return *_stats;
  }

  const std::string& name() const;
  const std::vector<Attack>& attacks() const;
  void add_attack(Attack&& attack);

  const sf::Texture& texture() const;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  sf::FloatRect bounds() const;

  sf::FloatRect texture_bounds() const;

  std::vector<sf::RectangleShape> move(
    std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed,
    std::chrono::milliseconds now, PlayState& play_state, const sf::Vector2f& map_size, Info& info);

  virtual std::optional<float> z_index() const;

  float distance_to(const Character& other);

  void set_effect_rect(sf::IntRect effect);
  void reset_effect();

  void lookToward(sf::Vector2f point);
};

} // namespace tol

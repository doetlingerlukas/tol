#pragma once
#define TOL_CHARACTER_HPP

#include <optional>

#include <SFML/Audio.hpp>

#include <animation.hpp>
#include <attack.hpp>
#include <overlay/info.hpp>
#include <stats.hpp>
#include <z_indexable.hpp>

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

  std::string name;
  fs::path character_texture;

  std::vector<Attack> attacks;

  mutable CharacterDirection last_direction = DOWN;

  protected:
  std::chrono::milliseconds now = std::chrono::milliseconds(0);
  std::shared_ptr<Stats> stats;

  public:
  Character(
    const fs::path& path, std::shared_ptr<AssetCache> asset_cache_, std::shared_ptr<Stats> stats_,
    const std::string& name_, std::vector<Attack>&& attacks_);

  inline std::shared_ptr<Stats> getStats() {
    return stats;
  }

  fs::path getCharacterTexture();

  std::string getName() const;
  std::vector<Attack> getAttacks() const;
  void addAttack(Attack&& attack);

  fs::path getCharacterTexture() const;

  std::shared_ptr<Stats> getStats() const;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  sf::FloatRect getBoundingRect() const;

  sf::FloatRect getTextureBoundingRect() const;

  std::vector<sf::RectangleShape> move(
    std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed,
    std::chrono::milliseconds now, PlayState& play_state, const sf::Vector2f& map_size, Info& info);

  virtual std::optional<float> zIndex() const;

  float distanceTo(const Character& other);

  void setEffectRect(sf::IntRect effect);

  void resetEffect();

  void lookToward(sf::Vector2f point);
};

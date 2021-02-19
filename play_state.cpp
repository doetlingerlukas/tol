#include "play_state.hpp"

namespace tol {

void PlayState::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  target.setView(map_view);
  target.draw(map);

  for (auto shape: collision_shapes) {
    shape.setScale(scale);
    auto position = shape.getPosition();
    shape.setPosition({ position.x * scale.x, position.y * scale.y });
    target.draw(shape);
  }

  auto center = map_view.getCenter();
  const auto& position = getPlayer().getPosition();

  auto ss =
    fmt::format("Center Coords: {:.1f}, {:.1f}\nPlayer: {:.1f}, {:.1f}\n", center.x, center.y, position.x, position.y);

  sf::Text text;
  text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
  text.setCharacterSize(static_cast<unsigned int>(16 * scale.y));
  text.setFillColor(sf::Color::White);
  text.setOutlineColor(sf::Color::Black);
  text.setOutlineThickness(1);
  text.setString(ss);

  target.setView(target.getDefaultView());
  target.draw(text);
}

PlayState::PlayState(
  TiledMap& map_, Protagonist& player_, QuestStack& quest_stack_, std::shared_ptr<AssetCache> asset_cache_,
  const sf::Vector2f& scale_, const sf::Vector2u& window_size):
  asset_cache(asset_cache_),
  map(map_), player(player_), quest_stack(quest_stack_), scale(scale_) {
  map_view.reset({ 0, (getMap().getSize().y - window_size.y) * scale_.y, (float)window_size.x, (float)window_size.y });

  const auto spawn = getMap().getSpawn();
  if (spawn) {
    map_view.setCenter({ spawn->x * scale_.x, spawn->y * scale_.y });
    player_.setPosition(*spawn);
  }
}

GameState PlayState::update(
  KeyInput& key_input, const sf::RenderWindow& window, const std::chrono::milliseconds& now, float dt,
  std::optional<std::string>& npc_dialog, Info& info) {
  auto state = GameState::PLAY;
  const auto window_size = window.getSize();
  map_view.setSize({ static_cast<float>(window_size.x), static_cast<float>(window_size.y) });

  collision_shapes = getPlayer().move(
    (key_input.a && !key_input.d) ? std::optional(LEFT)
                                  : ((key_input.d && !key_input.a) ? std::optional(RIGHT) : std::nullopt),
    (key_input.w && !key_input.s) ? std::optional(UP)
                                  : ((key_input.s && !key_input.w) ? std::optional(DOWN) : std::nullopt),
    dt * CHARACTER_MOVE_SPEED, now, *this, getMap().getCollectibles(), getMap().getSize(), info);

  if (key_input.up && !key_input.down) {
    direction.y = std::clamp(direction.y + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
  } else if (key_input.down && !key_input.up) {
    direction.y = std::clamp(direction.y - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
  } else {
    if (direction.y >= 0.5) {
      direction.y -= 1.0 * dt * VIEW_MOVE_DECEL;
    } else if (direction.y <= -0.5) {
      direction.y += 1.0 * dt * VIEW_MOVE_DECEL;
    } else {
      direction.y = 0;
    }
  }

  if (key_input.right && !key_input.left) {
    direction.x = std::clamp(direction.x - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
  } else if (key_input.left && !key_input.right) {
    direction.x = std::clamp(direction.x + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
  } else {
    if (direction.x >= 0.5) {
      direction.x -= 1.0 * dt * VIEW_MOVE_DECEL;
    } else if (direction.x <= -0.5) {
      direction.x += 1.0 * dt * VIEW_MOVE_DECEL;
    } else {
      direction.x = 0;
    }
  }

  for (auto& npc: getMap().getNpcs()) {
    const auto dist = getPlayer().distanceTo(npc);
    const auto tileDiagonal = std::sqrt(std::pow(getMap().getTileSize().x, 2) + std::pow(getMap().getTileSize().y, 2));

    if (dist < tileDiagonal) {
      npc.lookToward(getPlayer().getPosition());
      npc.setEffectRect({ 480, 192, EFFECT_TILE_SIZE, EFFECT_TILE_SIZE });

      if (key_input.e) {
        npc_dialog = npc.getName();
        state = GameState::DIALOG;
      }
    } else {
      npc.resetEffect();
    }
  }

  map_view.setCenter(getMap().getView(window.getSize().x, window.getSize().y));
  getMap().update(map_view, window, now);

  return state;
}

bool PlayState::check_unlock_condition(const std::string& condition_name, bool collided) const {
  if (condition_name == "bridge_gate") {
    return getQuestStack().completed(0);
  }

  if (condition_name == "city_gate_1") {
    if (getQuestStack().completed(1)) {
      if (collided && !getQuestStack().completed(2) && getQuestStack().getSelected() != 2) {
        getQuestStack().select(2);
      }

      return true;
    }
  }

  if (condition_name == "city_gate_2") {
    return getQuestStack().completed(2);
  }

  return false;
}

sf::Vector2f PlayState::player_position() const {
  return getPlayer().getPosition();
}

void PlayState::set_inventory(const json& inventory_array) {
  auto& inventory = getPlayer().inventory();
  auto& collectibles = getMap().getCollectibles();

  for (const auto& id: inventory_array) {
    auto item = collectibles.at(id.get<int>());
    inventory.add(std::make_pair(id, item));
    collectibles.erase(id.get<int>());
  }
}

void PlayState::set_player_position(sf::Vector2f pos) {
  getPlayer().setPosition(pos);
}

} // namespace tol

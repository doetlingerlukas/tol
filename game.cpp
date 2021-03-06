#if __APPLE__
#include <CoreGraphics/CGDisplayConfiguration.h>
#endif

#include "game.hpp"

namespace tol {

void Game::handle_event(
  sf::Event& event, KeyInput& key_input, PlayState& play_state, tol::Music& music, Inventory& inventory,
  Overlay& overlay, Fight& fight) {
  switch (event.type) {
    case sf::Event::Closed:
      set_state(GameState::QUIT);
      break;
    case sf::Event::MouseButtonPressed:
    case sf::Event::MouseButtonReleased:
      mouse_pressed = event.type == sf::Event::MouseButtonPressed;

      if (event.mouseButton.button == sf::Mouse::Button::Left) {
        const auto& state = instance().state();
        if (state == GameState::INVENTORY) {
          inventory.mouse(
            { static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y) }, mouse_pressed);
        } else if (state == GameState::OVERLAY) {
          overlay.mouse(
            { static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y) }, mouse_pressed);
        } else {
          mouse_pressed = false;
        }
      }
      break;

    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased: {
      switch (event.key.code) {
        case sf::Keyboard::Escape:
          if (event.type == sf::Event::KeyPressed) {
            switch (instance().state()) {
              case GameState::INVENTORY:
              case GameState::OVERLAY:
                instance().set_state(GameState::PLAY);
                break;
              case GameState::DEAD:
                break;
              default:
                set_state(GameState::MENU);
                break;
            }
          }
          break;
        case sf::Keyboard::Enter: {
          fight.enter(event.type == sf::Event::KeyPressed);
          key_input.enter = event.type == sf::Event::KeyPressed;
          break;
        }
        case sf::Keyboard::Right:
          key_input.right = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::D:
          key_input.d = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::Left:
          key_input.left = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::A:
          key_input.a = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::Up: {
          if (event.type == sf::Event::KeyPressed) {
            fight.up();
          }
          key_input.up = event.type == sf::Event::KeyPressed;
          break;
        }
        case sf::Keyboard::W:
          key_input.w = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::Down: {
          if (event.type == sf::Event::KeyPressed) {
            fight.down();
          }
          key_input.down = event.type == sf::Event::KeyPressed;
          break;
        }
        case sf::Keyboard::S:
          key_input.s = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::E:
          key_input.e = event.type == sf::Event::KeyPressed;
          break;
        case sf::Keyboard::H:
          if (event.type == sf::Event::KeyPressed) {
            play_state.set_debug(!play_state.debug());
          }

          break;
        case sf::Keyboard::I:
          if (event.type == sf::Event::KeyReleased) {
            if (instance().state() == GameState::INVENTORY) {
              instance().set_state(GameState::PLAY);
            } else {
              instance().set_state(GameState::INVENTORY);
            }
          }
          break;
        case sf::Keyboard::Tab:
          if (event.type == sf::Event::KeyReleased) {
            if (instance().state() == GameState::OVERLAY) {
              instance().set_state(GameState::PLAY);
            } else {
              instance().set_state(GameState::OVERLAY);
            }
          }
          break;
        case sf::Keyboard::X:
          if (instance().state() == GameState::INVENTORY && event.type == sf::Event::KeyReleased) {
            inventory.drop_selected(player, map);
          }
          break;
        case sf::Keyboard::C:
          if (instance().state() == GameState::INVENTORY && event.type == sf::Event::KeyReleased) {
            const auto callback = [&play_state](int id) { play_state.add_used_collectibles(id); };
            const auto message = inventory.use_selected(player, callback);
            if (message) {
              info.display_info(*message, std::chrono::seconds(5));
            }
          }
          break;
        default:
          break;
      }

      break;
    }
    default:
      break;
  }

  nk_sfml_handle_event(&event);
}

void Game::handle_settings_update(tol::Music& music) {
  window.setVerticalSyncEnabled(settings.vsync());
  music.set_volume(settings.volume_level);

  const auto [window_width, window_height] = settings.resolution();

  if (settings.fullscreen() && window_style != sf::Style::Fullscreen) {
    window_style = sf::Style::Fullscreen;
  }

  if (!settings.fullscreen() && window_style == sf::Style::Fullscreen) {
    window_style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
  }

  window.create(
    sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);

  instance().set_settings_changed(false);
}

Game::Game(fs::path dir_, Settings& settings_):
  dir(dir_), settings(settings_), _instance(GameInstance(dir_)),
  asset_cache(std::make_shared<AssetCache>(dir_ / "assets")), info(asset_cache), map("map.json", asset_cache),
  player(Protagonist(
    fs::path("tilesets/character-ruby.png"), asset_cache, std::make_shared<Stats>(instance().load_stats()),
    instance().load_attacks(), "Ruby")),
  mouse_pressed(false) {
  scale = { 2.0, 2.0 };
  resolution_scale = { 1.0, 1.0 };

  auto video_mode = sf::VideoMode::getDesktopMode();
  fmt::print("Full Resolution: {}, {}\n", video_mode.width, video_mode.height);

#if __APPLE__
  auto display_id = CGMainDisplayID();
  auto width = CGDisplayPixelsWide(display_id);
  auto height = CGDisplayPixelsHigh(display_id);
  fmt::print("Scaled Resolution: {}, {}\n", width, height);

  resolution_scale.x = static_cast<float>(video_mode.width) / static_cast<float>(width);
  resolution_scale.y = static_cast<float>(video_mode.height) / static_cast<float>(height);
#endif

  scale.x *= resolution_scale.x;
  scale.y *= resolution_scale.y;

  if (settings.fullscreen()) {
    window_style = sf::Style::Fullscreen;
  } else {
    window_style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
  }
}

void Game::run() {
  const auto [window_width, window_height] = settings.resolution();

  window.create(
    sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);
  window.setVerticalSyncEnabled(settings.vsync());
  window.requestFocus();
  window.setKeyRepeatEnabled(false);

  map.setScale(scale);
  player.setScale(scale);

  map.set_player(&player);

  Music music(fs::path("assets/music"), settings.volume_level);

  QuestStack quest_stack(info);
  PlayState play_state(map, player, quest_stack, asset_cache, scale, window.getSize(), music);
  Overlay overlay(asset_cache, std::cref(player.stats()), quest_stack);
  std::reference_wrapper<Inventory> inventory = player.inventory();

  instance().load(play_state);

  KeyInput key_input;

  info.display_info(
    "Welcome to a very loost island with some very loost "
    "people, who are doing very loost things!",
    std::chrono::seconds(10));

  instance().load(play_state);
  play_state.set_inventory(instance().load_inventory());

  sf::Clock clock;
  std::chrono::milliseconds now = std::chrono::milliseconds(0);

  const std::shared_ptr<Nuklear> nuklear =
    std::make_shared<Nuklear>(window.getSize(), player.stats(), asset_cache, &window);
  auto dialog = Dialog(nuklear);

  nuklear->setScale(scale);

  std::optional<std::string> last_npc_interaction;

  Fight fight(asset_cache, player);

  music.play_default();

  while (window.isOpen()) {
    const auto millis = clock.getElapsedTime().asMilliseconds();
    if (millis > 0) {
      clock.restart();
    }

    const auto dt = millis / 1000.f;
    now += std::chrono::milliseconds(millis);

    sf::Event event;
    nk_input_begin(nuklear->ctx());
    while (window.pollEvent(event)) {
      handle_event(event, key_input, play_state, music, inventory, overlay, fight);
    }
    nk_input_end(nuklear->ctx());

    if (instance().isSettingsChanged()) {
      handle_settings_update(music);
      nuklear->setSize(window.getSize());
    }

    quest_stack.check(play_state);

    window.clear();
    window.resetGLStates();

    switch (_state) {
      case GameState::PLAY:
        switch (instance().state()) {
          case GameState::INVENTORY:
            window.draw(play_state);
            info.update_time(std::chrono::milliseconds(millis));
            window.draw(info);
            window.draw(inventory);
            break;
          case GameState::OVERLAY:
            window.draw(play_state);
            info.update_time(std::chrono::milliseconds(millis));
            window.draw(info);
            window.draw(overlay);
            break;
          case GameState::FIGHT:
            instance().set_state(fight.with(now, last_npc_interaction, map, info));

            if (instance().state() == GameState::FIGHT)
              window.draw(fight);
            break;
          case GameState::DEAD:
            nuklear->render_death(*this, play_state);
            break;
          case GameState::PLAY:
          case GameState::QUEST:
            instance().set_state(play_state.update(key_input, window, now, dt, last_npc_interaction, info));
            window.draw(play_state);

            info.update_time(std::chrono::milliseconds(millis));
            window.draw(info);

            nuklear->render_hud();
            break;
          case GameState::DIALOG:
            window.draw(play_state);

            if (last_npc_interaction) {
              auto [state, quest] = dialog.show(*last_npc_interaction);
              instance().set_state(state);

              if (state == GameState::QUEST) {
                quest_stack.select(quest);
              }
            }
            break;
          default:
            break;
        }
        break;
      case GameState::MENU:
        nuklear->render_menu(*this, play_state);
        break;
      case GameState::SETTINGS:
        nuklear->render_settings(*this, settings);
        break;
      case GameState::QUIT:
        window.close();
        break;
      default:
        break;
    }

    window.display();
  }

  nk_sfml_shutdown();
}

} // namespace tol

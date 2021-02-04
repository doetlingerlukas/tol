#include <functional>

struct MenuItem {
  std::string title;
  std::function<void()> callback;

  MenuItem(std::string title_, std::function<void()> callback_): title(title_), callback(callback_) {}
};

class Menu: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  size_t current_item = 0;
  std::vector<MenuItem> items;
  bool enter_pressed = false;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::RectangleShape menu_background;
    menu_background.setSize({ (float)target.getSize().x, (float)target.getSize().y });
    menu_background.setFillColor(sf::Color(0, 0, 0, 200));
    target.draw(menu_background);

    for (size_t i = 0; i < items.size(); i++) {
      sf::Text text;

      const auto character_size = 32 * getScale().y;

      text.setString(items[i].title);
      text.setCharacterSize(character_size);

      if (i == current_item) {
        text.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

        text.setStyle(sf::Text::Bold);
        text.setFillColor(sf::Color::White);

        if (enter_pressed) {
          text.setFillColor(sf::Color(50, 200, 100, 255));
        }
      }
      else {
        text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
        text.setFillColor(sf::Color(200, 200, 200, 255));
      }

      text.setPosition({ character_size, static_cast<float>(character_size + i * character_size) });

      target.draw(text);
    }
  }

public:
  Menu(const std::shared_ptr<AssetCache> asset_cache_): asset_cache(asset_cache_) {}

  void up() {
    if (current_item > 0) {
      current_item--;
    }
  }

  void down() {
    if (current_item + 1 < items.size()) {
      current_item++;
    }
  }

  void add_item(std::string title, std::function<void()> callback) {
    MenuItem item(title, callback);
    items.push_back(item);
  }

  mutable std::map<std::string, std::shared_ptr<sf::Font>> fonts;

  std::shared_ptr<sf::Font> load_font(const fs::path& path) const {
    if (fonts.count(path.string()) == 0) {
      auto font = std::make_shared<sf::Font>();

      if (fs::exists(path) && fs::is_regular_file(path) && font->loadFromFile(path.string())) {
        fonts[path.string()] = std::move(font);
        return fonts.at(path.string());
      }

      throw std::runtime_error("Failed to load font '" + path.string() + "'.");
    } else {
      return fonts.at(path.string());
    }
  }

  void enter(bool pressed) {
    enter_pressed = pressed;

    if (!pressed) {
      const auto item = items[current_item];
      item.callback();
    }
  }
};

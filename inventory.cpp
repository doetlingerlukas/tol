#include "inventory.hpp"

namespace tol {

void Inventory::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
  sf::FloatRect inventory_dims(
    ((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.1f, target_size.x, target_size.y * 0.85f);
  sf::FloatRect detail(inventory_dims.left, inventory_dims.top, inventory_dims.width * 0.29f, inventory_dims.height);
  sf::FloatRect objects(
    detail.left + inventory_dims.width * 0.3f, inventory_dims.top, inventory_dims.width * 0.7f, inventory_dims.height);
  const auto font = *asset_cache->load_font("fonts/Gaegu-Regular.ttf");

  sf::Text header;
  header.setFont(font);
  header.setStyle(sf::Text::Style::Bold);
  header.setFillColor(sf::Color::White);
  header.setCharacterSize(60);
  header.setOutlineColor(sf::Color::Black);
  header.setOutlineThickness(4.f);
  header.setString("Inventory");
  header.setPosition({ ((float)target.getSize().x - header.getGlobalBounds().width) / 2, 0 });
  target.draw(header);

  sf::RectangleShape detail_box;
  detail_box.setFillColor(sf::Color(0, 0, 0, 175));
  detail_box.setOutlineColor(sf::Color::Blue);
  detail_box.setOutlineThickness(2.f);
  detail_box.setSize({ detail.width, detail.height });
  detail_box.setPosition({ detail.left, detail.top });

  sf::RectangleShape objects_box;
  objects_box.setFillColor(sf::Color(0, 0, 0, 175));
  objects_box.setOutlineColor(sf::Color::Blue);
  objects_box.setOutlineThickness(2.f);
  objects_box.setSize({ objects.width, objects.height });
  objects_box.setPosition({ objects.left, objects.top });

  target.draw(detail_box);
  target.draw(objects_box);

  sf::Vector2f margin({ 30, 30 });
  sf::Vector2f scale({ 3.f, 3.f });

  auto i = 0;
  // Offsets for placement.
  auto h = 0;
  auto w = 0;
  for (auto [id, item]: _items) {
    item.setScale(scale);

    auto rect = item.bounds();
    sf::Vector2f bounding_size({ rect.width * scale.x, rect.height * scale.y });
    sf::RectangleShape bounding_box(bounding_size);
    if (margin.x + w + bounding_size.x + margin.x > objects.width) {
      h += margin.y + bounding_size.y;
      w = 0;
    }
    bounding_box.setPosition({ objects.left + margin.x + w, objects.top + margin.y + h });
    if (bounding_box.getGlobalBounds().contains(mouse_location) && mouse_pressed) {
      selected = i;
    }
    bounding_box.setFillColor(sf::Color(0, 0, 0, 220));
    bounding_box.setOutlineColor(i == selected ? sf::Color::White : sf::Color::Black);
    bounding_box.setOutlineThickness(2.f);
    target.draw(bounding_box);

    item.setPosition({ (objects.left + margin.x + w) / scale.x, (objects.top + margin.y + h) / scale.y });
    target.draw(item);

    w += margin.x + bounding_size.x;
    i++;
  }

  const auto display_text = [&target, &font](const std::string& text_, sf::Vector2f pos) {
    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::White);
    text.setString(text_);
    text.setPosition(pos);
    target.draw(text);
  };

  if (_items.size() > 0 && selected) {
    const auto& [id, item] = _items.at(*selected);

    sf::Vector2f info_pos({ detail.left + margin.x / 2, detail.top + margin.y / 2 });
    auto max_line_width = detail.width - margin.x;

    if (item.usable()) {
      display_text("<C> use item", { info_pos.x, detail.top + detail.height - 4 * margin.y });
    }

    display_text("<X> drop item", { info_pos.x, detail.top + detail.height - 2 * margin.y });

    std::istringstream iss(item.description());
    const std::vector<std::string> words(
      { std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} });

    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::White);
    text.setPosition(info_pos);

    std::vector<sf::Text> lines;
    lines.push_back(text);
    for (auto& word: words) {
      if (lines.back().getGlobalBounds().width + word.size() * 14 > max_line_width) {
        lines.push_back(text);
        lines.back().setPosition({ info_pos.x, info_pos.y + text.getCharacterSize() * (lines.size() - 1) });
      }
      std::string current = lines.back().getString();
      lines.back().setString(current + " " + word);
    }

    for (auto& line: lines) {
      target.draw(line);
    }
  }
}

Inventory::Inventory(size_t max_size_, const std::shared_ptr<AssetCache> asset_cache_):
  asset_cache(asset_cache_), max_size(max_size_), mouse_pressed(false) {}

[[nodiscard]] bool Inventory::empty() const {
  return _items.empty();
}

[[nodiscard]] size_t Inventory::size() const {
  return _items.size();
}

[[nodiscard]] const std::vector<std::pair<int, Object>>& Inventory::items() const {
  return _items;
}

bool Inventory::add(std::pair<int, Object> item) {
  if (size() < max_size) {
    _items.push_back(item);
    return true;
  }

  return false;
}

std::pair<int, Object> Inventory::remove(size_t index) {
  auto item = _items[index];
  _items.erase(_items.cbegin() + index);
  return item;
}

void Inventory::mouse(sf::Vector2f location, bool pressed) {
  mouse_location = location;
  mouse_pressed = pressed;
}

void Inventory::drop_selected(Protagonist& player, Map& map) {
  if (selected) {
    auto [id, collectible] = remove(*selected);

    auto new_position = player.getPosition();
    new_position.x -= collectible.bounds().width / 2.f;
    new_position.y -= collectible.bounds().height / 2.f;
    collectible.setPosition(new_position);

    map.collectibles().emplace(std::make_pair(id, collectible));
    select_next();

    player.drop_item();
  }
}

std::optional<std::string> Inventory::use_selected(Protagonist& player, std::function<void(int)> callback) {
  if (selected) {
    if (_items[*selected].second.usable()) {
      auto pair = remove(*selected);
      const auto& add_to_used = callback;
      add_to_used(pair.first);
      const auto message = player.use_item(pair);
      select_next();
      return message;
    }
  }

  return std::nullopt;
}

void Inventory::select_next() {
  if (*selected < size()) {
    return;
  } else if (*selected > 0) {
    (*selected)--;
  } else {
    selected = std::nullopt;
  }
}

} // namespace tol

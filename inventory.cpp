#include "inventory.hpp"

namespace tol {

void Inventory::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
  sf::FloatRect inventory_dims(
    ((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.1f, target_size.x, target_size.y * 0.85f);
  sf::FloatRect detail(inventory_dims.left, inventory_dims.top, inventory_dims.width * 0.29f, inventory_dims.height);
  sf::FloatRect objects(
    detail.left + inventory_dims.width * 0.3f, inventory_dims.top, inventory_dims.width * 0.7f, inventory_dims.height);
  const auto font = *asset_cache->loadFont("fonts/Gaegu-Regular.ttf");

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
  for (auto [id, element]: elements) {
    element.setScale(scale);

    auto rect = element.getBoundingRect();
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

    element.setPosition({ (objects.left + margin.x + w) / scale.x, (objects.top + margin.y + h) / scale.y });
    target.draw(element);

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

  if (elements.size() > 0 && selected) {
    auto& [id, element] = elements.at(*selected);

    sf::Vector2f info_pos({ detail.left + margin.x / 2, detail.top + margin.y / 2 });
    auto max_line_width = detail.width - margin.x;

    if (element.usable()) {
      display_text("<C> use item", { info_pos.x, detail.top + detail.height - 4 * margin.y });
    }

    display_text("<X> drop item", { info_pos.x, detail.top + detail.height - 2 * margin.y });

    std::istringstream iss(Collectible::getCollectible(element.getName()).info());
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

Inventory::Inventory(int max_size_, const std::shared_ptr<AssetCache> asset_cache_):
  asset_cache(asset_cache_), max_size(max_size_), mouse_pressed(false) {}

[[nodiscard]] int Inventory::size() const {
  return elements.size();
}

std::vector<std::pair<int, Object>>& Inventory::getElements() {
  return elements;
}

bool Inventory::add(std::pair<int, Object> new_element) {
  if (size() < max_size) {
    elements.push_back(new_element);
    return true;
  }

  return false;
}

std::pair<int, Object> Inventory::remove(size_t index) {
  auto element = elements[index];
  elements.erase(elements.cbegin() + index);
  return element;
}

void Inventory::mouse(sf::Vector2f location, bool pressed) {
  mouse_location = location;
  mouse_pressed = pressed;
}

void Inventory::drop_selected(Protagonist& player, TiledMap& map) {
  if (selected) {
    auto [id, collectible] = remove(*selected);

    auto new_position = player.getPosition();
    new_position.x -= collectible.getBoundingRect().width / 2.f;
    new_position.y -= collectible.getBoundingRect().height / 2.f;
    collectible.setPosition(new_position);

    map.getCollectibles().emplace(std::make_pair(id, collectible));
    select_next();

    player.drop_item();
  }
}

std::optional<std::string> Inventory::use_selected(Protagonist& player) {
  if (selected) {
    if (elements[*selected].second.usable()) {
      const auto message = player.use_item(remove(*selected));
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

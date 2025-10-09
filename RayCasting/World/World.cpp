#include <print>

#include "World.hpp"

#include "Canvas.hpp"
#include "Map.hpp"
#include "Palette.hpp"
#include "Player.hpp"

namespace RC::World {

void start() {
    std::println("Starting world");
}

void draw() {
    Canvas::fill(Palette::GUNMETAL_GRAY_DARKER);
    Map::draw();
}

void update() {
    Player::update();
    Map::update();
}

} // namespace RC::World

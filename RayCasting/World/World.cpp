#include <chrono>

#include "World.hpp"

#include "Canvas.hpp"
#include "Map.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Text.hpp"
#include "Viewport.hpp"

namespace RC::World {

int64_t fps = 0;

void start() {
    Viewport::initialize();
}

void drawFPS() {
    Palette::color = Palette::white;
    Text::draw(4, 4, std::to_string(fps));
}

void draw() {
    Viewport::draw();
    Map::draw();
    drawFPS();
}

void updateFPS() {
    static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
    std::chrono::time_point currentTime = std::chrono::steady_clock::now();
    if (currentTime != lastTime) {
        std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
        fps = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)).count() / duration.count();
        lastTime = currentTime;
    }
}

void update() {
    Player::update();
    Viewport::update();
    Map::update();
    updateFPS();
}

} // namespace RC::World

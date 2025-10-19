#include <chrono>
#include <print>

#include "World.hpp"

#include "Canvas.hpp"
#include "Map.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Text.hpp"
#include "Viewport.hpp"

namespace RC::World {

int64_t framesPerSecond = 0;

void start() {
    std::println("Starting world");
    Viewport::initialize();
}

void drawFPS() {
    Palette::setColor(Palette::WHITE);
    Text::draw(4, 4, std::to_string(framesPerSecond));
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
        framesPerSecond = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)).count() / duration.count();
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

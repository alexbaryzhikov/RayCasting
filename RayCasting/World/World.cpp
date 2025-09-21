//
//  World.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#include <print>

#include "World.hpp"
#include "WorldPresenter.hpp"

namespace RC {

World world;
WorldPresenter presenter;

World& World::get() {
    return world;
}

void World::loadMap(const void* data, size_t size) {
    std::println("Loading map: {} size: {}", data, size);
}

void World::start() {
    std::println("Starting world");
}

void World::draw() {
    presenter.drawWorld();
}

} // namespace RC

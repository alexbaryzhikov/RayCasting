#include <limits>
#include <numbers>
#include <random>

#include "Config.h"
#include "MapGenerator.hpp"
#include "Player.hpp"

namespace RC::Map {

struct Position {
    int x;
    int y;
};

struct Rect {
    int x;
    int y;
    int w;
    int h;

    bool overlaps(const Rect& other) const {
        return (x < other.x + other.w &&
                x + w > other.x &&
                y < other.y + other.h &&
                y + h > other.y);
    }

    Position center() const {
        return {x + w / 2, y + h / 2};
    }
};

bool operator==(const Rect& a, const Rect& b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

std::mt19937 rng(MAP_SEED);

int getRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

int getRandomOdd(int min, int max) {
    if (max < min) std::swap(min, max);
    if (min % 2 == 0) min++;
    if (max % 2 == 0) max--;

    if (min > max) {
        // No odd number in range. e.g. [10, 10].
        // min becomes 11, max becomes 9.
        // Return the largest odd number <= original max.
        return max;
    }

    std::uniform_int_distribution<int> dist(0, (max - min) / 2);
    return min + dist(rng) * 2;
}

std::vector<Tile> generateWalls() {
    std::vector<Tile> walls(MAP_WIDTH * MAP_HEIGHT);
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            int idx = y * MAP_WIDTH + x;
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                walls[idx] = Tile::wallRock;
            } else {
                walls[idx] = Tile::wall;
            }
        }
    }
    return walls;
}

void carveRoom(const Rect& room, int& floorTiles) {
    for (int y = room.y; y < room.y + room.h; ++y) {
        for (int x = room.x; x < room.x + room.w; ++x) {
            if (tiles[y * MAP_WIDTH + x] == Tile::wall) {
                tiles[y * MAP_WIDTH + x] = Tile::floor;
                ++floorTiles;
            }
        }
    }
}

void generateIslands(const Rect& room, int& floorTiles) {
    int roomArea = room.w * room.h;
    if (roomArea < ISLAND_MIN_ROOM_AREA) return;

    // Probability of adding structures increases with size
    float structureChance = fmin(0.4f, (roomArea - 25) / 200.0f);

    auto roomCenter = room.center();

    for (int i = 0; i < ISLAND_MAX_LOOPS; ++i) {
        if (getRandomInt(0, 100) / 100.0f > structureChance) {
            continue; // This attempt failed
        }

        int max_w = fmax(1, room.w * ISLAND_MAX_SIZE_RATIO);
        int max_h = fmax(1, room.h * ISLAND_MAX_SIZE_RATIO);
        int island_w = getRandomInt(1, max_w);
        int island_h = getRandomInt(1, max_h);
        int island_x = getRandomInt(room.x, room.x + room.w - island_w);
        int island_y = getRandomInt(room.y, room.y + room.h - island_h);

        Rect newIsland = {island_x, island_y, island_w, island_h};

        // CRITICAL: Check if the island covers the room's center.
        // If it does, skip this island to ensure tunnels can connect.
        if (roomCenter.x >= newIsland.x && roomCenter.x < newIsland.x + newIsland.w &&
            roomCenter.y >= newIsland.y && roomCenter.y < newIsland.y + newIsland.h) {
            continue;
        }

        for (int y = newIsland.y; y < newIsland.y + newIsland.h; ++y) {
            for (int x = newIsland.x; x < newIsland.x + newIsland.w; ++x) {
                if (tiles[y * MAP_WIDTH + x] == Tile::floor) {
                    tiles[y * MAP_WIDTH + x] = Tile::wallTopBeam;
                    floorTiles--;
                }
            }
        }
    }
}

void safeSetWallTile(int x, int y, Tile newTile) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return;
    }
    if (tiles[y * MAP_WIDTH + x] == Tile::wall) {
        tiles[y * MAP_WIDTH + x] = newTile;
    }
}

void decorateRoomWalls(const Rect& room) {
    // Decorate horizontal walls (top/bottom)
    for (int x = room.x; x < room.x + room.w; ++x) {
        if ((x - 1) % 8 == 0) {
            safeSetWallTile(x, room.y - 1, Tile::wallVerticalBeam);
            safeSetWallTile(x, room.y + room.h, Tile::wallVerticalBeam);
        } else if ((x - 1) % 4 == 0) {
            safeSetWallTile(x, room.y - 1, Tile::wallWindow);
            safeSetWallTile(x, room.y + room.h, Tile::wallWindow);
        } else {
            safeSetWallTile(x, room.y - 1, Tile::wallTopBeam);
            safeSetWallTile(x, room.y + room.h, Tile::wallTopBeam);
        }
    }
    // Decorate vertical walls (left/right)
    for (int y = room.y; y < room.y + room.h; ++y) {
        if ((y - 1) % 8 == 0) {
            safeSetWallTile(room.x - 1, y, Tile::wallVerticalBeam);
            safeSetWallTile(room.x + room.w, y, Tile::wallVerticalBeam);
        } else if ((y - 1) % 4 == 0) {
            safeSetWallTile(room.x - 1, y, Tile::wallWindow);
            safeSetWallTile(room.x + room.w, y, Tile::wallWindow);
        } else {
            safeSetWallTile(room.x - 1, y, Tile::wallTopBeam);
            safeSetWallTile(room.x + room.w, y, Tile::wallTopBeam);
        }
    }
}

void carveTunnelH(int x1, int x2, int y, int& floorTiles) {
    if (x1 > x2) std::swap(x1, x2);
    for (int x = x1; x <= x2; ++x) {
        if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            if (tiles[y * MAP_WIDTH + x] != Tile::floor) {
                tiles[y * MAP_WIDTH + x] = Tile::floor;
                floorTiles++;
            }
            if (tiles[(y - 1) * MAP_WIDTH + x] == Tile::wall) {
                tiles[(y - 1) * MAP_WIDTH + x] = Tile::wallTopBeam;
            }
            if (tiles[(y + 1) * MAP_WIDTH + x] == Tile::wall) {
                tiles[(y + 1) * MAP_WIDTH + x] = Tile::wallTopBeam;
            }
        }
    }
}

void carveTunnelV(int y1, int y2, int x, int& floorTiles) {
    if (y1 > y2) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y) {
        if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            if (tiles[y * MAP_WIDTH + x] != Tile::floor) {
                tiles[y * MAP_WIDTH + x] = Tile::floor;
                floorTiles++;
            }
            if (tiles[y * MAP_WIDTH + x - 1] == Tile::wall) {
                tiles[y * MAP_WIDTH + x - 1] = Tile::wallTopBeam;
            }
            if (tiles[y * MAP_WIDTH + x + 1] == Tile::wall) {
                tiles[y * MAP_WIDTH + x + 1] = Tile::wallTopBeam;
            }
        }
    }
}

void carveTunnels(const std::vector<Rect>& rooms, const Rect& room, int& floorTiles) {
    // Connect the new room to the closest previous room
    auto center = room.center();

    const Rect* closestRoom = nullptr;
    int minDistanceSq = std::numeric_limits<int>::max();

    for (const auto& otherRoom : rooms) {
        if (otherRoom == room) continue;

        auto otherCenter = otherRoom.center();
        int dx = center.x - otherCenter.x;
        int dy = center.y - otherCenter.y;
        int distanceSq = dx * dx + dy * dy; // Use squared distance for efficiency

        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            closestRoom = &otherRoom;
        }
    }

    if (closestRoom) {
        auto closestCenter = closestRoom->center();

        // Randomly decide to carve horizontal or vertical tunnel first
        if (getRandomInt(0, 1) == 0) {
            carveTunnelH(closestCenter.x, center.x, closestCenter.y, floorTiles);
            carveTunnelV(closestCenter.y, center.y, center.x, floorTiles);
        } else {
            carveTunnelV(closestCenter.y, center.y, closestCenter.x, floorTiles);
            carveTunnelH(closestCenter.x, center.x, center.y, floorTiles);
        }
    }
}

std::vector<Rect> generateRooms() {
    std::vector<Rect> rooms;
    int floorTiles = 0;
    int totalTiles = MAP_WIDTH * MAP_HEIGHT;

    for (int i = 0; i < ROOM_MAX_LOOPS && static_cast<float>(floorTiles) / totalTiles < FLOOR_COVERAGE_TARGET; ++i) {
        // We use odd numbers for dimensions and positions to make tunnel connections easier
        int w = getRandomOdd(ROOM_MIN_SIZE, ROOM_MAX_SIZE);
        int h = getRandomOdd(ROOM_MIN_SIZE, ROOM_MAX_SIZE);
        int x = getRandomOdd(1, MAP_WIDTH - w - 1);
        int y = getRandomOdd(1, MAP_HEIGHT - h - 1);

        Rect newRoom = {x, y, w, h};

        bool overlaps = false;
        for (const auto& room : rooms) {
            if (newRoom.overlaps(room)) {
                overlaps = true;
                break;
            }
        }
        if (overlaps) continue;

        carveRoom(newRoom, floorTiles);
        decorateRoomWalls(newRoom);
        generateIslands(newRoom, floorTiles);
        carveTunnels(rooms, newRoom, floorTiles);
        rooms.push_back(newRoom);
    }
    return rooms;
}

void placePlayer(const std::vector<Rect>& rooms) {
    if (rooms.empty()) return;

    // Find the room closest to any corner of the map.
    const Rect* startRoom = nullptr;
    int minCornerDistSq = std::numeric_limits<int>::max();

    std::vector<Position> corners = {{0, 0}, {MAP_WIDTH - 1, 0}, {0, MAP_HEIGHT - 1}, {MAP_WIDTH - 1, MAP_HEIGHT - 1}};

    for (const auto& room : rooms) {
        auto center = room.center();
        int currentMinDistSq = std::numeric_limits<int>::max();

        for (const auto& corner : corners) {
            int dx = center.x - corner.x;
            int dy = center.y - corner.y;
            int distSq = dx * dx + dy * dy;
            if (distSq < currentMinDistSq) {
                currentMinDistSq = distSq;
            }
        }

        if (currentMinDistSq < minCornerDistSq) {
            minCornerDistSq = currentMinDistSq;
            startRoom = &room;
        }
    }

    // Place player at the center of the start room
    // The room's center is guaranteed to be floor, it is protected from island generation
    Position startPos = startRoom->center();
    Player::position.x = (startPos.x + 0.5f) * MAP_TILE_SIZE;
    Player::position.y = (startPos.y + 0.5f) * MAP_TILE_SIZE;

    // Determine player direction by facing the closest other room
    const Rect* closestRoom = nullptr;
    int minRoomDistSq = std::numeric_limits<int>::max();

    for (const auto& room : rooms) {
        if (&room == startRoom) continue;

        auto roomCenter = room.center();
        int dx = roomCenter.x - startPos.x;
        int dy = roomCenter.y - startPos.y;
        int distSq = dx * dx + dy * dy;

        if (distSq < minRoomDistSq) {
            minRoomDistSq = distSq;
            closestRoom = &room;
        }
    }

    // If we found a closest room, set direction based on it
    if (closestRoom != nullptr) {
        auto closestCenter = closestRoom->center();
        int dx = closestCenter.x - startPos.x;
        int dy = closestCenter.y - startPos.y;

        if (std::abs(dx) > std::abs(dy)) {
            // More horizontal distance
            Player::angle = (dx > 0) ? 0 : std::numbers::pi;
        } else {
            // More vertical distance (or equal)
            Player::angle = (dy > 0) ? std::numbers::pi / 2 : -std::numbers::pi / 2;
        }
    }
}

void generate() {
    printf("Generating %dx%d map with seed %d...", MAP_WIDTH, MAP_HEIGHT, MAP_SEED);
    tiles = generateWalls();
    auto rooms = generateRooms();
    placePlayer(rooms);
    tilesWidth = MAP_WIDTH;
    tilesHeight = MAP_HEIGHT;
    width = MAP_WIDTH * MAP_TILE_SIZE;
    height = MAP_HEIGHT * MAP_TILE_SIZE;
}

} // namespace RC::Map

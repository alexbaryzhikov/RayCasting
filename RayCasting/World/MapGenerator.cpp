#include <cassert>
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
                walls[idx] = Tile::wallIndestructible;
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
            if (tiles[y * MAP_WIDTH + x] != Tile::floor) {
                tiles[y * MAP_WIDTH + x] = Tile::floor;
                ++floorTiles;
            }
        }
    }
}

void fortifyRoom(const Rect& room) {
    for (int y = room.y - 1; y <= room.y + room.h; ++y) {
        if (y > 0 && y < MAP_HEIGHT - 1) {
            if (room.x - 1 > 0) {
                tiles[y * MAP_WIDTH + room.x - 1] = Tile::wallFortified;
            }
            if (room.x + room.w < MAP_WIDTH - 1) {
                tiles[y * MAP_WIDTH + room.x + room.w] = Tile::wallFortified;
            }
        }
    }
    for (int x = room.x; x < room.x + room.w; ++x) {
        if (x > 0 && x < MAP_WIDTH - 1) {
            if (room.y - 1 > 0) {
                tiles[(room.y - 1) * MAP_WIDTH + x] = Tile::wallFortified;
            }
            if (room.y + room.h < MAP_HEIGHT - 1) {
                tiles[(room.y + room.h) * MAP_WIDTH + x] = Tile::wallFortified;
            }
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
        if (getRandomInt(0, 1)) {
            fortifyRoom(newRoom);
        }
        rooms.push_back(newRoom);
    }
    return rooms;
}

void carveTunnelH(int x1, int x2, int y) {
    if (x1 > x2) std::swap(x1, x2);
    for (int x = x1; x <= x2; ++x) {
        if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            tiles[y * MAP_WIDTH + x] = Tile::floor;
        }
    }
}

void carveTunnelV(int y1, int y2, int x) {
    if (y1 > y2) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y) {
        if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            tiles[y * MAP_WIDTH + x] = Tile::floor;
        }
    }
}

int findClosestRoom(const std::vector<Rect>& rooms, const std::vector<bool>& connected, int roomIndex, bool isConnected) {
    const Rect& room = rooms[roomIndex];
    int minDistanceSq = std::numeric_limits<int>::max();
    int closestRoomIndex = -1;

    for (int i = 0; i < rooms.size(); ++i) {
        if (i == roomIndex || (isConnected ? !connected[i] : connected[i])) continue;

        const Rect& otherRoom = rooms[i];
        int dx = room.x < otherRoom.x ? otherRoom.x - (room.x + room.w) : room.x - (otherRoom.x + otherRoom.w);
        int dy = room.y < otherRoom.y ? otherRoom.y - (room.y + room.h) : room.y - (otherRoom.y + otherRoom.h);
        int distanceSq = std::max(dx, 0) + std::max(dy, 0);

        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            closestRoomIndex = i;
        }
    }

    return closestRoomIndex;
}

void carveTunnels(const std::vector<Rect>& rooms) {
    std::vector<bool> connected(rooms.size(), false);
    connected[0] = true;
    int roomIndex = 0;
    while (true) {
        roomIndex = findClosestRoom(rooms, connected, roomIndex, false);
        if (roomIndex == -1) break;

        int otherRoomIndex = findClosestRoom(rooms, connected, roomIndex, true);
        assert(otherRoomIndex != -1);

        const Rect& room = rooms[roomIndex];
        const Rect& otherRoom = rooms[otherRoomIndex];
        auto center = room.center();
        auto otherCenter = otherRoom.center();

        int xStart = std::max(room.x, otherRoom.x);
        int xEnd = std::min(room.x + room.w, otherRoom.x + otherRoom.w);
        int yStart = std::max(room.y, otherRoom.y);
        int yEnd = std::min(room.y + room.h, otherRoom.y + otherRoom.h);

        if (xStart < xEnd) {
            carveTunnelV(otherCenter.y, center.y, xStart + (xEnd - xStart) / 2);
        } else if (yStart < yEnd) {
            carveTunnelH(otherCenter.x, center.x, yStart + (yEnd - yStart) / 2);
        } else {
            carveTunnelV(otherCenter.y, center.y, center.x);
            carveTunnelH(otherCenter.x, center.x, otherCenter.y);
        }

        connected[roomIndex] = true;
    }
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
    carveTunnels(rooms);
    placePlayer(rooms);
    tilesWidth = MAP_WIDTH;
    tilesHeight = MAP_HEIGHT;
    width = MAP_WIDTH * MAP_TILE_SIZE;
    height = MAP_HEIGHT * MAP_TILE_SIZE;
}

} // namespace RC::Map

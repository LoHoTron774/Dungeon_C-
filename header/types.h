#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cstdlib>

enum class Direction { North, South, East, West };

inline constexpr std::array<Direction, 4> kAllDirections{
    Direction::North, Direction::South, Direction::East, Direction::West};

struct Position {
    int y;
    int x;
};

inline Direction opposite(Direction d) {
    switch (d) {
        case Direction::North: return Direction::South;
        case Direction::South: return Direction::North;
        case Direction::East:  return Direction::West;
        case Direction::West:  return Direction::East;
    }
    return d;
}

inline int manhattan(Position a, Position b) {
    return std::abs(a.y - b.y) + std::abs(a.x - b.x);
}

#endif

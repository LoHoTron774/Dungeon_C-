#include "header/creature.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "header/lifecycle_log.h"

namespace {
constexpr std::string_view kReservedGlyphs = "#.GEZ ";
}

Creature::Creature(int y, int x, int hp, int maxHp, char glyph, int attack, std::string name)
    : y_(y), x_(x), health_(hp), maxHealth_(maxHp), attack_(attack), glyph_(glyph), name_(std::move(name)) {
    if (maxHp <= 0) throw std::invalid_argument("максимальное здоровье должно быть положительным");
    if (hp <= 0 || hp > maxHp) throw std::invalid_argument("здоровье должно быть в пределах от 1 до максимума");
    if (attack_ < 0) throw std::invalid_argument("сила атаки не может быть отрицательной");
    if (name_.empty()) throw std::invalid_argument("у существа должно быть имя");
    if (!std::isprint(static_cast<unsigned char>(glyph)) || kReservedGlyphs.find(glyph) != std::string_view::npos)
        throw std::invalid_argument("символ существа печатный и не совпадает с символами клеток");
    lifecycle::log("Creature ctor", this);
}

Creature::~Creature() {
    lifecycle::log("Creature dtor", this);
}

Position Creature::peek(Direction d) const {
    Position p{y_, x_};
    switch (d) {
        case Direction::North: --p.y; break;
        case Direction::South: ++p.y; break;
        case Direction::East:  ++p.x; break;
        case Direction::West:  --p.x; break;
    }
    return p;
}

void Creature::takeDamage(int amount) {
    if (amount < 0) throw std::invalid_argument("урон не может быть отрицательным");
    health_ = (amount >= health_) ? 0 : health_ - amount;
}

void Creature::heal(int amount) {
    if (amount < 0) throw std::invalid_argument("лечение не может быть отрицательным");
    if (!isAlive()) return;
    health_ = std::min(maxHealth_, health_ + std::min(amount, maxHealth_));
}

bool Creature::tryStep(World& world, Direction d) {
    const Position p = peek(d);
    if (!world.isFree(p.y, p.x)) return false;
    y_ = p.y;
    x_ = p.x;
    return true;
}

bool Creature::adjacentTo(const Creature& other) const {
    return manhattan(Position{y_, x_}, Position{other.y_, other.x_}) == 1;
}

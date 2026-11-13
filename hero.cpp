#include "hero.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

bool Hero::trace_ = false;

void Hero::setTrace(bool enabled) { trace_ = enabled; }
bool Hero::traceEnabled() { return trace_; }

Hero::Hero() {
    if (trace_) std::cout << "  [Hero ctor по умолчанию] this=" << this << '\n';
}

Hero::Hero(int startY, int startX, int hp, int startGold)
    : y_(startY), x_(startX), health_(hp), maxHealth_(hp), gold_(startGold) {
    // Объект с некорректным состоянием не должен существовать вообще.
    if (hp <= 0) throw std::invalid_argument("здоровье героя должно быть положительным");
    if (startGold < 0) throw std::invalid_argument("золото не может быть отрицательным");
    if (trace_) std::cout << "  [Hero ctor (" << startY << "," << startX << ")] this=" << this << '\n';
}

Hero::~Hero() {
    if (trace_) std::cout << "  [Hero dtor] this=" << this << '\n';
}

Position Hero::peek(Direction d) const {
    Position p{y_, x_};
    switch (d) {
        case Direction::North: --p.y; break;
        case Direction::South: ++p.y; break;
        case Direction::East:  ++p.x; break;
        case Direction::West:  --p.x; break;
    }
    return p;
}

Hero& Hero::move(Direction d) {
    const Position p = peek(d);
    y_ = p.y;
    x_ = p.x;
    return *this;
}

void Hero::takeDamage(int amount) {
    if (amount < 0) throw std::invalid_argument("урон не может быть отрицательным");
    health_ -= amount;
    if (health_ < 0) health_ = 0;
}

void Hero::addGold(int amount) {
    if (amount < 0) throw std::invalid_argument("нельзя добавить отрицательное количество золота");
    gold_ += amount;
}

void Hero::printStatus() const {
    const double percent = maxHealth_ > 0 ? static_cast<double>(health_) / maxHealth_ * 100.0 : 0.0;
    std::cout << "____Характеристики героя____\n"
              << "Здоровье: " << health_ << " / " << maxHealth_
              << " (" << std::fixed << std::setprecision(1) << percent << "%)\n"
              << std::defaultfloat
              << "Золото: " << gold_ << "\n"
              << "Герой в клетке (" << y_ << ", " << x_ << ")\n";
}

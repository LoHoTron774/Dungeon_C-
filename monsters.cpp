#include "header/monsters.h"

#include <cstdlib>
#include <utility>

#include "header/lifecycle_log.h"

int Monster::created_ = 0;

Monster::Monster(int y, int x, int hp, char glyph, int attack, std::string name)
    : Creature(y, x, hp, hp, glyph, attack, std::move(name)) {
    ++created_;
}

bool Monster::strikeIfAdjacent(World& world, const std::string& verb) {
    Creature& target = world.hero();
    if (!adjacentTo(target)) return false;
    target.takeDamage(attack());
    world.say(name() + " " + verb + " вас на " + std::to_string(attack()) +
              " урона. Ваше HP: " + std::to_string(target.health()));
    return true;
}

Wolf::Wolf(int y, int x, int hp, int attack) : Monster(y, x, hp, 'W', attack, "Волк") {
    lifecycle::log("Wolf ctor", this);
}
Wolf::~Wolf() { lifecycle::log("Wolf dtor", this); }

void Wolf::takeTurn(World& world) {
    if (!isAlive()) return;
    if (strikeIfAdjacent(world, "кусает")) return;

    const Creature& h = world.hero();
    const int dy = h.y() - y();
    const int dx = h.x() - x();
    const Direction vertical = dy < 0 ? Direction::North : Direction::South;
    const Direction horizontal = dx < 0 ? Direction::West : Direction::East;

    if (std::abs(dy) >= std::abs(dx)) {
        if (dy != 0 && tryStep(world, vertical)) return;
        if (dx != 0) tryStep(world, horizontal);
    } else {
        if (dx != 0 && tryStep(world, horizontal)) return;
        if (dy != 0) tryStep(world, vertical);
    }
}

Rat::Rat(int y, int x, int hp, int attack) : Monster(y, x, hp, 'R', attack, "Крыса") {
    lifecycle::log("Rat ctor", this);
}
Rat::~Rat() { lifecycle::log("Rat dtor", this); }

void Rat::takeTurn(World& world) {
    if (!isAlive()) return;
    if (strikeIfAdjacent(world, "кусает")) return;

    static const Direction all[] = {Direction::North, Direction::South, Direction::East, Direction::West};
    tryStep(world, all[world.randomInt(0, 3)]);
}

Sentinel::Sentinel(int y, int x, int hp, int attack) : Monster(y, x, hp, 'S', attack, "Страж") {
    lifecycle::log("Sentinel ctor", this);
}
Sentinel::~Sentinel() { lifecycle::log("Sentinel dtor", this); }

void Sentinel::takeTurn(World& world) {
    if (!isAlive()) return;
    strikeIfAdjacent(world, "бьёт");
}

std::unique_ptr<Monster> MonsterFactory::create(MonsterKind kind, int y, int x, int hp, int attack) {
    switch (kind) {
        case MonsterKind::Wolf:     return std::make_unique<Wolf>(y, x, hp, attack);
        case MonsterKind::Rat:      return std::make_unique<Rat>(y, x, hp, attack);
        case MonsterKind::Sentinel: return std::make_unique<Sentinel>(y, x, hp, attack);
    }
    return nullptr;
}

MonsterSpec MonsterFactory::spec(MonsterKind kind) {
    switch (kind) {
        case MonsterKind::Wolf:     return MonsterSpec{15, 30, 4, 10};
        case MonsterKind::Rat:      return MonsterSpec{10, 15, 3, 6};
        case MonsterKind::Sentinel: return MonsterSpec{25, 30, 10, 20};
    }
    return MonsterSpec{10, 10, 3, 3};
}

const char* MonsterFactory::token(MonsterKind kind) {
    switch (kind) {
        case MonsterKind::Wolf:     return "WOLF";
        case MonsterKind::Rat:      return "RAT";
        case MonsterKind::Sentinel: return "SENTINEL";
    }
    return "?";
}

std::optional<MonsterKind> MonsterFactory::parse(const std::string& t) {
    if (t == "WOLF") return MonsterKind::Wolf;
    if (t == "RAT") return MonsterKind::Rat;
    if (t == "SENTINEL") return MonsterKind::Sentinel;
    return std::nullopt;
}

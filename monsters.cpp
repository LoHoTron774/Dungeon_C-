#include "header/monsters.h"

#include <utility>

#include "header/lifecycle_log.h"

int Monster::created_ = 0;

Monster::Monster(int y, int x, int hp, char glyph, int attack, std::string name, std::string verb, MoveRule rule)
    : Creature(y, x, hp, hp, glyph, attack, std::move(name)), verb_(std::move(verb)), rule_(std::move(rule)) {
    ++created_;
}

void Monster::setRule(MoveRule rule) {
    rule_ = std::move(rule);
}

bool Monster::strikeIfAdjacent(World& world) {
    Creature& target = world.hero();
    if (!adjacentTo(target)) return false;
    target.takeDamage(attack());
    world.say(name() + " " + verb_ + " вас на " + std::to_string(attack()) +
              " урона. Ваше HP: " + std::to_string(target.health()));
    return true;
}

void Monster::takeTurn(World& world) {
    if (!isAlive()) return;
    if (strikeIfAdjacent(world)) return;
    if (!rule_) return;
    if (const std::optional<Direction> step = rule_(*this, world)) tryStep(world, *step);
}

Wolf::Wolf(int y, int x, int hp, int attack)
    : Monster(y, x, hp, 'W', attack, "Волк", "кусает", rules::towardHero()) {
    lifecycle::log("Wolf ctor", this);
}
Wolf::~Wolf() { lifecycle::log("Wolf dtor", this); }

Rat::Rat(int y, int x, int hp, int attack)
    : Monster(y, x, hp, 'R', attack, "Крыса", "кусает", rules::randomWalk()) {
    lifecycle::log("Rat ctor", this);
}
Rat::~Rat() { lifecycle::log("Rat dtor", this); }

Sentinel::Sentinel(int y, int x, int hp, int attack)
    : Monster(y, x, hp, 'S', attack, "Страж", "бьёт", rules::standStill()) {
    lifecycle::log("Sentinel ctor", this);
}
Sentinel::~Sentinel() { lifecycle::log("Sentinel dtor", this); }

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

std::string MonsterFactory::token(MonsterKind kind) {
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

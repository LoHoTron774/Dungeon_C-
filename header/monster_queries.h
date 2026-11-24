#ifndef MONSTER_QUERIES_H
#define MONSTER_QUERIES_H

#include <functional>
#include <memory>
#include <vector>

#include "monsters.h"
#include "types.h"

using MonsterPredicate = std::function<bool(const Monster&)>;
using MonsterView = std::vector<const Monster*>;

namespace queries {

struct Summary {
    int alive;
    int nearby;
    int totalHealth;
    int totalDamage;
    int nearbyDamage;
    std::vector<char> symbols;
};

MonsterPredicate isAlive();
MonsterPredicate isDead();
MonsterPredicate isNear(Position origin, int maxDistance);
MonsterPredicate both(MonsterPredicate first, MonsterPredicate second);
MonsterPredicate negate(MonsterPredicate predicate);

MonsterView view(const std::vector<std::unique_ptr<Monster>>& monsters);
MonsterView select(const MonsterView& monsters, const MonsterPredicate& keep);

int totalHealth(const MonsterView& monsters);
int totalDamage(const MonsterView& monsters);
std::vector<char> symbols(const std::vector<const Creature*>& creatures);

Summary summarize(const MonsterView& monsters, Position hero, int maxDistance);

}

#endif

#include "header/monster_queries.h"

#include <utility>

#include "header/fp.h"

namespace queries {

MonsterPredicate isAlive() {
    return [](const Monster& m) { return m.isAlive(); };
}

MonsterPredicate isDead() {
    return negate(isAlive());
}

MonsterPredicate isNear(Position origin, int maxDistance) {
    return [origin, maxDistance](const Monster& m) {
        return manhattan(Position{m.y(), m.x()}, origin) <= maxDistance;
    };
}

MonsterPredicate both(MonsterPredicate first, MonsterPredicate second) {
    return [first = std::move(first), second = std::move(second)](const Monster& m) {
        return first(m) && second(m);
    };
}

MonsterPredicate negate(MonsterPredicate predicate) {
    return [predicate = std::move(predicate)](const Monster& m) { return !predicate(m); };
}

MonsterView view(const std::vector<std::unique_ptr<Monster>>& monsters) {
    return fp::map(monsters, [](const std::unique_ptr<Monster>& m) -> const Monster* { return m.get(); });
}

MonsterView select(const MonsterView& monsters, const MonsterPredicate& keep) {
    return fp::filter(monsters, [&keep](const Monster* m) { return keep(*m); });
}

int totalHealth(const MonsterView& monsters) {
    return fp::reduce(monsters, 0, [](int sum, const Monster* m) { return sum + m->health(); });
}

int totalDamage(const MonsterView& monsters) {
    return fp::reduce(monsters, 0, [](int sum, const Monster* m) { return sum + m->attack(); });
}

std::vector<char> symbols(const std::vector<const Creature*>& creatures) {
    return fp::map(creatures, [](const Creature* c) { return c->symbol(); });
}

Summary summarize(const MonsterView& monsters, Position hero, int maxDistance) {
    const MonsterView alive = select(monsters, isAlive());
    const MonsterView nearby = select(monsters, both(isAlive(), isNear(hero, maxDistance)));
    const std::vector<const Creature*> creatures =
        fp::map(alive, [](const Monster* m) -> const Creature* { return m; });
    return Summary{static_cast<int>(alive.size()), static_cast<int>(nearby.size()),
                   totalHealth(alive), totalDamage(alive), totalDamage(nearby), symbols(creatures)};
}

}

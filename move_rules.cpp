#include "header/move_rules.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "header/fp.h"

namespace {

std::vector<Direction> approachOrder(const Creature& self, const Creature& target) {
    const int dy = target.y() - self.y();
    const int dx = target.x() - self.x();
    const Direction vertical = dy < 0 ? Direction::North : Direction::South;
    const Direction horizontal = dx < 0 ? Direction::West : Direction::East;

    std::vector<Direction> order;
    if (std::abs(dy) >= std::abs(dx)) {
        if (dy != 0) order.push_back(vertical);
        if (dx != 0) order.push_back(horizontal);
    } else {
        if (dx != 0) order.push_back(horizontal);
        if (dy != 0) order.push_back(vertical);
    }
    return order;
}

std::optional<Direction> firstFree(const Creature& self, World& world, const std::vector<Direction>& order) {
    const auto it = std::find_if(order.begin(), order.end(), [&self, &world](Direction d) {
        const Position p = self.peek(d);
        return world.isFree(p.y, p.x);
    });
    if (it == order.end()) return std::nullopt;
    return *it;
}

Direction randomDirection(World& world) {
    const int last = static_cast<int>(kAllDirections.size()) - 1;
    return kAllDirections[static_cast<std::size_t>(world.randomInt(0, last))];
}

const std::vector<std::pair<std::string, MoveRule>>& table() {
    static const std::vector<std::pair<std::string, MoveRule>> entries = {
        {"toward", rules::towardHero()},
        {"away",   rules::awayFromHero()},
        {"stay",   rules::standStill()},
        {"wander", rules::randomWalk()},
        {"drunk",  rules::drunk(rules::towardHero())},
    };
    return entries;
}

}

namespace rules {

MoveRule towardHero() {
    return [](const Creature& self, World& world) -> std::optional<Direction> {
        return firstFree(self, world, approachOrder(self, world.hero()));
    };
}

MoveRule awayFromHero() {
    return [](const Creature& self, World& world) -> std::optional<Direction> {
        std::vector<Direction> order = approachOrder(self, world.hero());
        std::transform(order.begin(), order.end(), order.begin(), opposite);
        return firstFree(self, world, order);
    };
}

MoveRule standStill() {
    return [](const Creature&, World&) -> std::optional<Direction> {
        return std::nullopt;
    };
}

MoveRule randomWalk() {
    return [](const Creature&, World& world) -> std::optional<Direction> {
        return randomDirection(world);
    };
}

MoveRule drunk(MoveRule base) {
    return [base = std::move(base)](const Creature& self, World& world) -> std::optional<Direction> {
        if (world.randomInt(1, 6) == 1) return randomDirection(world);
        return base(self, world);
    };
}

std::optional<MoveRule> byName(const std::string& name) {
    const auto& entries = table();
    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [&name](const auto& entry) { return entry.first == name; });
    if (it == entries.end()) return std::nullopt;
    return it->second;
}

std::string names() {
    return fp::reduce(table(), std::string{}, [](const std::string& joined, const auto& entry) {
        return joined.empty() ? entry.first : joined + ", " + entry.first;
    });
}

}

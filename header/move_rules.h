#ifndef MOVE_RULES_H
#define MOVE_RULES_H

#include <functional>
#include <optional>
#include <string>

#include "creature.h"
#include "types.h"

using MoveRule = std::function<std::optional<Direction>(const Creature&, World&)>;

namespace rules {

MoveRule towardHero();
MoveRule awayFromHero();
MoveRule standStill();
MoveRule randomWalk();
MoveRule drunk(MoveRule base);

std::optional<MoveRule> byName(const std::string& name);
std::string names();

}

#endif

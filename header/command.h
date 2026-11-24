#ifndef COMMAND_H
#define COMMAND_H

#include <string>

#include "hero.h"
#include "types.h"

enum class CommandType {
    Empty, Quit, Help, Look, Stats, Inventory, New, Load, Save,
    Move, Use, StatusOn, StatusOff, StatusToggle, Summary,
    CheatHeroExit, CheatHesoyam, CheatDamage, CheatKill, CheatRule,
    Unknown,
    Error
};

struct Command {
    CommandType type = CommandType::Unknown;
    Direction direction = Direction::North;
    std::string directionName;
    int number = 0;
    HeroStatus status = HeroStatus::Poisoned;
    std::string statusName;
    std::string ruleName;
    std::string message;
    bool showInventory = false;
};

class CommandParser {
public:
    Command parse(const std::string& line) const;
};

#endif

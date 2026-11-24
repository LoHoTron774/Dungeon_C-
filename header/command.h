#ifndef COMMAND_H
#define COMMAND_H

#include <string>

#include "hero.h"
#include "types.h"

enum class CommandType {
    Empty, Quit, Help, Look, Stats, Inventory, New, Load, Save,
    Move, Use, StatusOn, StatusOff, StatusToggle,
    CheatHeroExit, CheatHesoyam, CheatDamage, CheatKill,
    Unknown,
    Error
};

struct Command {
    CommandType type = CommandType::Unknown;
    Direction direction = Direction::North;
    const char* directionName = "";
    int number = 0;
    HeroStatus status = HeroStatus::Poisoned;
    std::string statusName;
    std::string message;
    bool showInventory = false;
};

class CommandParser {
public:
    Command parse(const std::string& line) const;
};

#endif

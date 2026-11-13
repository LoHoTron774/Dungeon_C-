#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <optional>
#include <string>
#include <vector>

#include "hero.h"
#include "inventory.h"
#include "monsters.h"

struct MonsterRecord {
    MonsterKind kind;
    int y;
    int x;
    int hp;
    int attack;
};

struct SaveData {
    std::vector<std::string> mapLines;
    std::vector<MonsterRecord> monsters;
    HeroSnapshot hero{};
    int floor = 0;
    int turn = 1;
    std::vector<InventorySlot> inventory;
};

class SaveFile {
public:
    explicit SaveFile(std::string path) : path_(std::move(path)) {}

    const std::string& path() const { return path_; }
    const std::string& lastError() const { return error_; }

    bool write(const SaveData& data);
    std::optional<SaveData> read();

private:
    const std::string path_;
    std::string error_;
};

#endif

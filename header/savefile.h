#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <stdexcept>
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

class SaveFileError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SaveFile {
public:
    explicit SaveFile(std::string path) : path_(std::move(path)) {}

    const std::string& path() const { return path_; }

    void write(const SaveData& data) const;
    SaveData read() const;

private:
    const std::string path_;
};

#endif

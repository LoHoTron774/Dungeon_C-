#ifndef GAME_H
#define GAME_H

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "command.h"
#include "creature.h"
#include "hero.h"
#include "inventory.h"
#include "map.h"
#include "monsters.h"
#include "renderer.h"
#include "savefile.h"

class Game : private World {
public:
    explicit Game(std::mt19937 rng, const std::string& levelPath = "");

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run();

    void setSavePath(const std::string& path);

private:
    bool isFree(int y, int x) const override;
    Creature* creatureAt(int y, int x) override;
    Creature& hero() override;
    int randomInt(int lo, int hi) override;
    void say(const std::string& text) override;

    void handle(const Command& cmd);
    void handleWhileDead(const Command& cmd);
    void heroMove(const Command& cmd);
    void useItem(int number);
    void changeStatus(const Command& cmd);
    void showStats() const;
    void cheatHeroExit();
    void cheatDamage();

    std::unique_ptr<Map> buildFloor(int rows, int cols, int floor);
    std::vector<std::unique_ptr<Monster>> buildMonsters(const Map& map, Position heroPos, int floor);
    void descend();
    void victory();
    void newGame();
    bool saveGame();
    void loadGame();
    SaveData makeSaveData() const;
    bool applySave(const SaveData& data, std::string& error);

    bool onHeroEntered();
    void removeDead();
    void endTurn();
    const Creature* find(int y, int x) const;
    void draw() const;

    std::mt19937 rng_;
    std::unique_ptr<Map> map_;
    std::unique_ptr<Hero> hero_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    Inventory inventory_;
    Renderer renderer_;
    CommandParser parser_;
    std::string savePath_ = "saves/save.txt";
    int floor_ = 0;
    int turn_ = 1;
    bool dead_ = false;
    bool over_ = false;
};

#endif

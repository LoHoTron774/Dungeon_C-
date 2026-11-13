#ifndef CREATURE_H
#define CREATURE_H

#include <string>

#include "types.h"

class Creature;

class World {
public:
    virtual ~World() = default;

    virtual bool isFree(int y, int x) const = 0;
    virtual Creature* creatureAt(int y, int x) = 0;
    virtual Creature& hero() = 0;
    virtual int randomInt(int lo, int hi) = 0;
    virtual void say(const std::string& text) = 0;
};

class Creature {
public:
    virtual ~Creature();

    Creature(const Creature&) = delete;
    Creature& operator=(const Creature&) = delete;

    int y() const { return y_; }
    int x() const { return x_; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    int attack() const { return attack_; }
    char symbol() const { return glyph_; }
    const std::string& name() const { return name_; }
    bool isAlive() const { return health_ > 0; }

    Position peek(Direction d) const;
    void takeDamage(int amount);
    void heal(int amount);

    virtual void takeTurn(World& world) = 0;

protected:
    Creature(int y, int x, int hp, int maxHp, char glyph, int attack, std::string name);

    bool tryStep(World& world, Direction d);

    bool adjacentTo(const Creature& other) const;

private:
    int y_;
    int x_;
    int health_;
    const int maxHealth_;
    const int attack_;
    const char glyph_;
    const std::string name_;
};

#endif

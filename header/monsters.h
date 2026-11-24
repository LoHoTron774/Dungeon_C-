#ifndef MONSTERS_H
#define MONSTERS_H

#include <memory>
#include <optional>
#include <string>

#include "creature.h"
#include "move_rules.h"

enum class MonsterKind { Wolf, Rat, Sentinel };

struct MonsterSpec {
    int hpMin, hpMax;
    int attackMin, attackMax;
};

class Monster : public Creature {
public:
    static int created() { return created_; }

    virtual MonsterKind kind() const = 0;

    void setRule(MoveRule rule);
    void takeTurn(World& world) override;

protected:
    Monster(int y, int x, int hp, char glyph, int attack, std::string name, std::string verb, MoveRule rule);

private:
    bool strikeIfAdjacent(World& world);

    static int created_;

    std::string verb_;
    MoveRule rule_;
};

class Wolf final : public Monster {
public:
    Wolf(int y, int x, int hp, int attack);
    ~Wolf() override;
    MonsterKind kind() const override { return MonsterKind::Wolf; }
};

class Rat final : public Monster {
public:
    Rat(int y, int x, int hp, int attack);
    ~Rat() override;
    MonsterKind kind() const override { return MonsterKind::Rat; }
};

class Sentinel final : public Monster {
public:
    Sentinel(int y, int x, int hp, int attack);
    ~Sentinel() override;
    MonsterKind kind() const override { return MonsterKind::Sentinel; }
};

class MonsterFactory {
public:
    static constexpr int kKindCount = 3;

    static std::unique_ptr<Monster> create(MonsterKind kind, int y, int x, int hp, int attack);
    static MonsterSpec spec(MonsterKind kind);
    static std::string token(MonsterKind kind);
    static std::optional<MonsterKind> parse(const std::string& token);
};

#endif

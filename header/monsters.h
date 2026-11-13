#ifndef MONSTERS_H
#define MONSTERS_H

#include <memory>
#include <optional>
#include <string>

#include "creature.h"

enum class MonsterKind { Wolf, Rat, Sentinel };

struct MonsterSpec {
    int hpMin, hpMax;
    int attackMin, attackMax;
};

class Monster : public Creature {
public:
    static int created() { return created_; }

    virtual MonsterKind kind() const = 0;

protected:
    Monster(int y, int x, int hp, char glyph, int attack, std::string name);

    bool strikeIfAdjacent(World& world, const std::string& verb);

private:
    static int created_;
};

class Wolf final : public Monster {
public:
    Wolf(int y, int x, int hp, int attack);
    ~Wolf() override;
    MonsterKind kind() const override { return MonsterKind::Wolf; }
    void takeTurn(World& world) override;
};

class Rat final : public Monster {
public:
    Rat(int y, int x, int hp, int attack);
    ~Rat() override;
    MonsterKind kind() const override { return MonsterKind::Rat; }
    void takeTurn(World& world) override;
};

class Sentinel final : public Monster {
public:
    Sentinel(int y, int x, int hp, int attack);
    ~Sentinel() override;
    MonsterKind kind() const override { return MonsterKind::Sentinel; }
    void takeTurn(World& world) override;
};

class MonsterFactory {
public:
    static constexpr int kKindCount = 3;

    static std::unique_ptr<Monster> create(MonsterKind kind, int y, int x, int hp, int attack);
    static MonsterSpec spec(MonsterKind kind);
    static const char* token(MonsterKind kind);
    static std::optional<MonsterKind> parse(const std::string& token);
};

#endif

#ifndef HERO_H
#define HERO_H

#include <cstdint>
#include <iosfwd>
#include <optional>

#include "creature.h"
#include "types.h"

enum class HeroStatus : std::uint32_t {
    Poisoned  = 1u << 0,
    Blinded   = 1u << 1,
    Invisible = 1u << 2,
    Petrified = 1u << 3,
    Burning   = 1u << 4,
    Sleep     = 1u << 5
};

struct HeroSnapshot {
    int y;
    int x;
    int health;
    int maxHealth;
    int attack;
    int gold;
    std::uint32_t statuses;
    int bonusAttack;
    int bonusTurns;
};

class Hero final : public Creature {
public:
    Hero();
    Hero(int startY, int startX, int hp, int startGold = 0, int attackPower = 10);
    explicit Hero(const HeroSnapshot& s);
    ~Hero() override;

    HeroSnapshot snapshot() const;

    int gold() const { return gold_; }
    void addGold(int amount);

    bool hasStatus(HeroStatus s) const;
    void setStatus(HeroStatus s);
    void clearStatus(HeroStatus s);
    void toggleStatus(HeroStatus s);

    int currentAttack() const { return attack() + bonusAttack_; }
    int buffBonus() const { return bonusAttack_; }
    int buffTurns() const { return bonusTurns_; }
    void applyStrengthBuff(int turns, int bonus);
    bool tickBuff();

    void printStatus(std::ostream& out, int floor) const;

    void orderMove(Direction d);
    void takeTurn(World& world) override;

private:
    static constexpr std::uint32_t kAllStatuses = 0x3Fu;

    int gold_ = 0;
    std::uint32_t statuses_ = 0;
    int bonusAttack_ = 0;
    int bonusTurns_ = 0;
    std::optional<Direction> order_;
};

#endif

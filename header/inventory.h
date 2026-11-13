#ifndef INVENTORY_H
#define INVENTORY_H

#include <iosfwd>
#include <optional>
#include <vector>

enum class PotionType {
    None = 0,
    HealSmall,
    HealMedium,
    HealLarge,
    Strength
};

struct InventorySlot {
    PotionType type;
    int count;
};

struct PotionEffect {
    int heal = 0;
    int strengthTurns = 0;
    int strengthBonus = 0;
};

class Inventory {
public:
    static constexpr int kSize = 10;

    Inventory() = default;
    explicit Inventory(const std::vector<InventorySlot>& slots);

    bool add(PotionType type, int amount);
    std::optional<PotionEffect> use(int index);

    int slotCount() const { return static_cast<int>(slots_.size()); }
    std::vector<InventorySlot> slots() const { return slots_; }

    static const char* name(PotionType type);
    void print(std::ostream& out) const;

private:
    std::vector<InventorySlot> slots_;
};

#endif

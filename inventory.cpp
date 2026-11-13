#include "header/inventory.h"

#include <iostream>
#include <stdexcept>

namespace {
constexpr int kHeal[] = {0, 20, 45, 80};
}

Inventory::Inventory(const std::vector<InventorySlot>& slots) {
    if (static_cast<int>(slots.size()) > kSize) throw std::invalid_argument("в инвентаре слишком много слотов");
    for (const InventorySlot& s : slots) {
        if (s.type < PotionType::HealSmall || s.type > PotionType::Strength)
            throw std::invalid_argument("неизвестный тип зелья в слоте");
        if (s.count < 0) throw std::invalid_argument("отрицательное количество зелий");
        for (const InventorySlot& other : slots_)
            if (other.type == s.type) throw std::invalid_argument("два слота одного типа зелья");
        slots_.push_back(s);
    }
}

bool Inventory::add(PotionType type, int amount) {
    if (type < PotionType::HealSmall || type > PotionType::Strength || amount <= 0) return false;
    for (InventorySlot& s : slots_) {
        if (s.type == type) { s.count += amount; return true; }
    }
    if (slotCount() >= kSize) return false;
    slots_.push_back(InventorySlot{type, amount});
    return true;
}

std::optional<PotionEffect> Inventory::use(int index) {
    if (index < 0 || index >= slotCount()) return std::nullopt;
    InventorySlot& slot = slots_[static_cast<size_t>(index)];
    if (slot.count <= 0) return std::nullopt;
    --slot.count;

    PotionEffect e;
    if (slot.type >= PotionType::HealSmall && slot.type <= PotionType::HealLarge) {
        e.heal = kHeal[static_cast<int>(slot.type)];
    } else {
        e.strengthTurns = 5;
        e.strengthBonus = 5;
    }
    return e;
}

const char* Inventory::name(PotionType type) {
    switch (type) {
        case PotionType::None:       return "пусто";
        case PotionType::HealSmall:  return "Зелье лечения малое +20 HP";
        case PotionType::HealMedium: return "Зелье лечения среднее +45 HP";
        case PotionType::HealLarge:  return "Зелье лечения большое +80 HP";
        case PotionType::Strength:   return "Зелье силы +5 к урону на 5 ходов";
    }
    return "?";
}

void Inventory::print(std::ostream& out) const {
    out << "\n--- Инвентарь ---\n";
    if (slots_.empty()) {
        out << "Инвентарь пуст.\n-----------------\n\n";
        return;
    }
    for (int i = 0; i < slotCount(); ++i) {
        const InventorySlot& s = slots_[static_cast<size_t>(i)];
        if (s.count <= 0) continue;
        out << (i + 1) << ". " << name(s.type) << " — " << s.count << " шт.\n";
    }
    out << "-----------------\n\n";
}

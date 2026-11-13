#include "header/hero.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "header/lifecycle_log.h"

Hero::Hero() : Hero(1, 1, 100) {}

Hero::Hero(int startY, int startX, int hp, int startGold, int attackPower)
    : Hero(HeroSnapshot{startY, startX, hp, hp, attackPower, startGold, 0u, 0, 0}) {}

Hero::Hero(const HeroSnapshot& s)
    : Creature(s.y, s.x, s.health, s.maxHealth, '@', s.attack, "Герой"),
      gold_(s.gold), statuses_(s.statuses), bonusAttack_(s.bonusAttack), bonusTurns_(s.bonusTurns) {
    if (gold_ < 0) throw std::invalid_argument("золото не может быть отрицательным");
    if ((statuses_ & ~kAllStatuses) != 0u) throw std::invalid_argument("неизвестные состояния героя");
    if (bonusAttack_ < 0 || bonusTurns_ < 0 || (bonusAttack_ == 0) != (bonusTurns_ == 0))
        throw std::invalid_argument("бонус силы задан несогласованно");
    lifecycle::log("Hero ctor", this);
}

Hero::~Hero() {
    lifecycle::log("Hero dtor", this);
}

HeroSnapshot Hero::snapshot() const {
    return HeroSnapshot{y(), x(), health(), maxHealth(), attack(), gold_, statuses_, bonusAttack_, bonusTurns_};
}

void Hero::addGold(int amount) {
    if (amount < 0) throw std::invalid_argument("нельзя добавить отрицательное количество золота");
    gold_ += amount;
}

bool Hero::hasStatus(HeroStatus s) const {
    return (statuses_ & static_cast<std::uint32_t>(s)) != 0u;
}
void Hero::setStatus(HeroStatus s)    { statuses_ |= static_cast<std::uint32_t>(s); }
void Hero::clearStatus(HeroStatus s)  { statuses_ &= ~static_cast<std::uint32_t>(s); }
void Hero::toggleStatus(HeroStatus s) { statuses_ ^= static_cast<std::uint32_t>(s); }

void Hero::applyStrengthBuff(int turns, int bonus) {
    if (turns <= 0 || bonus <= 0) throw std::invalid_argument("бонус силы: ходы и прибавка должны быть положительными");
    bonusTurns_ = turns;
    bonusAttack_ = bonus;
}

bool Hero::tickBuff() {
    if (bonusTurns_ <= 0) return false;
    if (--bonusTurns_ == 0) {
        bonusAttack_ = 0;
        return true;
    }
    return false;
}

void Hero::orderMove(Direction d) {
    order_ = d;
}

void Hero::takeTurn(World& world) {
    if (!order_) return;
    const Direction d = *order_;
    order_.reset();

    const Position p = peek(d);
    if (Creature* target = world.creatureAt(p.y, p.x)) {
        const int dmg = currentAttack();
        target->takeDamage(dmg);
        world.say("Вы бьёте монстра (" + target->name() + ") на " + std::to_string(dmg) +
                  " урона. У монстра осталось HP: " + std::to_string(target->health()));
        return;
    }
    tryStep(world, d);
}

void Hero::printStatus(std::ostream& out, int floor) const {
    const double percent = static_cast<double>(health()) / maxHealth() * 100.0;
    out << "____Характеристики героя____\n"
        << "Здоровье: " << health() << " / " << maxHealth()
        << " (" << std::fixed << std::setprecision(1) << percent << "%)\n"
        << std::defaultfloat
        << "Урон: " << attack() << "\n"
        << "Золото: " << gold_ << "\n"
        << "Этаж подземелья: " << floor << "\n";
    if (bonusTurns_ > 0)
        out << "Бонус силы: +" << bonusAttack_ << " к урону, действует ещё " << bonusTurns_ << " ходов\n";
    else
        out << "Бонус силы: отсутствует\n";

    out << "Состояния:\n";
    bool any = false;
    if (hasStatus(HeroStatus::Poisoned))  { out << "Отравлен\n"; any = true; }
    if (hasStatus(HeroStatus::Blinded))   { out << "Ослеплен\n"; any = true; }
    if (hasStatus(HeroStatus::Invisible)) { out << "Невидим\n";  any = true; }
    if (hasStatus(HeroStatus::Petrified)) { out << "Окаменел\n"; any = true; }
    if (hasStatus(HeroStatus::Burning))   { out << "Горит\n";    any = true; }
    if (hasStatus(HeroStatus::Sleep))     { out << "Спит\n";     any = true; }
    if (!any) out << "Нет активных состояний\n";
    out << "\n\n";
}

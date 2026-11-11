#include "hero.h"

#include <iomanip>
#include <iostream>

bool Hero::trace = false;

Hero::Hero() {
    if (trace) std::cout << "  [Hero ctor по умолчанию] this=" << this << '\n';
}

// Список инициализации: поля получают значения сразу при создании, а не присваиванием в теле.
Hero::Hero(int startY, int startX, int hp, int startGold)
    : y(startY), x(startX), health(hp), maxHealth(hp), gold(startGold) {
    if (trace) std::cout << "  [Hero ctor (" << startY << "," << startX << ")] this=" << this << '\n';
}

Hero::~Hero() {
    if (trace) std::cout << "  [Hero dtor] this=" << this << '\n';
}

static void printHeroLine(const Hero* h) {
    std::cout << "Герой в клетке (" << h->y << ", " << h->x << ")\n";
}

Position Hero::peek(Direction d) const {
    Position p{y, x};
    switch (d) {
        case Direction::North: --p.y; break;
        case Direction::South: ++p.y; break;
        case Direction::East:  ++p.x; break;
        case Direction::West:  --p.x; break;
    }
    return p;
}

Hero& Hero::move(Direction d) {
    // Вызов hero.move(dir) компилятор превращает в Hero::move(&hero, dir): адрес объекта
    // передаётся скрытым первым параметром this, а y и x внутри метода — это this->y и this->x.
    // В отличие от C-функции move_hero(&hero, dir), объект не перечисляется в списке параметров:
    // он «стоит перед точкой». Методы общие для всех героев, а this у каждого вызова свой.
    Position p = this->peek(d);
    this->y = p.y;
    this->x = p.x;
    return *this;
}

void Hero::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

void Hero::printStatus() const {
    double percent = maxHealth > 0 ? static_cast<double>(health) / maxHealth * 100.0 : 0.0;
    std::cout << "____Характеристики героя____\n"
              << "Здоровье: " << health << " / " << maxHealth
              << " (" << std::fixed << std::setprecision(1) << percent << "%)\n"
              << std::defaultfloat
              << "Золото: " << gold << "\n";
    printHeroLine(this);
}

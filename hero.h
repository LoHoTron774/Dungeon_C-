#ifndef HERO_H
#define HERO_H

enum class Direction { North, South, East, West };

struct Position {
    int y;
    int x;
};

// Практика 10: добавлены конструкторы. Члены пока открыты (закроем на практике 11).
class Hero {
public:
    int y = 1;
    int x = 1;
    int health = 100;
    int maxHealth = 100;
    int gold = 0;

    Hero();                                          // по умолчанию: клетка (1,1), 100 HP
    Hero(int startY, int startX, int hp, int startGold = 0);
    ~Hero();

    Position peek(Direction d) const;
    Hero& move(Direction d);
    void takeDamage(int amount);
    void printStatus() const;

    // Печать сообщений о создании/уничтожении (включается в демонстрации времени жизни).
    static bool trace;
};

#endif // HERO_H

#ifndef HERO_H
#define HERO_H

enum class Direction { North, South, East, West };

// Простое значение (координата). Position возвращается по значению, то есть это КОПИЯ:
// изменение её полей никак не влияет на героя.
struct Position {
    int y;
    int x;
};

// Практика 11: инкапсуляция. Всё состояние героя закрыто (private).
// Читать его можно только через const-геттеры, менять — только через методы,
// которые сами следят за корректностью (здоровье не уходит ниже 0 и выше максимума,
// золото не бывает отрицательным).
class Hero {
public:
    Hero();                                                   // клетка (1,1), 100 HP, 0 золота
    // Бросает std::invalid_argument, если hp <= 0 или startGold < 0.
    Hero(int startY, int startX, int hp, int startGold = 0);
    ~Hero();

    // --- чтение состояния (ничего не меняют) ---
    int y() const { return y_; }
    int x() const { return x_; }
    Position position() const { return Position{y_, x_}; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    int gold() const { return gold_; }
    bool isAlive() const { return health_ > 0; }

    // --- поведение ---
    Position peek(Direction d) const;     // куда попадёт герой, не двигая его
    Hero& move(Direction d);              // шаг на одну клетку (проверка карты — на вызывающем)
    void takeDamage(int amount);          // amount < 0 -> std::invalid_argument
    void addGold(int amount);             // amount < 0 -> std::invalid_argument
    void printStatus() const;

    // Печать сообщений о создании/уничтожении (для демонстрации времени жизни).
    static void setTrace(bool enabled);
    static bool traceEnabled();

private:
    int y_ = 1;
    int x_ = 1;
    int health_ = 100;
    int maxHealth_ = 100;
    int gold_ = 0;

    static bool trace_;
};

#endif // HERO_H

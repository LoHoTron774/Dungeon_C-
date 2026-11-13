#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "hero.h"
#include "map.h"

static std::string normalize(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool parseDirection(const std::string& w, Direction& out, const char*& name) {
    if (w == "north" || w == "n") { out = Direction::North; name = "север";  return true; }
    if (w == "south" || w == "s") { out = Direction::South; name = "юг";     return true; }
    if (w == "east"  || w == "e") { out = Direction::East;  name = "восток"; return true; }
    if (w == "west"  || w == "w") { out = Direction::West;  name = "запад";  return true; }
    return false;
}

static const char* kHelp =
    "'#' - стена; '.' - пол; '@' - герой; 'G' - золото; 'E' - выход на следующий этаж\n"
    "Доступные команды:\n"
    "   north, south, east, west - Перемещение (можно n, s, e, w)\n"
    "   stats                    - Характеристики героя\n"
    "   look                     - Осмотреться\n"
    "   help                     - Показать команды\n"
    "   quit                     - Выйти\n";

// Демонстрация времени жизни объектов.
//
// Правила, которые подтверждает вывод:
//  1. Объект на стеке создаётся там, где объявлен, и уничтожается автоматически при выходе
//     из блока { ... }. Деструкторы вызываются в ОБРАТНОМ порядке создания (последний создан —
//     первый уничтожен), как стопка тарелок.
//  2. Объект в куче (new) живёт, пока его не удалят явно через delete. Забыли delete —
//     деструктор не вызовется и память утечёт. Автоматически за кучу отвечаем мы.
//  3. Объекты внутри блока { ... } уничтожаются при выходе из этого блока, а не в конце функции.
static void lifetimeDemo() {
    Map::setTrace(true);
    Hero::setTrace(true);

    std::cout << "1) Объекты на стеке в main-функции демонстрации:\n";
    Map stackMap(6, 12);          // создан первым -> уничтожен последним
    Hero stackHero;               // создан вторым -> уничтожен предпоследним

    std::cout << "2) Объекты в куче (new):\n";
    Map* heapMap = new Map(7, 14);
    Hero* heapHero = new Hero(2, 2, 50);

    std::cout << "3) Вход в блок:\n";
    {
        Map blockMap(5, 5);
        Hero blockHero(1, 1, 10, 3);
        std::cout << "   ...работаем внутри блока...\n";
        std::cout << "   Выход из блока (сейчас сработают деструкторы blockHero, затем blockMap):\n";
    }
    std::cout << "   Блок закончился, объекты блока уничтожены автоматически.\n";

    std::cout << "4) Явное удаление объектов из кучи:\n";
    delete heapHero;
    delete heapMap;

    std::cout << "5) Конец функции: сейчас автоматически уничтожатся stackHero, затем stackMap:\n";
}

// Ошибка при загрузке: Map не создаётся, память освобождается сама.
static void loadDemo(const std::string& path) {
    Map::setTrace(true);
    std::optional<Map> level;
    LoadError e = Map::load(path, level);
    std::cout << path << " -> " << describe(e) << (level ? " (карта создана)" : " (карта не создана)") << '\n';
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    if (argc >= 2 && std::strcmp(argv[1], "--demo") == 0) {
        lifetimeDemo();
        return 0;
    }
    if (argc >= 3 && std::strcmp(argv[1], "--load-demo") == 0) {
        loadDemo(argv[2]);
        return 0;
    }

    std::mt19937 rng(std::random_device{}());

    // Карта: из файла (если передан) или случайная. Объект на стеке внутри optional:
    // при ошибке загрузки он просто не создаётся, и освобождать нечего.
    std::optional<Map> level;
    if (argc >= 2) {
        LoadError e = Map::load(argv[1], level);
        if (e != LoadError::Ok) {
            std::cerr << "[Ошибка] " << describe(e) << ": " << argv[1] << '\n';
            return 1;
        }
    } else {
        level.emplace(6, 12);
        level->generate(rng);
    }
    Map& map = *level;
    Hero hero(1, 1, 100);

    std::cout << "Добро пожаловать в подземелье!\n" << kHelp << '\n';
    map.print(hero.y(), hero.x());

    int turn = 1;
    std::string line;
    while (true) {
        std::cout << "\n[Ход " << turn << "] > " << std::flush;
        if (!std::getline(std::cin, line)) break;
        std::string cmd = normalize(line);

        if (cmd.empty()) {
            std::cout << "[Ошибка] Пустая команда.\n";
        } else if (cmd == "quit") {
            std::cout << "Игра окончена\n";
            break;
        } else if (cmd == "help") {
            std::cout << kHelp;
        } else if (cmd == "stats") {
            hero.printStatus();
        } else if (cmd == "look") {
            std::cout << "Ты осмотрелся\n";
            ++turn;
            map.print(hero.y(), hero.x());
        } else {
            Direction dir;
            const char* dirName = "";
            if (!parseDirection(cmd, dir, dirName)) {
                std::cout << "[Ошибка] Неизвестная команда\n" << kHelp;
                continue;
            }
            Position next = hero.peek(dir);
            if (!map.inBounds(next.y, next.x)) {
                std::cout << "Нельзя выйти за границу карты!\n";
                map.print(hero.y(), hero.x());
                continue;
            }
            if (!map.isPassable(next.y, next.x)) {
                std::cout << "Через стены ходить нельзя!\n";
                map.print(hero.y(), hero.x());
                continue;
            }
            hero.move(dir);
            if (map.collectGold(hero.y(), hero.x())) {
                hero.addGold(1);
                std::cout << "Вы нашли золото. Ваше золото: " << hero.gold() << '\n';
            } else if (map.get(hero.y(), hero.x()) == 'E') {
                std::cout << "Вы нашли выход на следующий этаж (переход будет добавлен позже).\n";
            }
            ++turn;
            std::cout << "Ты пошёл на " << dirName << '\n';
        }
    }
    return 0;   // hero и level уничтожатся автоматически
}

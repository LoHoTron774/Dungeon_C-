#include "header/demos.h"

#include <iostream>
#include <memory>

#include "header/hero.h"
#include "header/lifecycle_log.h"
#include "header/map.h"
#include "header/monsters.h"

namespace {
void enableTrace() {
    lifecycle::setSink([](const std::string& line) { std::cout << line << '\n'; });
}
}

void lifetimeDemo() {
    enableTrace();

    std::cout << "1) Объекты на стеке в функции демонстрации:\n";
    Map stackMap(6, 12);
    Hero stackHero;
    Wolf stackWolf(3, 3, 20, 6);

    std::cout << "2) Объекты в куче (new):\n";
    Map* heapMap = new Map(7, 14);
    Creature* heapCreature = new Rat(4, 4, 8, 2);

    std::cout << "3) Вход в блок:\n";
    {
        Map blockMap(5, 5);
        Hero blockHero(1, 1, 10, 3);
        std::cout << "   ...работаем внутри блока...\n";
        std::cout << "   Выход из блока (сейчас сработают деструкторы blockHero, затем blockMap):\n";
    }
    std::cout << "   Блок закончился, объекты блока уничтожены автоматически.\n";

    std::cout << "4) Явное удаление объектов из кучи (у Rat виртуальный деструктор через Creature*):\n";
    delete heapCreature;
    delete heapMap;

    std::cout << "5) Конец функции: сейчас автоматически уничтожатся stackWolf, stackHero, затем stackMap:\n";
}

void loadDemo(const std::string& path) {
    enableTrace();
    std::unique_ptr<Map> level;
    const LoadError e = Map::load(path, level);
    std::cout << path << " -> " << describe(e) << (level ? " (карта создана)" : " (карта не создана)") << '\n';
}

namespace {

int liveBlocks = 0;

struct BadBase {
    ~BadBase() { std::cout << "  [BadBase dtor]\n"; }
};
struct BadDerived : BadBase {
    int* data = new int[100];
    BadDerived() { ++liveBlocks; }
    ~BadDerived() { delete[] data; --liveBlocks; std::cout << "  [BadDerived dtor] блок освобождён\n"; }
};

struct GoodBase {
    virtual ~GoodBase() { std::cout << "  [GoodBase dtor]\n"; }
};
struct GoodDerived : GoodBase {
    int* data = new int[100];
    GoodDerived() { ++liveBlocks; }
    ~GoodDerived() override { delete[] data; --liveBlocks; std::cout << "  [GoodDerived dtor] блок освобождён\n"; }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
void virtualDestructorDemo() {
    std::cout << "A) Деструктор базы НЕвиртуальный: delete через BadBase*\n";
    BadBase* bad = new BadDerived;
    delete bad;
    std::cout << "   Неосвобождённых блоков: " << liveBlocks << (liveBlocks ? "  <- УТЕЧКА\n" : "\n");

    liveBlocks = 0;
    std::cout << "B) Деструктор базы виртуальный: delete через GoodBase*\n";
    GoodBase* good = new GoodDerived;
    delete good;
    std::cout << "   Неосвобождённых блоков: " << liveBlocks << '\n';
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

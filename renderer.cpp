#include "header/renderer.h"

#include <ostream>

namespace {
constexpr const char* kColorExit    = "\x1b[32m";
constexpr const char* kColorHero    = "\x1b[34m";
constexpr const char* kColorMonster = "\x1b[31m";
constexpr const char* kColorGold    = "\x1b[33m";
constexpr const char* kColorPotion  = "\x1b[35m";
constexpr const char* kColorReset   = "\x1b[0m";
}

void Renderer::draw(std::ostream& out, const Map& map,
                    const std::vector<const Creature*>& monsters, const Creature& hero) const {
    for (int y = 0; y < map.rows(); ++y) {
        for (int x = 0; x < map.cols(); ++x) {
            if (hero.y() == y && hero.x() == x) {
                out << kColorHero << hero.symbol() << kColorReset;
                continue;
            }
            const Creature* monster = nullptr;
            for (const Creature* m : monsters) {
                if (m->y() == y && m->x() == x) { monster = m; break; }
            }
            if (monster) {
                out << kColorMonster << monster->symbol() << kColorReset;
                continue;
            }
            const char c = map.get(y, x);
            const char* color = nullptr;
            switch (c) {
                case 'E': color = kColorExit; break;
                case 'G': color = kColorGold; break;
                case 'Z': color = kColorPotion; break;
                default: break;
            }
            if (color) out << color << c << kColorReset;
            else       out << c;
        }
        out << '\n';
    }
}

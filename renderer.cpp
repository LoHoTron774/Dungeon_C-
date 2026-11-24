#include "header/renderer.h"

#include <algorithm>
#include <ostream>
#include <string_view>

#include "header/fp.h"

namespace {

struct Glyph {
    Position pos;
    char symbol;
};

constexpr std::string_view kColorExit    = "\x1b[32m";
constexpr std::string_view kColorHero    = "\x1b[34m";
constexpr std::string_view kColorMonster = "\x1b[31m";
constexpr std::string_view kColorGold    = "\x1b[33m";
constexpr std::string_view kColorPotion  = "\x1b[35m";
constexpr std::string_view kColorReset   = "\x1b[0m";

}

void Renderer::draw(std::ostream& out, const Map& map,
                    const std::vector<const Creature*>& monsters, const Creature& hero) const {
    const std::vector<Glyph> glyphs = fp::map(monsters, [](const Creature* m) {
        return Glyph{Position{m->y(), m->x()}, m->symbol()};
    });

    for (int y = 0; y < map.rows(); ++y) {
        for (int x = 0; x < map.cols(); ++x) {
            if (hero.y() == y && hero.x() == x) {
                out << kColorHero << hero.symbol() << kColorReset;
                continue;
            }
            const auto monster = std::find_if(glyphs.begin(), glyphs.end(), [y, x](const Glyph& g) {
                return g.pos.y == y && g.pos.x == x;
            });
            if (monster != glyphs.end()) {
                out << kColorMonster << monster->symbol << kColorReset;
                continue;
            }
            const char c = map.get(y, x);
            std::string_view color;
            switch (c) {
                case 'E': color = kColorExit; break;
                case 'G': color = kColorGold; break;
                case 'Z': color = kColorPotion; break;
                default: break;
            }
            if (!color.empty()) out << color << c << kColorReset;
            else       out << c;
        }
        out << '\n';
    }
}

#include "header/savefile.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>

#include "header/map.h"

namespace {

[[noreturn]] void fail(const std::string& why) {
    throw SaveFileError("Повреждённый файл сохранения: " + why);
}

}

void SaveFile::write(const SaveData& d) const {
    namespace fs = std::filesystem;

    std::error_code ec;
    const fs::path target(path_);
    if (target.has_parent_path()) fs::create_directories(target.parent_path(), ec);

    const std::string tmpPath = path_ + ".tmp";
    {
        std::ofstream f(tmpPath, std::ios::trunc);
        if (!f) throw SaveFileError("Не удалось открыть файл для сохранения: " + tmpPath);

        f << "MAP_SIZE " << d.mapLines.size() << ' ' << (d.mapLines.empty() ? 0 : d.mapLines[0].size()) << '\n';
        for (const std::string& line : d.mapLines) f << line << '\n';

        f << "MONSTERS_COUNT " << d.monsters.size() << '\n';
        for (const MonsterRecord& m : d.monsters)
            f << "MONSTER " << MonsterFactory::token(m.kind) << ' ' << m.y << ' ' << m.x << ' '
              << m.hp << ' ' << m.attack << '\n';

        f << "HERO_POS " << d.hero.y << ' ' << d.hero.x << '\n';
        f << "HERO_STATS " << d.hero.health << ' ' << d.hero.maxHealth << ' ' << d.hero.attack << ' '
          << d.floor << ' ' << d.hero.gold << ' ' << d.hero.statuses << '\n';
        f << "HERO_BUFF " << d.hero.bonusAttack << ' ' << d.hero.bonusTurns << '\n';

        f << "INVENTORY_SLOTS " << d.inventory.size() << '\n';
        for (const InventorySlot& s : d.inventory) f << "ITEM " << static_cast<int>(s.type) << ' ' << s.count << '\n';

        f << "GAME_STATS " << d.turn << '\n';

        f.flush();
        if (!f) {
            f.close();
            fs::remove(tmpPath, ec);
            throw SaveFileError("Ошибка записи: возможно, диск заполнен");
        }
    }

    fs::rename(tmpPath, target, ec);
    if (ec) {
        const std::string reason = ec.message();
        fs::remove(tmpPath, ec);
        throw SaveFileError("Не удалось заменить файл сохранения: " + reason);
    }
}

SaveData SaveFile::read() const {
    std::ifstream in(path_);
    if (!in) throw SaveFileError("Не удалось открыть файл для загрузки: " + path_);

    std::string token;
    auto expect = [&](std::string_view keyword) { return (in >> token) && token == keyword; };

    SaveData d;

    int rows = 0, cols = 0;
    if (!expect("MAP_SIZE") || !(in >> rows >> cols)) fail("нет MAP_SIZE");
    if (rows < Map::kMinSize || rows > Map::kMaxRows || cols < Map::kMinSize || cols > Map::kMaxCols)
        fail("недопустимые размеры карты");
    d.mapLines.resize(static_cast<std::size_t>(rows));
    for (std::string& line : d.mapLines) {
        if (!(in >> line)) fail("неожиданный конец файла при чтении карты");
        if (static_cast<int>(line.size()) != cols) fail("строка карты неверной длины");
    }

    std::size_t count = 0;
    if (!expect("MONSTERS_COUNT") || !(in >> count)) fail("нет MONSTERS_COUNT");
    if (count > 10000) fail("слишком много монстров");
    for (std::size_t i = 0; i < count; ++i) {
        std::string kindToken;
        MonsterRecord m{};
        if (!expect("MONSTER") || !(in >> kindToken >> m.y >> m.x >> m.hp >> m.attack))
            fail("запись о монстре");
        const std::optional<MonsterKind> kind = MonsterFactory::parse(kindToken);
        if (!kind) fail("неизвестный вид монстра: " + kindToken);
        m.kind = *kind;
        d.monsters.push_back(m);
    }

    if (!expect("HERO_POS") || !(in >> d.hero.y >> d.hero.x)) fail("нет HERO_POS");
    if (!expect("HERO_STATS") ||
        !(in >> d.hero.health >> d.hero.maxHealth >> d.hero.attack >> d.floor >> d.hero.gold >> d.hero.statuses))
        fail("нет HERO_STATS");
    if (!expect("HERO_BUFF") || !(in >> d.hero.bonusAttack >> d.hero.bonusTurns)) fail("нет HERO_BUFF");

    int slots = 0;
    if (!expect("INVENTORY_SLOTS") || !(in >> slots)) fail("нет INVENTORY_SLOTS");
    if (slots < 0 || slots > Inventory::kSize) fail("неверное число слотов инвентаря");
    for (int i = 0; i < slots; ++i) {
        int type = 0, cnt = 0;
        if (!expect("ITEM") || !(in >> type >> cnt)) fail("запись предмета");
        d.inventory.push_back(InventorySlot{static_cast<PotionType>(type), cnt});
    }

    if (!expect("GAME_STATS") || !(in >> d.turn)) fail("нет GAME_STATS");

    return d;
}

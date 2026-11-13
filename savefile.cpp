#include "header/savefile.h"

#include <cstdint>
#include <filesystem>
#include <fstream>

#include "header/map.h"

bool SaveFile::write(const SaveData& d) {
    namespace fs = std::filesystem;
    error_.clear();

    std::error_code ec;
    const fs::path target(path_);
    if (target.has_parent_path()) fs::create_directories(target.parent_path(), ec);

    const std::string tmpPath = path_ + ".tmp";
    {
        std::ofstream f(tmpPath, std::ios::trunc);
        if (!f) { error_ = "Не удалось открыть файл для сохранения: " + tmpPath; return false; }

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
            error_ = "Ошибка записи: возможно, диск заполнен";
            f.close();
            fs::remove(tmpPath, ec);
            return false;
        }
    }

    fs::rename(tmpPath, target, ec);
    if (ec) {
        error_ = "Не удалось заменить файл сохранения: " + ec.message();
        fs::remove(tmpPath, ec);
        return false;
    }
    return true;
}

std::optional<SaveData> SaveFile::read() {
    error_.clear();
    std::ifstream in(path_);
    if (!in) { error_ = "Не удалось открыть файл для загрузки: " + path_; return std::nullopt; }

    auto fail = [this](const std::string& why) -> std::optional<SaveData> {
        error_ = "Повреждённый файл сохранения: " + why;
        return std::nullopt;
    };
    std::string token;
    auto expect = [&](const char* keyword) { return (in >> token) && token == keyword; };

    SaveData d;

    int rows = 0, cols = 0;
    if (!expect("MAP_SIZE") || !(in >> rows >> cols)) return fail("нет MAP_SIZE");
    if (rows < Map::kMinSize || rows > Map::kMaxRows || cols < Map::kMinSize || cols > Map::kMaxCols)
        return fail("недопустимые размеры карты");
    d.mapLines.resize(static_cast<size_t>(rows));
    for (std::string& line : d.mapLines) {
        if (!(in >> line)) return fail("неожиданный конец файла при чтении карты");
        if (static_cast<int>(line.size()) != cols) return fail("строка карты неверной длины");
    }

    size_t count = 0;
    if (!expect("MONSTERS_COUNT") || !(in >> count)) return fail("нет MONSTERS_COUNT");
    if (count > 10000) return fail("слишком много монстров");
    for (size_t i = 0; i < count; ++i) {
        std::string kindToken;
        MonsterRecord m{};
        if (!expect("MONSTER") || !(in >> kindToken >> m.y >> m.x >> m.hp >> m.attack))
            return fail("запись о монстре");
        const std::optional<MonsterKind> kind = MonsterFactory::parse(kindToken);
        if (!kind) return fail("неизвестный вид монстра: " + kindToken);
        m.kind = *kind;
        d.monsters.push_back(m);
    }

    if (!expect("HERO_POS") || !(in >> d.hero.y >> d.hero.x)) return fail("нет HERO_POS");
    if (!expect("HERO_STATS") ||
        !(in >> d.hero.health >> d.hero.maxHealth >> d.hero.attack >> d.floor >> d.hero.gold >> d.hero.statuses))
        return fail("нет HERO_STATS");
    if (!expect("HERO_BUFF") || !(in >> d.hero.bonusAttack >> d.hero.bonusTurns)) return fail("нет HERO_BUFF");

    int slots = 0;
    if (!expect("INVENTORY_SLOTS") || !(in >> slots)) return fail("нет INVENTORY_SLOTS");
    if (slots < 0 || slots > Inventory::kSize) return fail("неверное число слотов инвентаря");
    for (int i = 0; i < slots; ++i) {
        int type = 0, cnt = 0;
        if (!expect("ITEM") || !(in >> type >> cnt)) return fail("запись предмета");
        d.inventory.push_back(InventorySlot{static_cast<PotionType>(type), cnt});
    }

    if (!expect("GAME_STATS") || !(in >> d.turn)) return fail("нет GAME_STATS");

    return d;
}

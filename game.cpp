#include "header/game.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <new>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "header/fp.h"

namespace {

constexpr std::string_view kLegend =
    "'#' - стена; '.' - пол; '@' - герой; 'G' - золото; 'Z' - зелье; 'E' - выход на следующий этаж\n"
    "Монстры: 'W' - волк (преследует), 'R' - крыса (блуждает), 'S' - страж (стоит на месте)\n"
    "Чтобы ударить монстра, идите в его сторону.\n";

constexpr std::string_view kCommands =
    "Доступные команды:\n"
    "   north, south, east, west - Перемещение (можно n, s, e, w или go north)\n"
    "   stats                    - Характеристики героя\n"
    "   inventory                - Показать инвентарь\n"
    "   use N                    - Использовать предмет номер N\n"
    "   on <state>               - Включить состояние\n"
    "   off <state>              - Выключить состояние\n"
    "   toggle <state>           - Переключить состояние\n"
    "   Состояния: poisoned, blinded, invisible, petrified, burning, sleep\n"
    "   summary                  - Сводка по монстрам\n"
    "   rule <name>              - (отладка) Сменить правило хода всех монстров\n"
    "   Правила: toward, away, stay, wander, drunk\n"
    "   look                     - Осмотреться\n"
    "   help                     - Показать команды\n"
    "   new                      - Новая игра\n"
    "   save                     - Сохранить прогресс\n"
    "   load                     - Загрузить последнее сохранение\n"
    "   quit                     - Выйти (прогресс сохраняется)\n";

std::string percent(int value, int max) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << static_cast<double>(value) / max * 100.0;
    return os.str();
}

constexpr std::string_view kDeathMenu =
    "Что дальше?\n"
    "   load - загрузить последнее сохранение\n"
    "   new  - начать новую игру\n"
    "   quit - выйти\n";

constexpr int kKillReward = 2;
constexpr int kNearDistance = 3;

}

Game::Game(std::mt19937 rng, const std::string& levelPath) : rng_(std::move(rng)) {
    if (levelPath.empty()) {
        map_ = buildFloor(6, 12, 0);
    } else {
        map_ = Map::load(levelPath);
    }
    hero_ = std::make_unique<Hero>(1, 1, 100);
    monsters_ = buildMonsters(*map_, Position{1, 1}, 0);
}

void Game::setSavePath(const std::string& path) { savePath_ = path; }

std::unique_ptr<Map> Game::buildFloor(int rows, int cols, int floor) {
    std::unique_ptr<Map> m = std::make_unique<Map>(rows, cols);
    m->generate(rng_, 6 + 2 * floor, 1 + 2 * floor);
    return m;
}

std::vector<std::unique_ptr<Monster>> Game::buildMonsters(const Map& map, Position heroPos, int floor) {
    std::vector<Position> candidates;
    for (int y = 0; y < map.rows(); ++y)
        for (int x = 0; x < map.cols(); ++x)
            if (map.get(y, x) == '.' && std::abs(y - heroPos.y) + std::abs(x - heroPos.x) >= 4)
                candidates.push_back(Position{y, x});
    std::shuffle(candidates.begin(), candidates.end(), rng_);

    const std::size_t wanted = static_cast<std::size_t>(4 + 2 * floor + randomInt(0, 2));
    const std::size_t n = std::min(wanted, candidates.size());

    std::vector<std::unique_ptr<Monster>> result;
    for (std::size_t i = 0; i < n; ++i) {
        const MonsterKind kind = static_cast<MonsterKind>(i % MonsterFactory::kKindCount);
        const MonsterSpec s = MonsterFactory::spec(kind);
        result.push_back(MonsterFactory::create(kind, candidates[i].y, candidates[i].x,
                                                randomInt(s.hpMin, s.hpMax),
                                                randomInt(s.attackMin, s.attackMax)));
    }
    return result;
}

const Creature* Game::find(int y, int x) const {
    if (hero_->y() == y && hero_->x() == x) return hero_.get();
    const auto it = std::find_if(monsters_.begin(), monsters_.end(), [y, x](const std::unique_ptr<Monster>& m) {
        return m->y() == y && m->x() == x;
    });
    return it == monsters_.end() ? nullptr : it->get();
}

bool Game::isFree(int y, int x) const {
    return map_->isPassable(y, x) && find(y, x) == nullptr;
}

Creature* Game::creatureAt(int y, int x) {
    return const_cast<Creature*>(find(y, x));
}

Creature& Game::hero() { return *hero_; }

int Game::randomInt(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(rng_);
}

void Game::say(const std::string& text) {
    std::cout << text << '\n';
}

void Game::run() {
    std::cout << "Добро пожаловать в подземелье!\n" << kLegend << kCommands << '\n';
    draw();
    inventory_.print(std::cout);

    std::string line;
    while (!over_) {
        std::cout << "\n[Ход " << turn_ << "] > " << std::flush;
        if (!std::getline(std::cin, line)) break;
        handle(parser_.parse(line));
    }
}

void Game::handleWhileDead(const Command& cmd) {
    switch (cmd.type) {
        case CommandType::Load:
            loadGame();
            break;
        case CommandType::New:
            newGame();
            break;
        case CommandType::Quit:
            say("Игра окончена");
            over_ = true;
            break;
        case CommandType::Stats:
            showStats();
            break;
        case CommandType::Inventory:
            inventory_.print(std::cout);
            break;
        default:
            std::cout << "Герой погиб, продолжать нельзя.\n" << kDeathMenu;
            break;
    }
}

void Game::handle(const Command& cmd) {
    if (dead_) {
        handleWhileDead(cmd);
        return;
    }
    switch (cmd.type) {
        case CommandType::Empty:
            say("[Ошибка] Пустая команда.");
            break;
        case CommandType::Quit:
            saveGame();
            say("Игра окончена");
            over_ = true;
            break;
        case CommandType::Help:
            std::cout << kCommands;
            break;
        case CommandType::Look:
            say("Ты осмотрелся");
            endTurn();
            break;
        case CommandType::Stats:
            showStats();
            break;
        case CommandType::Inventory:
            inventory_.print(std::cout);
            break;
        case CommandType::New:
            newGame();
            break;
        case CommandType::Load:
            loadGame();
            break;
        case CommandType::Save:
            saveGame();
            break;
        case CommandType::Move:
            heroMove(cmd);
            break;
        case CommandType::Use:
            useItem(cmd.number);
            break;
        case CommandType::StatusOn:
        case CommandType::StatusOff:
        case CommandType::StatusToggle:
            changeStatus(cmd);
            break;
        case CommandType::CheatHeroExit:
            cheatHeroExit();
            break;
        case CommandType::CheatHesoyam:
            say("чит активирован");
            hero_->heal(hero_->maxHealth());
            say("Здоровье: " + std::to_string(hero_->health()) + "/" + std::to_string(hero_->maxHealth()) +
                " (" + percent(hero_->health(), hero_->maxHealth()) + "%)");
            break;
        case CommandType::CheatDamage:
            cheatDamage();
            break;
        case CommandType::CheatRule:
            cheatRule(cmd.ruleName);
            break;
        case CommandType::Summary:
            showSummary();
            break;
        case CommandType::CheatKill:
            cheatKill();
            break;
        case CommandType::Error:
            say(cmd.message);
            if (cmd.showInventory) inventory_.print(std::cout);
            break;
        case CommandType::Unknown:
            std::cout << "[Ошибка] Неизвестная команда\n" << kCommands;
            break;
    }
}

void Game::heroMove(const Command& cmd) {
    const Position next = hero_->peek(cmd.direction);
    if (!map_->inBounds(next.y, next.x)) {
        say("Нельзя выйти за границу карты!");
        draw();
        return;
    }
    if (!map_->isPassable(next.y, next.x)) {
        say("Через стены ходить нельзя!");
        draw();
        return;
    }

    const int oldY = hero_->y();
    const int oldX = hero_->x();
    hero_->orderMove(cmd.direction);
    hero_->takeTurn(*this);

    if (hero_->y() != oldY || hero_->x() != oldX) {
        if (onHeroEntered()) {
            descend();
            return;
        }
        say("Ты пошёл на " + cmd.directionName);
    }
    removeDead();
    endTurn();
}

bool Game::onHeroEntered() {
    const int y = hero_->y();
    const int x = hero_->x();
    switch (map_->takeItem(y, x)) {
        case Map::Item::Gold:
            hero_->addGold(1);
            say("Вы нашли золото. Ваше золото: " + std::to_string(hero_->gold()));
            break;
        case Map::Item::Potion: {
            const PotionType t = static_cast<PotionType>(randomInt(1, 4));
            if (inventory_.add(t, 1))
                say("Вы нашли зелье: " + Inventory::name(t) + "!");
            else
                say("Инвентарь полон! Вы нашли зелье, но не смогли его подобрать.");
            break;
        }
        case Map::Item::None:
            break;
    }
    return map_->get(y, x) == 'E';
}

void Game::useItem(int number) {
    const std::optional<PotionEffect> effect = inventory_.use(number - 1);
    if (!effect) {
        say("[Ошибка] Неверный номер предмета или предмет закончился.");
        inventory_.print(std::cout);
        return;
    }
    if (effect->heal > 0) {
        hero_->heal(effect->heal);
        say("Вы использовали зелье лечения. Восстановлено " + std::to_string(effect->heal) +
            " HP. Здоровье: " + std::to_string(hero_->health()) + "/" + std::to_string(hero_->maxHealth()) +
            " (" + percent(hero_->health(), hero_->maxHealth()) + "%)");
    } else if (effect->strengthTurns > 0) {
        hero_->applyStrengthBuff(effect->strengthTurns, effect->strengthBonus);
        say("Вы выпили зелье силы! +" + std::to_string(effect->strengthBonus) + " к урону на " +
            std::to_string(effect->strengthTurns) + " ходов.");
    }
    inventory_.print(std::cout);
    endTurn();
}

void Game::changeStatus(const Command& cmd) {
    switch (cmd.type) {
        case CommandType::StatusOn:
            hero_->setStatus(cmd.status);
            say("Состояние \"" + cmd.statusName + "\" включено.");
            break;
        case CommandType::StatusOff:
            hero_->clearStatus(cmd.status);
            say("Состояние \"" + cmd.statusName + "\" выключено.");
            break;
        default:
            hero_->toggleStatus(cmd.status);
            say("Состояние \"" + cmd.statusName + "\" переключено (" +
                (hero_->hasStatus(cmd.status) ? "вкл" : "выкл") + ").");
            break;
    }
}

void Game::showStats() const {
    hero_->printStatus(std::cout, floor_);
    std::cout << "Монстров на этаже: " << monsters_.size()
              << " (всего создано за игру: " << Monster::created() << ")\n";
}

void Game::cheatHeroExit() {
    if (map_->findExit()) {
        say("чит активирован");
        descend();
    }
}

void Game::cheatKill() {
    say("Чит активирован");
    hero_->takeDamage(hero_->health());
    endTurn();
}

void Game::cheatDamage() {
    say("чит активирован");
    if (inventory_.add(PotionType::Strength, 1))
        say("Вы нашли зелье: " + Inventory::name(PotionType::Strength));
    else
        say("Инвентарь полон!");
}

void Game::cheatRule(const std::string& name) {
    const std::optional<MoveRule> rule = rules::byName(name);
    if (!rule) {
        say("[Ошибка] Неизвестное правило: " + name + "\nДоступные правила: " + rules::names());
        return;
    }
    say("чит активирован");
    std::for_each(monsters_.begin(), monsters_.end(), [&rule](const std::unique_ptr<Monster>& m) {
        m->setRule(*rule);
    });
    say("Правило хода \"" + name + "\" назначено монстрам: " + std::to_string(monsters_.size()));
}

void Game::showSummary() const {
    const queries::Summary s = queries::summarize(queries::view(monsters_),
                                                  Position{hero_->y(), hero_->x()}, kNearDistance);
    std::ostringstream symbols;
    std::copy(s.symbols.begin(), s.symbols.end(), std::ostream_iterator<char>(symbols, " "));

    std::cout << "____Сводка по монстрам____\n"
              << "Живых монстров: " << s.alive << "\n"
              << "Рядом (не дальше " << kNearDistance << " клеток): " << s.nearby << "\n"
              << "Суммарное здоровье: " << s.totalHealth << "\n"
              << "Суммарный урон за ход: " << s.totalDamage << " (от тех, кто рядом: " << s.nearbyDamage << ")\n"
              << "Символы: " << symbols.str() << "\n";
}

void Game::removeDead() {
    const MonsterView dead = queries::select(queries::view(monsters_), queries::isDead());
    std::for_each(dead.begin(), dead.end(), [this](const Monster* m) {
        say(m->name() + " повержен!");
        hero_->addGold(kKillReward);
        say("Вы получили " + std::to_string(kKillReward) + " золота. Всего золота: " + std::to_string(hero_->gold()));
    });
    monsters_.erase(std::remove_if(monsters_.begin(), monsters_.end(),
                                   [isDead = queries::isDead()](const std::unique_ptr<Monster>& m) {
                                       return isDead(*m);
                                   }),
                    monsters_.end());
}

void Game::endTurn() {
    for (const auto& m : monsters_) {
        if (!hero_->isAlive()) break;
        if (m->isAlive()) m->takeTurn(*this);
    }

    if (hero_->tickBuff()) say("Эффект зелья силы закончился.");
    ++turn_;
    draw();
    if (!hero_->isAlive()) {
        say("Вы погибли… Конец игры.");
        dead_ = true;
        std::cout << kDeathMenu;
    }
}

void Game::draw() const {
    const std::vector<const Creature*> monsters = fp::map(monsters_, [](const std::unique_ptr<Monster>& m) -> const Creature* {
        return m.get();
    });
    renderer_.draw(std::cout, *map_, monsters, *hero_);
}

void Game::descend() {
    const int nextFloor = floor_ + 1;

    std::unique_ptr<Map> nextMap;
    try {
        nextMap = buildFloor(map_->rows() + 2, map_->cols() + 2, nextFloor);
    } catch (const std::invalid_argument&) {
        floor_ = nextFloor;
        victory();
        return;
    } catch (const std::bad_alloc&) {
        say("[Ошибка] Недостаточно памяти для следующего этажа.");
        over_ = true;
        return;
    }
    std::vector<std::unique_ptr<Monster>> nextMonsters = buildMonsters(*nextMap, Position{1, 1}, nextFloor);
    HeroSnapshot s = hero_->snapshot();
    s.y = 1;
    s.x = 1;
    std::unique_ptr<Hero> nextHero = std::make_unique<Hero>(s);

    floor_ = nextFloor;
    turn_ = 1;
    map_ = std::move(nextMap);
    monsters_ = std::move(nextMonsters);
    hero_ = std::move(nextHero);

    say("Вы спустились на этаж " + std::to_string(floor_) + "!");
    draw();
}

void Game::victory() {
    std::cout << "\n!!! ПОДЗЕМЕЛЬЕ РАСШИРИЛОСЬ НАСТОЛЬКО, ЧТО ДАЖЕ ПАМЯТЬ НЕ ВМЕЩАЕТ ЕГО !!!\n"
              << "Вы прошли все доступные этажи и выбрались на поверхность!\n"
              << "Поздравляем, вы победили!\n\n";
    showStats();
    inventory_.print(std::cout);
    over_ = true;
}

void Game::newGame() {
    std::unique_ptr<Map> newMap;
    try {
        newMap = buildFloor(6, 12, 0);
    } catch (const std::exception&) {
        say("[Ошибка] Не удалось загрузить новую игру.");
        return;
    }
    std::unique_ptr<Hero> newHero = std::make_unique<Hero>(1, 1, 100);
    std::vector<std::unique_ptr<Monster>> newMonsters = buildMonsters(*newMap, Position{1, 1}, 0);

    map_ = std::move(newMap);
    hero_ = std::move(newHero);
    monsters_ = std::move(newMonsters);
    inventory_ = Inventory();
    floor_ = 0;
    turn_ = 1;
    dead_ = false;

    say("[Загрузка] Новая игра загружена.");
    std::cout << kLegend << kCommands << '\n';
    draw();
    inventory_.print(std::cout);
}

SaveData Game::makeSaveData() const {
    SaveData d;
    d.mapLines = map_->toLines();
    for (const auto& m : monsters_)
        d.monsters.push_back(MonsterRecord{m->kind(), m->y(), m->x(), m->health(), m->attack()});
    d.hero = hero_->snapshot();
    d.floor = floor_;
    d.turn = turn_;
    d.inventory = inventory_.slots();
    return d;
}

bool Game::saveGame() {
    try {
        SaveFile(savePath_).write(makeSaveData());
    } catch (const std::exception& e) {
        say(std::string("[Ошибка] Ошибка сохранения! ") + e.what());
        return false;
    }
    say("Игра сохранена в " + savePath_);
    return true;
}

void Game::applySave(const SaveData& d) {
    std::unique_ptr<Map> newMap = Map::fromLines(d.mapLines);
    std::unique_ptr<Hero> newHero = std::make_unique<Hero>(d.hero);
    Inventory newInventory(d.inventory);
    if (d.floor < 0 || d.turn < 1) throw std::runtime_error("неверный номер этажа или хода");

    std::set<std::pair<int, int>> taken;
    auto place = [&](const Creature& c) {
        return newMap->isPassable(c.y(), c.x()) && taken.insert({c.y(), c.x()}).second;
    };
    if (!place(*newHero)) throw std::runtime_error("герой стоит в стене или за пределами карты");

    std::vector<std::unique_ptr<Monster>> newMonsters;
    for (const MonsterRecord& r : d.monsters) {
        std::unique_ptr<Monster> m = MonsterFactory::create(r.kind, r.y, r.x, r.hp, r.attack);
        if (!place(*m)) throw std::runtime_error("монстр стоит в стене, за пределами карты или в занятой клетке");
        newMonsters.push_back(std::move(m));
    }

    map_ = std::move(newMap);
    hero_ = std::move(newHero);
    monsters_ = std::move(newMonsters);
    inventory_ = std::move(newInventory);
    floor_ = d.floor;
    turn_ = d.turn;
    dead_ = false;
}

void Game::loadGame() {
    try {
        applySave(SaveFile(savePath_).read());
    } catch (const std::exception& e) {
        say("[Ошибка] Не удалось загрузить сохранение. Проверьте файл.");
        say(e.what());
        return;
    }
    say("[Загрузка] Сохранённый прогресс восстановлен!");
    say("Вы на " + std::to_string(floor_) + " этаже подземелья");
    draw();
    showStats();
    inventory_.print(std::cout);
}

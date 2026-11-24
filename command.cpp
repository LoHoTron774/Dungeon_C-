#include "header/command.h"

#include <cctype>
#include <climits>
#include <vector>

namespace {

std::vector<std::string> splitWords(const std::string& line) {
    std::vector<std::string> words;
    std::string cur;
    for (unsigned char c : line) {
        if (std::isspace(c)) {
            if (!cur.empty()) { words.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(static_cast<char>(std::tolower(c)));
        }
    }
    if (!cur.empty()) words.push_back(cur);
    return words;
}

bool parseDirection(const std::string& w, Direction& dir, const char*& name) {
    if (w == "north" || w == "n") { dir = Direction::North; name = "север";  return true; }
    if (w == "south" || w == "s") { dir = Direction::South; name = "юг";     return true; }
    if (w == "east"  || w == "e") { dir = Direction::East;  name = "восток"; return true; }
    if (w == "west"  || w == "w") { dir = Direction::West;  name = "запад";  return true; }
    return false;
}

bool parseStatus(const std::string& w, HeroStatus& st) {
    if (w == "poisoned")  { st = HeroStatus::Poisoned;  return true; }
    if (w == "blinded")   { st = HeroStatus::Blinded;   return true; }
    if (w == "invisible") { st = HeroStatus::Invisible; return true; }
    if (w == "petrified") { st = HeroStatus::Petrified; return true; }
    if (w == "burning")   { st = HeroStatus::Burning;   return true; }
    if (w == "sleep")     { st = HeroStatus::Sleep;     return true; }
    return false;
}

int parseNumber(const std::string& s, int& out) {
    long long value = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return -1;
        value = value * 10 + (c - '0');
        if (value > INT_MAX) return -2;
    }
    out = static_cast<int>(value);
    return s.empty() ? -1 : 1;
}

constexpr const char* kStatusList = "poisoned, blinded, invisible, petrified, burning, sleep";

Command error(const std::string& text, bool showInventory = false) {
    Command c;
    c.type = CommandType::Error;
    c.message = text;
    c.showInventory = showInventory;
    return c;
}

}

Command CommandParser::parse(const std::string& line) const {
    const std::vector<std::string> words = splitWords(line);
    Command cmd;

    if (words.empty()) { cmd.type = CommandType::Empty; return cmd; }
    const std::string& first = words[0];

    if (first == "quit")      { cmd.type = CommandType::Quit; return cmd; }
    if (first == "help")      { cmd.type = CommandType::Help; return cmd; }
    if (first == "look")      { cmd.type = CommandType::Look; return cmd; }
    if (first == "stats")     { cmd.type = CommandType::Stats; return cmd; }
    if (first == "inventory") { cmd.type = CommandType::Inventory; return cmd; }
    if (first == "new")       { cmd.type = CommandType::New; return cmd; }
    if (first == "load")      { cmd.type = CommandType::Load; return cmd; }
    if (first == "save")      { cmd.type = CommandType::Save; return cmd; }
    if (first == "heroexit")  { cmd.type = CommandType::CheatHeroExit; return cmd; }
    if (first == "hesoyam")   { cmd.type = CommandType::CheatHesoyam; return cmd; }
    if (first == "damage")    { cmd.type = CommandType::CheatDamage; return cmd; }
    if (first == "kill")      { cmd.type = CommandType::CheatKill; return cmd; }

    if (first == "use") {
        if (words.size() < 2)
            return error("[Ошибка] команда use требует номер предмета.\nИспользуйте: use N (N - номер предмета)", true);
        int n = 0;
        switch (parseNumber(words[1], n)) {
            case -1: return error("[Ошибка] аргумент должен содержать только цифры.");
            case -2: return error("[Ошибка] число слишком большое.");
            default: break;
        }
        cmd.type = CommandType::Use;
        cmd.number = n;
        return cmd;
    }

    if (first == "on" || first == "off" || first == "toggle") {
        if (words.size() < 2)
            return error(std::string("[Ошибка] требуется название состояния.\nСостояния: ") + kStatusList);
        if (!parseStatus(words[1], cmd.status))
            return error("[Ошибка] Неизвестное состояние: " + words[1] +
                         "\nДоступные состояния: " + kStatusList);
        cmd.statusName = words[1];
        cmd.type = first == "on" ? CommandType::StatusOn
                 : first == "off" ? CommandType::StatusOff : CommandType::StatusToggle;
        return cmd;
    }

    const std::string* dirWord = &first;
    if (first == "go") {
        if (words.size() < 2) return error("[Ошибка] после go нужно направление: north, south, east, west.");
        dirWord = &words[1];
    }
    if (parseDirection(*dirWord, cmd.direction, cmd.directionName)) {
        cmd.type = CommandType::Move;
        return cmd;
    }
    if (first == "go") return error("[Ошибка] Неизвестное направление: " + *dirWord);

    cmd.type = CommandType::Unknown;
    return cmd;
}

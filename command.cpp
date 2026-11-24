#include "header/command.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <variant>
#include <vector>

#include "header/move_rules.h"

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

struct ParsedDirection {
    Direction direction;
    std::string name;
};

std::optional<ParsedDirection> parseDirection(const std::string& w) {
    if (w == "north" || w == "n") return ParsedDirection{Direction::North, "север"};
    if (w == "south" || w == "s") return ParsedDirection{Direction::South, "юг"};
    if (w == "east"  || w == "e") return ParsedDirection{Direction::East,  "восток"};
    if (w == "west"  || w == "w") return ParsedDirection{Direction::West,  "запад"};
    return std::nullopt;
}

std::optional<HeroStatus> parseStatus(const std::string& w) {
    if (w == "poisoned")  return HeroStatus::Poisoned;
    if (w == "blinded")   return HeroStatus::Blinded;
    if (w == "invisible") return HeroStatus::Invisible;
    if (w == "petrified") return HeroStatus::Petrified;
    if (w == "burning")   return HeroStatus::Burning;
    if (w == "sleep")     return HeroStatus::Sleep;
    return std::nullopt;
}

enum class NumberError { NotANumber, TooLarge };

std::variant<int, NumberError> parseNumber(const std::string& s) {
    const bool onlyDigits = !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isdigit(c) != 0;
    });
    if (!onlyDigits) return NumberError::NotANumber;

    int value = 0;
    const auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec == std::errc::result_out_of_range) return NumberError::TooLarge;
    return value;
}

const std::string kStatusList = "poisoned, blinded, invisible, petrified, burning, sleep";

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
    if (first == "summary")   { cmd.type = CommandType::Summary; return cmd; }

    if (first == "use") {
        if (words.size() < 2)
            return error("[Ошибка] команда use требует номер предмета.\nИспользуйте: use N (N - номер предмета)", true);
        const std::variant<int, NumberError> number = parseNumber(words[1]);
        if (const NumberError* e = std::get_if<NumberError>(&number))
            return error(*e == NumberError::TooLarge ? "[Ошибка] число слишком большое."
                                                    : "[Ошибка] аргумент должен содержать только цифры.");
        cmd.type = CommandType::Use;
        cmd.number = std::get<int>(number);
        return cmd;
    }

    if (first == "rule") {
        if (words.size() < 2)
            return error("[Ошибка] команда rule требует название правила.\nПравила: " + rules::names());
        cmd.type = CommandType::CheatRule;
        cmd.ruleName = words[1];
        return cmd;
    }

    if (first == "on" || first == "off" || first == "toggle") {
        if (words.size() < 2)
            return error("[Ошибка] требуется название состояния.\nСостояния: " + kStatusList);
        const std::optional<HeroStatus> status = parseStatus(words[1]);
        if (!status)
            return error("[Ошибка] Неизвестное состояние: " + words[1] +
                         "\nДоступные состояния: " + kStatusList);
        cmd.status = *status;
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
    if (const std::optional<ParsedDirection> parsed = parseDirection(*dirWord)) {
        cmd.type = CommandType::Move;
        cmd.direction = parsed->direction;
        cmd.directionName = parsed->name;
        return cmd;
    }
    if (first == "go") return error("[Ошибка] Неизвестное направление: " + *dirWord);

    cmd.type = CommandType::Unknown;
    return cmd;
}

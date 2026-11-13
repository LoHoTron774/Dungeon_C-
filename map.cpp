#include "map.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

#define COLOR_EXIT    "\x1b[32m"
#define COLOR_PLAYER  "\x1b[34m"
#define COLOR_MONSTER "\x1b[31m"
#define COLOR_GOLD    "\x1b[33m"
#define COLOR_POTION  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

bool Map::trace_ = false;

void Map::setTrace(bool enabled) { trace_ = enabled; }
bool Map::traceEnabled() { return trace_; }

const char* describe(LoadError e) {
    switch (e) {
        case LoadError::Ok:           return "Уровень загружен";
        case LoadError::FileNotFound: return "Файл уровня не найден или недоступен для чтения";
        case LoadError::BadFormat:    return "Повреждённый формат уровня";
        case LoadError::OutOfMemory:  return "Недостаточно памяти для уровня";
        case LoadError::BadSize:      return "Недопустимый размер уровня";
    }
    return "Неизвестная ошибка";
}

Map::Map(int rowCount, int colCount) {
    if (rowCount < kMinSize || colCount < kMinSize || rowCount > kMaxRows || colCount > kMaxCols)
        throw std::invalid_argument("недопустимый размер карты");
    // Если new[] бросит bad_alloc, конструктор не завершится, объект не будет считаться созданным,
    // и деструктор для него не вызовется — но и утекать нечему: память не выделена.
    cells_ = new char[static_cast<size_t>(rowCount) * colCount];
    rows_ = rowCount;
    cols_ = colCount;
    std::fill(cells_, cells_ + static_cast<size_t>(rows_) * cols_, '.');
    if (trace_) std::cout << "  [Map ctor " << rows_ << "x" << cols_ << "] this=" << this << '\n';
}

Map::~Map() {
    if (trace_) std::cout << "  [Map dtor " << rows_ << "x" << cols_ << "] this=" << this << '\n';
    delete[] cells_;   // единственное место, где освобождается память карты
}

bool Map::inBounds(int y, int x) const {
    return y >= 0 && y < rows_ && x >= 0 && x < cols_;
}

char Map::get(int y, int x) const {
    if (!inBounds(y, x)) throw std::out_of_range("клетка вне карты");
    return cells_[static_cast<size_t>(y) * cols_ + x];
}

void Map::set(int y, int x, char c) {
    if (!inBounds(y, x)) throw std::out_of_range("клетка вне карты");
    cells_[static_cast<size_t>(y) * cols_ + x] = c;
}

bool Map::collectGold(int y, int x) {
    if (!inBounds(y, x) || get(y, x) != 'G') return false;
    set(y, x, '.');
    return true;
}

bool Map::isPassable(int y, int x) const {
    return inBounds(y, x) && get(y, x) != '#';
}

void Map::print(int heroY, int heroX) const {
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            if (y == heroY && x == heroX) {
                std::cout << COLOR_PLAYER << '@' << COLOR_RESET;
                continue;
            }
            char c = get(y, x);
            const char* color = nullptr;
            switch (c) {
                case 'E': color = COLOR_EXIT;    break;
                case 'M': color = COLOR_MONSTER; break;
                case 'G': color = COLOR_GOLD;    break;
                case 'Z': color = COLOR_POTION;  break;
                default: break;
            }
            if (color) std::cout << color << c << COLOR_RESET;
            else       std::cout << c;
        }
        std::cout << '\n';
    }
}

bool Map::canReachExit(int startY, int startX) const {
    std::vector<char> visited(static_cast<size_t>(rows_) * cols_, 0);   // vector сам освободит память
    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(startY, startX);
    const int dy[4] = {-1, 1, 0, 0};
    const int dx[4] = {0, 0, -1, 1};

    while (!stack.empty()) {
        auto [y, x] = stack.back();
        stack.pop_back();
        if (!inBounds(y, x)) continue;
        size_t i = static_cast<size_t>(y) * cols_ + x;
        if (visited[i] || cells_[i] == '#') continue;
        visited[i] = 1;
        if (cells_[i] == 'E') return true;
        for (int k = 0; k < 4; ++k) stack.emplace_back(y + dy[k], x + dx[k]);
    }
    return false;
}

void Map::generate(std::mt19937& rng) {
    std::fill(cells_, cells_ + static_cast<size_t>(rows_) * cols_, '.');

    for (int y = 0; y < rows_; ++y)
        for (int x = 0; x < cols_; ++x)
            if (y == 0 || y == rows_ - 1 || x == 0 || x == cols_ - 1) set(y, x, '#');

    std::vector<std::pair<int, int>> free;
    for (int y = 1; y < rows_ - 1; ++y)
        for (int x = 1; x < cols_ - 1; ++x)
            if (!(y == 1 && x == 1)) free.emplace_back(y, x);
    std::shuffle(free.begin(), free.end(), rng);

    size_t pos = 0;
    size_t wallCount = std::min(free.size() > 1 ? free.size() - 1 : 0,
                                static_cast<size_t>((rows_ - 2) * (cols_ - 2) * 0.35));
    std::vector<std::pair<int, int>> walls;
    for (size_t i = 0; i < wallCount; ++i, ++pos) {
        set(free[pos].first, free[pos].second, '#');
        walls.push_back(free[pos]);
    }
    set(free[pos].first, free[pos].second, 'E');
    ++pos;
    for (int i = 0; i < 6 && pos < free.size(); ++i, ++pos)
        set(free[pos].first, free[pos].second, 'G');

    for (size_t i = 0; i < walls.size() && !canReachExit(1, 1); ++i)
        set(walls[i].first, walls[i].second, '.');
}

LoadError Map::load(const std::string& path, std::optional<Map>& out) {
    out.reset();   // на случай повторного вызова: до успеха карты нет

    try {
        std::ifstream in(path);
        if (!in.is_open()) return LoadError::FileNotFound;   // ifstream закроется сам (RAII)

        // 1. Читаем строки во временный vector<string>. Он сам освободит память при любом выходе
        //    из функции: и через return с ошибкой, и при исключении.
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();   // файлы из Windows
            lines.push_back(line);
            if (lines.size() > static_cast<size_t>(Map::kMaxRows) + 1)   // +1: возможна пустая строка в конце
                return LoadError::BadSize;
        }
        if (in.bad()) return LoadError::FileNotFound;

        // Пустые строки в конце файла — не часть карты
        while (!lines.empty() && lines.back().empty()) lines.pop_back();
        if (lines.empty()) return LoadError::BadFormat;   // пустой файл

        // 2. Проверяем формат: все строки одной длины, допустимые символы
        const size_t width = lines[0].size();
        for (const std::string& l : lines) {
            if (l.size() != width) return LoadError::BadFormat;
            for (char c : l)
                if (c != '#' && c != '.' && c != 'G' && c != 'E') return LoadError::BadFormat;
        }

        // 3. Проверяем размер
        const size_t h = lines.size();
        if (h < static_cast<size_t>(Map::kMinSize) || h > static_cast<size_t>(Map::kMaxRows) ||
            width < static_cast<size_t>(Map::kMinSize) || width > static_cast<size_t>(Map::kMaxCols))
            return LoadError::BadSize;

        // 4. Герой стартует в (1,1) — там не должно быть стены
        if (lines[1][1] == '#') return LoadError::BadFormat;

        // 5. Только теперь, когда файл проверен, создаём объект Map — сразу на месте внутри optional.
        //    Если здесь бросится bad_alloc, Map не создан, а lines освободится автоматически.
        out.emplace(static_cast<int>(h), static_cast<int>(width));
        for (size_t y = 0; y < h; ++y)
            for (size_t x = 0; x < width; ++x)
                out->set(static_cast<int>(y), static_cast<int>(x), lines[y][x]);
        return LoadError::Ok;
    } catch (const std::bad_alloc&) {
        out.reset();
        return LoadError::OutOfMemory;
    } catch (const std::invalid_argument&) {
        out.reset();
        return LoadError::BadSize;
    }
}

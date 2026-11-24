#include "header/map.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <new>
#include <stdexcept>
#include <utility>

#include "header/lifecycle_log.h"

std::string describe(LoadError e) {
    switch (e) {
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
    rows_ = rowCount;
    cols_ = colCount;
    cells_.assign(static_cast<std::size_t>(rows_) * cols_, '.');
    lifecycle::log("Map ctor " + std::to_string(rows_) + "x" + std::to_string(cols_), this);
}

Map::~Map() {
    lifecycle::log("Map dtor " + std::to_string(rows_) + "x" + std::to_string(cols_), this);
}

bool Map::inBounds(int y, int x) const {
    return y >= 0 && y < rows_ && x >= 0 && x < cols_;
}

char Map::get(int y, int x) const {
    if (!inBounds(y, x)) throw std::out_of_range("клетка вне карты");
    return cells_[static_cast<std::size_t>(y) * cols_ + x];
}

void Map::set(int y, int x, char c) {
    if (!inBounds(y, x)) throw std::out_of_range("клетка вне карты");
    cells_[static_cast<std::size_t>(y) * cols_ + x] = c;
}

bool Map::isPassable(int y, int x) const {
    return inBounds(y, x) && get(y, x) != '#';
}

std::optional<Position> Map::findExit() const {
    for (int y = 0; y < rows_; ++y)
        for (int x = 0; x < cols_; ++x)
            if (get(y, x) == 'E') return Position{y, x};
    return std::nullopt;
}

std::vector<std::string> Map::toLines() const {
    std::vector<std::string> lines;
    lines.reserve(static_cast<std::size_t>(rows_));
    for (int y = 0; y < rows_; ++y)
        lines.emplace_back(cells_.begin() + static_cast<std::ptrdiff_t>(y) * cols_,
                           cells_.begin() + static_cast<std::ptrdiff_t>(y + 1) * cols_);
    return lines;
}

Map::Item Map::takeItem(int y, int x) {
    if (!inBounds(y, x)) return Item::None;
    const char c = get(y, x);
    if (c == 'G') { set(y, x, '.'); return Item::Gold; }
    if (c == 'Z') { set(y, x, '.'); return Item::Potion; }
    return Item::None;
}

bool Map::canReachExit(int startY, int startX) const {
    std::vector<char> visited(static_cast<std::size_t>(rows_) * cols_, 0);
    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(startY, startX);
    constexpr std::array<Position, 4> steps{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    while (!stack.empty()) {
        auto [y, x] = stack.back();
        stack.pop_back();
        if (!inBounds(y, x)) continue;
        std::size_t i = static_cast<std::size_t>(y) * cols_ + x;
        if (visited[i] || cells_[i] == '#') continue;
        visited[i] = 1;
        if (cells_[i] == 'E') return true;
        for (const Position& step : steps) stack.emplace_back(y + step.y, x + step.x);
    }
    return false;
}

void Map::generate(std::mt19937& rng, int goldCount, int potionCount) {
    std::fill(cells_.begin(), cells_.end(), '.');

    for (int y = 0; y < rows_; ++y)
        for (int x = 0; x < cols_; ++x)
            if (y == 0 || y == rows_ - 1 || x == 0 || x == cols_ - 1) set(y, x, '#');

    std::vector<std::pair<int, int>> free;
    for (int y = 1; y < rows_ - 1; ++y)
        for (int x = 1; x < cols_ - 1; ++x)
            if (!(y == 1 && x == 1)) free.emplace_back(y, x);
    std::shuffle(free.begin(), free.end(), rng);

    std::size_t pos = 0;

    const std::size_t wallCount = std::min(free.size() > 1 ? free.size() - 1 : 0,
                                      static_cast<std::size_t>((rows_ - 2) * (cols_ - 2) * 0.35));
    std::vector<std::pair<int, int>> walls;
    for (std::size_t i = 0; i < wallCount; ++i, ++pos) {
        set(free[pos].first, free[pos].second, '#');
        walls.push_back(free[pos]);
    }

    set(free[pos].first, free[pos].second, 'E');
    ++pos;

    for (int i = 0; i < goldCount && pos < free.size(); ++i, ++pos)
        set(free[pos].first, free[pos].second, 'G');

    for (int i = 0; i < potionCount && pos < free.size(); ++i, ++pos)
        set(free[pos].first, free[pos].second, 'Z');

    for (std::size_t i = 0; i < walls.size() && !canReachExit(1, 1); ++i)
        set(walls[i].first, walls[i].second, '.');
}

std::unique_ptr<Map> Map::fromLines(const std::vector<std::string>& lines) {
    if (lines.empty()) throw LevelLoadError(LoadError::BadFormat);

    const std::size_t width = lines[0].size();
    for (const std::string& l : lines) {
        if (l.size() != width) throw LevelLoadError(LoadError::BadFormat);
        for (char c : l)
            if (c != '#' && c != '.' && c != 'G' && c != 'E' && c != 'Z') throw LevelLoadError(LoadError::BadFormat);
    }

    const std::size_t h = lines.size();
    if (h < static_cast<std::size_t>(kMinSize) || h > static_cast<std::size_t>(kMaxRows) ||
        width < static_cast<std::size_t>(kMinSize) || width > static_cast<std::size_t>(kMaxCols))
        throw LevelLoadError(LoadError::BadSize);

    try {
        std::unique_ptr<Map> m = std::make_unique<Map>(static_cast<int>(h), static_cast<int>(width));
        for (std::size_t y = 0; y < h; ++y)
            for (std::size_t x = 0; x < width; ++x)
                m->set(static_cast<int>(y), static_cast<int>(x), lines[y][x]);
        return m;
    } catch (const std::bad_alloc&) {
        throw LevelLoadError(LoadError::OutOfMemory);
    } catch (const std::invalid_argument&) {
        throw LevelLoadError(LoadError::BadSize);
    }
}

std::unique_ptr<Map> Map::load(const std::string& path) {
    try {
        std::ifstream in(path);
        if (!in.is_open()) throw LevelLoadError(LoadError::FileNotFound);

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
            if (lines.size() > static_cast<std::size_t>(kMaxRows) + 1) throw LevelLoadError(LoadError::BadSize);
        }
        if (in.bad()) throw LevelLoadError(LoadError::FileNotFound);

        while (!lines.empty() && lines.back().empty()) lines.pop_back();

        std::unique_ptr<Map> map = fromLines(lines);
        if (map->get(1, 1) == '#') throw LevelLoadError(LoadError::BadFormat);
        return map;
    } catch (const std::bad_alloc&) {
        throw LevelLoadError(LoadError::OutOfMemory);
    }
}

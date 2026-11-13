#ifndef MAP_H
#define MAP_H

#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "types.h"

enum class LoadError {
    Ok,
    FileNotFound,
    BadFormat,
    OutOfMemory,
    BadSize
};

const char* describe(LoadError e);

class Map {
public:
    static constexpr int kMinSize = 5;
    static constexpr int kMaxRows = 200;
    static constexpr int kMaxCols = 119;

    enum class Item { None, Gold, Potion };

    Map(int rowCount, int colCount);
    ~Map();

    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    bool inBounds(int y, int x) const;
    char get(int y, int x) const;
    bool isPassable(int y, int x) const;
    std::optional<Position> findExit() const;
    std::vector<std::string> toLines() const;

    Item takeItem(int y, int x);
    void generate(std::mt19937& rng, int goldCount, int potionCount);

    [[nodiscard]] static LoadError fromLines(const std::vector<std::string>& lines, std::unique_ptr<Map>& out);
    [[nodiscard]] static LoadError load(const std::string& path, std::unique_ptr<Map>& out);

private:
    void set(int y, int x, char c);
    bool canReachExit(int startY, int startX) const;

    int rows_ = 0;
    int cols_ = 0;
    char* cells_ = nullptr;
};

#endif

#ifndef MAP_H
#define MAP_H

#include <optional>
#include <random>
#include <string>

// Результат загрузки уровня из файла.
enum class LoadError {
    Ok,
    FileNotFound,   // файл не открылся
    BadFormat,      // пустой файл, строки разной длины, неизвестные символы, нет места для героя
    OutOfMemory,    // не хватило памяти
    BadSize         // размеры вне допустимых
};

const char* describe(LoadError e);   // понятное сообщение для пользователя

// Практика 10: карта сама владеет памятью под клетки (RAII).
// Конструктор выделяет, деструктор освобождает — больше никто память не трогает.
class Map {
public:
    static constexpr int kMinSize = 5;
    static constexpr int kMaxRows = 200;
    static constexpr int kMaxCols = 119;

    int rows = 0;
    int cols = 0;
    char* cells = nullptr;   // владеем: выделено в конструкторе, освобождается в деструкторе

    // Размер задаётся при создании. Бросает std::invalid_argument при плохом размере
    // и std::bad_alloc при нехватке памяти — в обоих случаях объект НЕ создаётся.
    Map(int rowCount, int colCount);
    ~Map();

    // Объект владеет блоком памяти, поэтому копирование запрещено:
    // копия по умолчанию скопировала бы указатель, и два объекта освобождали бы один блок.
    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    bool inBounds(int y, int x) const;
    char get(int y, int x) const;
    void set(int y, int x, char c);
    bool isPassable(int y, int x) const;
    void print(int heroY, int heroX) const;
    void generate(std::mt19937& rng);   // заполняет карту случайно (размер уже задан конструктором)

    // Загрузка уровня из текстового файла. Код ошибки — в возвращаемом значении,
    // результат — в out. При любой ошибке out остаётся пустым: объект Map не создаётся вообще.
    [[nodiscard]] static LoadError load(const std::string& path, std::optional<Map>& out);

    static bool trace;   // печатать сообщения конструктора/деструктора

private:
    bool canReachExit(int startY, int startX) const;
};

#endif // MAP_H

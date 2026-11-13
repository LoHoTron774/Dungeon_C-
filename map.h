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

// Практика 11: инкапсуляция. Размеры и массив клеток закрыты (private):
// снаружи их нельзя ни подменить, ни испортить, ни сделать delete на cells.
// Клетки можно только читать (get/isPassable/print). Изменяются они лишь
// самой картой: generate(), load() и collectGold().
class Map {
public:
    // Константы доступны всем, но изменить их нельзя (constexpr).
    static constexpr int kMinSize = 5;
    static constexpr int kMaxRows = 200;
    static constexpr int kMaxCols = 119;

    // Бросает std::invalid_argument при плохом размере и std::bad_alloc при нехватке памяти.
    Map(int rowCount, int colCount);
    ~Map();

    // Объект владеет блоком памяти, поэтому копирование запрещено.
    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    // --- чтение состояния ---
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    bool inBounds(int y, int x) const;
    char get(int y, int x) const;                // вне карты -> std::out_of_range
    bool isPassable(int y, int x) const;
    void print(int heroY, int heroX) const;

    // --- изменения, которые карта делает сама ---
    // Если в клетке золото: убирает его (клетка становится '.') и возвращает true.
    bool collectGold(int y, int x);
    void generate(std::mt19937& rng);            // случайное заполнение (размер уже задан)

    // Загрузка уровня из текстового файла. Код ошибки — в возвращаемом значении,
    // результат — в out. При любой ошибке out остаётся пустым.
    [[nodiscard]] static LoadError load(const std::string& path, std::optional<Map>& out);

    static void setTrace(bool enabled);          // сообщения конструктора/деструктора
    static bool traceEnabled();

private:
    void set(int y, int x, char c);              // только для внутреннего использования
    bool canReachExit(int startY, int startX) const;

    int rows_ = 0;
    int cols_ = 0;
    char* cells_ = nullptr;   // владеем: выделено в конструкторе, освобождается в деструкторе

    static bool trace_;
};

#endif // MAP_H

#include <exception>
#include <iostream>
#include <random>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "header/demos.h"
#include "header/game.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::string levelPath;
    std::string savePath;
    unsigned seed = std::random_device{}();

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--demo") {
            lifetimeDemo();
            return 0;
        } else if (arg == "--virtual-demo") {
            virtualDestructorDemo();
            return 0;
        } else if (arg == "--load-demo" || arg == "--seed" || arg == "--save") {
            if (i + 1 >= argc) {
                std::cerr << "[Ошибка] После " << arg << " нужен аргумент\n";
                return 2;
            }
            const std::string value = argv[++i];
            if (arg == "--load-demo") {
                loadDemo(value);
                return 0;
            }
            if (arg == "--save") {
                savePath = value;
                continue;
            }
            try {
                seed = static_cast<unsigned>(std::stoul(value));
            } catch (const std::exception&) {
                std::cerr << "[Ошибка] --seed ожидает число, получено: " << value << '\n';
                return 2;
            }
        } else {
            levelPath = arg;
        }
    }

    try {
        Game game(std::mt19937(seed), levelPath);
        if (!savePath.empty()) game.setSavePath(savePath);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "[Ошибка] " << e.what();
        if (!levelPath.empty()) std::cerr << ": " << levelPath;
        std::cerr << '\n';
        return 1;
    }
    return 0;
}

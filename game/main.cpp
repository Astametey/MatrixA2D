#include "MatrixA2D.h"
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[]) {
    std::locale::global(std::locale("Russian"));
    // Устанавливаем рабочий каталог
    std::cout << "cerrent path " << std::filesystem::current_path() << std::endl;

    MatrixA2D game;
    game.run();
    return 0;
}
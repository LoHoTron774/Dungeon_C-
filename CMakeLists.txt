cmake_minimum_required(VERSION 3.10)
project(dungeon_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dungeon
        main.cpp
        map.cpp
        hero.cpp
)

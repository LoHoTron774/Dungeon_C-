cmake_minimum_required(VERSION 3.10)
project(dungeon_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dungeon
        main.cpp
        demos.cpp
        game.cpp
        command.cpp
        savefile.cpp
        renderer.cpp
        map.cpp
        creature.cpp
        hero.cpp
        monsters.cpp
        inventory.cpp
        lifecycle_log.cpp
)

if(MINGW)
    target_link_options(dungeon PRIVATE -static-libgcc -static-libstdc++)

    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    if(EXISTS "${MINGW_BIN_DIR}/libwinpthread-1.dll")
        add_custom_command(TARGET dungeon POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                "$<TARGET_FILE_DIR:dungeon>")
    endif()
endif()
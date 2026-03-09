cmake_minimum_required(VERSION 3.16)

# vcpkg toolchain
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(Chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Console target (always builds)
add_executable(Chess
    src/main.cpp
    src/Board.cpp
    src/MoveGenerator.cpp
    src/Game.cpp
)
target_include_directories(Chess PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GUI target (only if SDL2 found)
find_package(SDL2 CONFIG QUIET)
find_package(SDL2_image CONFIG QUIET)

if(SDL2_FOUND AND SDL2_image_FOUND)
    add_executable(ChessGUI
        src/main.cpp
        src/Board.cpp
        src/MoveGenerator.cpp
        src/Game.cpp
        src/GUI.cpp
    )
    target_include_directories(ChessGUI PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(ChessGUI PRIVATE HAS_GUI=1)
    target_link_libraries(ChessGUI PRIVATE
        SDL2::SDL2 SDL2::SDL2main
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    )
    message(STATUS "SDL2 found -- ChessGUI target enabled")
else()
    message(STATUS "SDL2 not found -- console-only build")
endif()
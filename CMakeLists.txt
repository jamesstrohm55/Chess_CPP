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
    src/AI.cpp
)
target_include_directories(Chess PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GUI target (only if SDL2 found)
find_package(SDL2 CONFIG QUIET)
find_package(SDL2_image CONFIG QUIET)
find_package(SDL2_ttf CONFIG QUIET)

# --- Emscripten Web Build ---
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_executable(ChessWeb
        src/main.cpp
        src/Board.cpp
        src/MoveGenerator.cpp
        src/Game.cpp
        src/GUI.cpp
        src/AI.cpp
    )
    target_include_directories(ChessWeb PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(ChessWeb PRIVATE HAS_GUI=1)
    target_compile_options(ChessWeb PRIVATE
        -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2
        "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\"]"
    )
    target_link_options(ChessWeb PRIVATE
        -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2
        "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\"]"
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=33554432
        "--preload-file" "${CMAKE_SOURCE_DIR}/assets@assets"
        "--shell-file" "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
    set_target_properties(ChessWeb PROPERTIES OUTPUT_NAME "chess")
    message(STATUS "Emscripten detected -- ChessWeb target enabled")
endif()

# GUI target (only if SDL2 found)
if(NOT EMSCRIPTEN)
    if(SDL2_FOUND AND SDL2_image_FOUND AND SDL2_ttf_FOUND)
        add_executable(ChessGUI
            src/main.cpp
            src/Board.cpp
            src/MoveGenerator.cpp
            src/Game.cpp
            src/GUI.cpp
            src/AI.cpp
        )
        target_include_directories(ChessGUI PRIVATE ${CMAKE_SOURCE_DIR}/include)
        target_compile_definitions(ChessGUI PRIVATE HAS_GUI=1)
        target_link_libraries(ChessGUI PRIVATE
            SDL2::SDL2 SDL2::SDL2main
            $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
            $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        )
        message(STATUS "SDL2 + SDL2_image + SDL2_ttf found -- ChessGUI target enabled")
    else()
        message(STATUS "SDL2 not found -- console-only build")
    endif()
endif()
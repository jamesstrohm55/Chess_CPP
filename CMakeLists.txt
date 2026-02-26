cmake_minimum_required(VERSION 3.16)
project(Chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Chess src/main.cpp src/Board.cpp)

target_include_directories(Chess PRIVATE ${CMAKE_SOURCE_DIR}/include)
cmake_minimum_required(VERSION 3.1...3.27)
project(raytracer
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-g -Wall -Wextra -Wpedantic -Wl,--stack,16777216)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)


add_subdirectory(googletest)
add_subdirectory(geometry)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(raytracer raytracer.cc
        window.h)
target_include_directories(raytracer PUBLIC geometry)
target_link_libraries(raytracer ${SDL2_LIBRARIES})
add_executable(playground playground.cpp)

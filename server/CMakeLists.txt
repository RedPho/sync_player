cmake_minimum_required(VERSION 3.27)
project(mpv)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
find_package(SDL3 REQUIRED)
find_package(SDL3_net REQUIRED)

pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)

add_executable(mpv
        simple.cpp)

target_link_libraries(mpv PkgConfig::MPV SDL3::SDL3 SDL3_net::SDL3_net)
cmake_minimum_required(VERSION 3.27)
project(mpv)

set(CMAKE_CXX_STANDARD 14)

# Find SDL3 and SDL3_net
find_package(SDL3 REQUIRED)
find_package(SDL3_net REQUIRED)

if(CMAKE_CROSSCOMPILING)
    # Cross-compilation settings
    set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

    # Include and library paths for cross-compiling
    include_directories(/usr/x86_64-w64-mingw32/include)
    link_directories(/usr/x86_64-w64-mingw32/lib)

    # Find and link MPV library
    find_library(MPV_LIBRARY NAMES mpv HINTS /usr/x86_64-w64-mingw32/lib)

    if(NOT MPV_LIBRARY)
        message(FATAL_ERROR "Could not find libmpv in the specified paths.")
    endif()
else()
    # For native (Linux) builds
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
endif()

add_executable(sync_player_server main.cpp)

if(CMAKE_CROSSCOMPILING)
    # Link libraries for cross-compiling
    target_link_libraries(sync_player_server ${MPV_LIBRARY} SDL3::SDL3 SDL3_net::SDL3_net)
else()
    # Link libraries for native builds
    target_link_libraries(sync_player_server PkgConfig::MPV SDL3::SDL3 SDL3_net::SDL3_net)
endif()

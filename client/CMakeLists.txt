cmake_minimum_required(VERSION 3.27)
project(mpv)

set(CMAKE_CXX_STANDARD 14)

# Find SDL3 and SDL3_net
find_package(SDL3 REQUIRED)
find_package(SDL3_net REQUIRED)

if(WIN32)

    # Include and library paths for Windows (64-bit)
    include_directories("C:/Program Files (x86)/SDL3/include" "C:/Program Files (x86)/SDL3_net/include" "C:/Program Files (x86)/Libmpv/include")
    link_directories("C:/Program Files (x86)/SDL3/lib" "C:/Program Files (x86)/SDL3_net/lib" "C:/Program Files (x86)/Libmpv")

    # Manually specify the libmpv library
    set(LIBMPV_LIBRARIES "C:/Program Files (x86)/Libmpv/libmpv.dll.a")
else()
    # For native (Linux) builds
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
endif()

add_executable(mpv simple.cpp)

if(WIN32)
    # Link libraries for Windows
    target_link_libraries(mpv ${LIBMPV_LIBRARIES} SDL3::SDL3 SDL3_net::SDL3_net)
else()
    # Link libraries for native (Linux) builds
    target_link_libraries(mpv PkgConfig::MPV SDL3::SDL3 SDL3_net::SDL3_net)
endif()

cmake_minimum_required(VERSION 3.10)
project(sync_player_server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for ASIO and other project headers
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

# Find the MPV library
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)

# Include directories for MPV
include_directories(${MPV_INCLUDE_DIRS})

# Source files
set(SOURCES main.cpp)

# Platform-specific settings
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
    set(PLATFORM_LIBS ws2_32) # Add ws2_32 for Windows socket
else()
    set(PLATFORM_LIBS pthread) # Add pthread for Linux threading
endif()

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} networkData.hh)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${MPV_LIBRARIES} ${PLATFORM_LIBS})

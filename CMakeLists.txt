cmake_minimum_required(VERSION 3.10)

project(Lightning VERSION 1.0 LANGUAGES CXX)

# Use a single authoritative C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gather sources (re-run when files change)
file(GLOB_RECURSE LIGHTNING_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(lightning ${LIGHTNING_SOURCES})
target_include_directories(lightning PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()
add_test(NAME LightningTests COMMAND lightning)
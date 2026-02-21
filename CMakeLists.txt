cmake_minimum_required(VERSION 3.10)

project(Lightning VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lightning_lexer STATIC src/lexer.cpp)
target_include_directories(lightning_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(lexer_tests tests/lexer_test.cpp)
target_link_libraries(lexer_tests PRIVATE lightning_lexer)

enable_testing()
add_test(NAME LexerTests COMMAND lexer_tests)

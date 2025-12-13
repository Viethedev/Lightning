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

# Basic functional test: run the executable and check it prints a Function token
if(WIN32)
	# Ensure MSYS2 UCRT64 runtime DLLs are found when CTest runs the executable
	add_test(NAME LexerSample
			 COMMAND ${CMAKE_COMMAND} -E env "PATH=C:/msys64/ucrt64/bin;$ENV{PATH}" lightning)
else()
	add_test(NAME LexerSample COMMAND lightning)
endif()
# The demo now prints "Parse succeeded" on success; use that as the success regex
set_tests_properties(LexerSample PROPERTIES PASS_REGULAR_EXPRESSION "Parse succeeded")

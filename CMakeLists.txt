cmake_minimum_required(VERSION 3.25)
project(MicaSoftwareManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /EHsc /O2)
else()
    add_compile_options(-Wall -Wextra -pedantic -Wno-missing-designated-field-initializers -O3)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(include)

# 1. Core Engine Static Library
set(MICAMGR_SOURCES
    src/package.cpp
    src/catalog.cpp
    src/registry_scanner.cpp
    src/update_engine.cpp
    src/sideload.cpp
    src/manager.cpp
    src/cli.cpp
    src/ui.cpp
)

add_library(micamgr_core STATIC ${MICAMGR_SOURCES})
target_include_directories(micamgr_core PUBLIC include)

if (WIN32)
    target_link_libraries(micamgr_core PUBLIC advapi32 user32 gdi32 shell32 ws2_32)
endif()

# 2. Main Mica Software Manager Executable (micamgr.exe)
add_executable(micamgr src/main.cpp)
target_link_libraries(micamgr PRIVATE micamgr_core)

# 3. Verification Test Suite Executable
add_executable(micamgr_tests tests/test_main.cpp)
target_link_libraries(micamgr_tests PRIVATE micamgr_core)

enable_testing()
add_test(NAME MicaSoftwareManagerVerification COMMAND micamgr_tests)

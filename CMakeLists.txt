cmake_minimum_required(VERSION 3.14)
project(CellSimulation VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build raylib as shared library" OFF)
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_GAMES   OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_CMAKE   OFF CACHE BOOL "" FORCE)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

find_package(OpenMP REQUIRED)

set(SOURCES
    Biomass.cpp Field.cpp Food.cpp Antibiotic.cpp
    Graphs.cpp main.cpp visualization_modified.cpp
)

add_executable(simulation_test_mod ${SOURCES})

target_include_directories(simulation_test_mod PRIVATE ${raylib_SOURCE_DIR}/src)
target_link_libraries(simulation_test_mod PRIVATE raylib OpenMP::OpenMP_CXX)

if(WIN32)
    target_compile_definitions(simulation_test_mod PRIVATE RAYLIB_STATIC)
endif()

message(STATUS "✅ Проект настроен. Raylib будет скачан автоматически.")
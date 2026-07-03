cmake_minimum_required(VERSION 3.14)
project(CellSimulation VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Путь к исходникам Raylib 6.0 (измените под свой путь)
# ============================================================
if(NOT RAYLIB_SOURCE_DIR)
    set(RAYLIB_SOURCE_DIR "C:/Users/User/Documents/GitHub/cell_sim/build/raylib-6.0" CACHE PATH "Path to raylib source directory")
endif()

if(NOT EXISTS ${RAYLIB_SOURCE_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Raylib source not found at ${RAYLIB_SOURCE_DIR}")
endif()

# ============================================================
# Настройка сборки Raylib
# ============================================================
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_GAMES   OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_CMAKE   OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)   # статическая библиотека

add_subdirectory(${RAYLIB_SOURCE_DIR} ${CMAKE_BINARY_DIR}/raylib-build)

# ============================================================
# OpenMP
# ============================================================
find_package(OpenMP REQUIRED)

# ============================================================
# Ваши исходники
# ============================================================
set(SOURCES
    Biomass.cpp
    Field.cpp
    Food.cpp
    Antibiotic.cpp
    Graphs.cpp
    main.cpp
    visualization_modified.cpp
)

add_executable(simulation_test_mod ${SOURCES})

# ============================================================
# Подключение библиотек
# ============================================================
target_include_directories(simulation_test_mod PRIVATE ${RAYLIB_SOURCE_DIR}/src)
target_link_libraries(simulation_test_mod
    PRIVATE
        raylib
        OpenMP::OpenMP_CXX
)

# Для статической сборки на Windows
if(WIN32)
    target_compile_definitions(simulation_test_mod PRIVATE RAYLIB_STATIC)
endif()

message(STATUS "✅ Проект настроен с локальным Raylib 6.0 из ${RAYLIB_SOURCE_DIR}")
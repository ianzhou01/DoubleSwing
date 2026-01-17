cmake_minimum_required(VERSION 3.21)
project(DoubleSwing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Core library (physics engine) ----
add_library(doubleswing_core
        src/core/engine.cpp
        src/core/drag.cpp
)

target_include_directories(doubleswing_core
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ---- SFML app ----
add_executable(doubleswing_sfml
        src/app_sfml/main.cpp
)

target_link_libraries(doubleswing_sfml
        PRIVATE
        doubleswing_core
        sfml-system sfml-window sfml-graphics
)

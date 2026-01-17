cmake_minimum_required(VERSION 3.21)
project(DoubleSwing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# -------- SFML (only needed for the SFML frontend target) --------
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(SFML)

# -------- Core library (no SFML) --------
add_library(doubleswing_core
        src/core/engine.cpp
        src/core/drag.cpp
)
target_include_directories(doubleswing_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

# -------- SFML app --------
add_executable(doubleswing_sfml
        src/app_sfml/main.cpp
        src/app_sfml/sfml_app.cpp
)

target_link_libraries(doubleswing_sfml PRIVATE
        doubleswing_core
        sfml-graphics
        sfml-window
        sfml-system
)

cmake_minimum_required(VERSION 3.27)
project(DoublePendulumSim)

set(CMAKE_CXX_STANDARD 17)

add_executable(DoublePendulumSim src/main.cpp
        src/pendulum.cpp
        src/simulation.cpp
        src/func.cpp
        src/double_pend.cpp)

target_link_libraries(DoublePendulumSim sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML 2.5.1 COMPONENTS system window graphics audio network REQUIRED)

include_directories(c:/SFML/include/SFML)

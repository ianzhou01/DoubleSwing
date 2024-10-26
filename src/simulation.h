#pragma once
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <random>
#include "pendulum.h"
#include "double_pend.h"

const int FPS = 60;
const int WIDTH = 800;
const int HEIGHT = 800;
const float PIVOT_RAD = 10;
const float CENTER_X = (float)WIDTH / 2;
const float CENTER_Y = (float)(HEIGHT) / 2;
const int MAX_MOUSE_HISTORY = 40;

class Simulation {
    sf::RenderWindow win;
    DoublePendulum doublePendulum;
    std::vector<sf::RectangleShape> guidelines;
    sf::CircleShape pivot;

    bool isDraggingP1, isDraggingP2;
    sf::Clock clock;
//    float dt;
//    std::vector<std::pair<sf::Vector2f, float>> mouseHistoryP1;
//    std::vector<std::pair<sf::Vector2f, float>> mouseHistoryP2;

    void draw_all();
    void handle_click(sf::Event &event);
    void handle_mouse_move(sf::Event &event);
    void handle_mouse_release(sf::Event &event);
//    bool handle_mouse_move_buttons(int x, int y);
//    bool handle_mouse_move_nfs(int x, int y);

    static float distance(float x1, float y1, float x2, float y2);
public:
    Simulation();
    void operator()();
};


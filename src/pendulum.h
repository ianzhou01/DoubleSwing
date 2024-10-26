#pragma once
#include <cmath>
#include <vector>
#include <deque>
#include <SFML/Graphics.hpp>
#include "func.h"

const float g = 9.80665f;
const sf::Color ARM_CLR = sf::Color(50, 50, 50);
const float ARM_W = 5;
const float WEIGHT_RAD = 15;
const int PX_METER_RATIO = 40;

class Pendulum {
    float len;
    float theta;
    float accel;
    float omega;
    float friction;
    sf::RectangleShape arm;

    void integrateEuler();
    void integrate_RK4(float dt);
    float calculate_single_accel(float theta);

public:
    Pendulum(float theta, float len);
    Pendulum(float theta, float len, float fric);
//    Pendulum(float theta, float len, int weightSize);
//    Pendulum(float theta, float len, float fric, int weightSize);
    void update();
    void setCoords(sf::RenderWindow &win, int centerX, int centerY);
    void draw(sf::RenderWindow &win, int centerX, int centerY, bool paused);
    void setColor(sf::Color clr);
    friend class DoublePendulum;

    float getLen() const;
    float getTheta() const;
    float getSpeed() const;

    void setTheta(float theta);
    void setSpeed(float v);

    sf::CircleShape weight;
};



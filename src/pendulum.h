#pragma once
#include <cmath>
#include <vector>
#include <deque>
#include <SFML/Graphics.hpp>
#include "func.h"

const float g = 9.80665f;
const float RAD_LIMIT = Utility::deg_to_rad(10000.0f);
const sf::Color ARM_CLR = sf::Color(50, 50, 50);
const float ARM_W = 5;
const float WEIGHT_RAD = 15;
const int PX_METER_RATIO = 40;

class Pendulum {
    float len;
    float thetaRaw;
    float thetaNormal;
    float accel;
    float omega;
    float friction;
    sf::RectangleShape arm;

    void integrateEuler();
    void integrate_RK4(float dt);
    float calculate_single_accel(float theta);
    void preventOverflow();

public:
    Pendulum(float theta, float len);
    Pendulum(float theta, float len, float fric);
//    Pendulum(float thetaRaw, float len, int weightSize);
//    Pendulum(float thetaRaw, float len, float fric, int weightSize);
    void update();
    void setCoords(sf::RenderWindow &win, int centerX, int centerY);
    void draw(sf::RenderWindow &win, int centerX, int centerY, bool paused);
    void setColor(sf::Color clr);
    friend class DoublePendulum;

    float getLen() const;
    float getThetaRaw() const;
    float getThetaNorm() const;
    float getSpeed() const;
    float getAccel() const;

    void setTheta(float theta);
    void setSpeed(float v);
    void setAccel(float acc);

    sf::CircleShape weight;
};



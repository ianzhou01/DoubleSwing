#pragma once
#include "func.h"
#include "pendulum.h"


class DoublePendulum {
    Pendulum p1;
    Pendulum p2;
    float mass1, mass2;

    float calculate_accel_p1(float theta1, float omega1, float theta2, float omega2) const;
    float calculate_accel_p2(float theta1, float omega1, float theta2, float omega2) const;

    void integrate_RK4(float dt);

public:
    DoublePendulum(float theta1, float l1, float m1, float theta2, float l2, float m2);
    DoublePendulum(float theta1, float l1, float m1, float theta2, float l2, float m2, float fric);
    void update();
    void draw(sf::RenderWindow &win, int centerX, int centerY, bool paused);
    friend class Simulation;
};

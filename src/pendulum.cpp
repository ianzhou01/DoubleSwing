#include "pendulum.h"
#include "simulation.h"

Pendulum::Pendulum(float theta, float len) :
        len(len),
        theta(theta),
        arm({ARM_W, len * PX_METER_RATIO}),
        weight(WEIGHT_RAD),
        omega(0),
        accel(0),
        friction(1.f)
{
    arm.setOrigin(ARM_W / 2, 0);
    weight.setOrigin(WEIGHT_RAD, WEIGHT_RAD - len * PX_METER_RATIO);
    arm.setPosition(CENTER_X, CENTER_Y);
    weight.setPosition(CENTER_X, CENTER_Y);
    arm.setFillColor(ARM_CLR);
    weight.setFillColor(sf::Color::White);
    weight.setOutlineThickness(2);
    weight.setOutlineColor(sf::Color::Black);
}

Pendulum::Pendulum(float theta, float len, float fric) :
        len(len),
        theta(theta),
        arm({ARM_W, len * PX_METER_RATIO}),
        weight(WEIGHT_RAD),
        omega(0),
        accel(0),
        friction(fric)
{
    arm.setOrigin(ARM_W / 2, 0);
    weight.setOrigin(WEIGHT_RAD, WEIGHT_RAD - len * PX_METER_RATIO);
    arm.setPosition(CENTER_X, CENTER_Y);
    weight.setPosition(CENTER_X, CENTER_Y);
    arm.setFillColor(ARM_CLR);
    weight.setFillColor(sf::Color::White);
    weight.setOutlineThickness(2);
    weight.setOutlineColor(sf::Color::Black);
}

//Pendulum::Pendulum(float theta, float len, int weightSize) :
//        len(len),
//        theta(theta),
//        arm({ARM_W, len * PX_METER_RATIO}),
//        weight(static_cast<float>(weightSize)),
//        omega(0),
//        accel(0),
//        friction(1.f)
//{
//    arm.setOrigin(ARM_W / 2, 0);
//    weight.setOrigin(WEIGHT_RAD, WEIGHT_RAD - len * PX_METER_RATIO);
//    arm.setPosition(CENTER_X, CENTER_Y);
//    weight.setPosition(CENTER_X, CENTER_Y);
//    arm.setFillColor(ARM_CLR);
//    weight.setFillColor(sf::Color::White);
//    weight.setOutlineThickness(2);
//    weight.setOutlineColor(sf::Color::Black);
//}
//
//Pendulum::Pendulum(float theta, float len, float fric, int weightSize) :
//        len(len),
//        theta(theta),
//        arm({ARM_W, len * PX_METER_RATIO}),
//        weight(static_cast<float>(weightSize)),
//        omega(0),
//        accel(0),
//        friction(fric)
//{
//    arm.setOrigin(ARM_W / 2, 0);
//    weight.setOrigin(WEIGHT_RAD, WEIGHT_RAD - len * PX_METER_RATIO);
//    arm.setPosition(CENTER_X, CENTER_Y);
//    weight.setPosition(CENTER_X, CENTER_Y);
//    arm.setFillColor(ARM_CLR);
//    weight.setFillColor(sf::Color::White);
//    weight.setOutlineThickness(2);
//    weight.setOutlineColor(sf::Color::Black);
//}

void Pendulum::integrateEuler() { // Simple Euler integration
    omega += accel / (float)FPS;
    theta += omega / (float)FPS;
}

float Pendulum::calculate_single_accel(float theta) {
    return -g * sin(theta) / len; // for single pendulums only
}

void Pendulum::integrate_RK4(float dt) { // Runge-Kutta (RK4)
    // k1
    float k1_theta = omega; // rate of change of theta (angular velocity)
    float k1_acc = calculate_single_accel(theta);

    // k2
    float theta2 = theta + 0.5 * k1_theta * dt;
    float k2_theta = omega + 0.5 * k1_acc * dt; // speed2
    float k2_acc = calculate_single_accel(theta2);

    // k3
    float theta3 = theta + 0.5 * k2_theta * dt;
    float k3_theta = omega + 0.5 * k2_acc * dt; // speed3
    float k3_acc = calculate_single_accel(theta3);

    // k4
    float theta4 = theta + k3_theta * dt;
    float k4_theta = omega + k3_acc * dt; // speed4
    float k4_acc = calculate_single_accel(theta4);

    // final integration step
    theta += (dt / 6.0) * (k1_theta + 2 * k2_theta + 2 * k3_theta + k4_theta);
    omega += (dt / 6.0) * (k1_acc + 2 * k2_acc + 2 * k3_acc + k4_acc);
}


void Pendulum::update() {
    float dt = 1.0f / FPS; // Time step
    integrate_RK4(dt);
    theta = Utility::normalize_angle(theta);

    omega *= friction;
}

void Pendulum::setCoords(sf::RenderWindow &win, int centerX, int centerY) {
    arm.setPosition(centerX, centerY);
    weight.setPosition(centerX, centerY);
    // Ensure that weight and arm rotate together (to appear attached)
    arm.setRotation(Utility::rad_to_deg(theta));
    weight.setRotation(Utility::rad_to_deg(theta));
}

void Pendulum::draw(sf::RenderWindow &win, int centerX, int centerY, bool paused) {
    win.draw(arm);
    win.draw(weight);
}

void Pendulum::setColor(sf::Color clr) {
    weight.setFillColor(clr);
}

float Pendulum::getLen() const {
    return len;
}

float Pendulum::getTheta() const {
    return theta;
}

float Pendulum::getSpeed() const {
    return omega;
}


void Pendulum::setTheta(float angle) {
    theta = angle;
}

void Pendulum::setSpeed(float v) {
    omega = v;
}


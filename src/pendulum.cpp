#include "pendulum.h"
#include "simulation.h"

Pendulum::Pendulum(float theta, float len) :
        len(len),
        thetaRaw(theta),
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
        thetaRaw(theta),
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

//Pendulum::Pendulum(float thetaRaw, float len, int weightSize) :
//        len(len),
//        thetaRaw(thetaRaw),
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
//Pendulum::Pendulum(float thetaRaw, float len, float fric, int weightSize) :
//        len(len),
//        thetaRaw(thetaRaw),
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
    thetaRaw += omega / (float)FPS;
}

float Pendulum::calculate_single_accel(float theta) {
    return -g * sin(theta) / len; // for single pendulums only
}

void Pendulum::integrate_RK4(float dt) { // Runge-Kutta (RK4)
    // k1
    float k1_theta = omega; // rate of change of thetaRaw (angular velocity)
    float k1_acc = calculate_single_accel(thetaRaw);

    // k2
    float theta2 = thetaRaw + 0.5 * k1_theta * dt;
    float k2_theta = omega + 0.5 * k1_acc * dt; // speed2
    float k2_acc = calculate_single_accel(theta2);

    // k3
    float theta3 = thetaRaw + 0.5 * k2_theta * dt;
    float k3_theta = omega + 0.5 * k2_acc * dt; // speed3
    float k3_acc = calculate_single_accel(theta3);

    // k4
    float theta4 = thetaRaw + k3_theta * dt;
    float k4_theta = omega + k3_acc * dt; // speed4
    float k4_acc = calculate_single_accel(theta4);

    // final integration step
    thetaRaw += (dt / 6.0) * (k1_theta + 2 * k2_theta + 2 * k3_theta + k4_theta);
    thetaNormal = -1.0f * thetaRaw;
    Utility::normalize_angle(thetaNormal);

    omega += (dt / 6.0) * (k1_acc + 2 * k2_acc + 2 * k3_acc + k4_acc);

    preventOverflow();
}

void Pendulum::preventOverflow() {
    if (std::abs(thetaRaw) > RAD_LIMIT) {
        thetaRaw = fmod(thetaRaw, 2 * PI);
    }
}


void Pendulum::update() {
    float dt = 1.0f / FPS; // Time step
    integrate_RK4(dt);
    omega *= friction;
}

void Pendulum::setCoords(sf::RenderWindow &win, int centerX, int centerY) {
    arm.setPosition(centerX, centerY);
    weight.setPosition(centerX, centerY);
    // Ensure that weight and arm rotate together (to appear attached)
    arm.setRotation(Utility::rad_to_deg(thetaRaw));
    weight.setRotation(Utility::rad_to_deg(thetaRaw));
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

float Pendulum::getThetaRaw() const {
    return thetaRaw;
}

float Pendulum::getThetaNorm() const {
    return thetaNormal;
}

float Pendulum::getSpeed() const {
    return omega;
}

void Pendulum::setTheta(float angle) {
    thetaRaw = angle;
    thetaNormal = -1.0f * angle; // Reflect counterclockwise positivity convention
}

void Pendulum::setSpeed(float v) {
    omega = v;
}

void Pendulum::setAccel(float acc) {
    accel = acc;
}

float Pendulum::getAccel() const {
    return accel;
}


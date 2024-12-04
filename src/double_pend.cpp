#include "double_pend.h"
#include "simulation.h"

DoublePendulum::DoublePendulum(float theta1, float l1, float mass1, float theta2, float l2, float mass2) :
        p1(theta1, l1),
        p2(theta2, l2),
        mass1(mass1),
        mass2(mass2), damp(2.f), stiffness(1.0f) {}

DoublePendulum::DoublePendulum(float theta1, float l1, float mass1, float theta2, float l2, float mass2,
                               float fric) :
        p1(theta1, l1, fric),
        p2(theta2, l2, fric),
        mass1(mass1),
        mass2(mass2), damp(2.f), stiffness(1.0f) {}

float DoublePendulum::calculate_accel_p1(float theta1, float omega1, float theta2, float omega2) const {
    // Euler-Lagrange numerical computation
    return static_cast<float>((-g * (2 * mass1 + mass2) * sin(theta1)
                               - mass2 * g * sin(theta1 - 2 * theta2)
                               - 2 * sin(theta1 - theta2) * mass2 *
                                 (omega2 * omega2 * p2.len + omega1 * omega1 * p1.len * cos(theta1 - theta2)))
                              / (p1.len * (2 * mass1 + mass2 * (1 - cos(2 * theta1 - 2 * theta2)))));
}


// **FIX** changing axis of gravity
float DoublePendulum::calculate_accel_p2(float theta1, float omega1, float theta2, float omega2) const {
    return static_cast<float>((2 * sin(theta1 - theta2)
                               * (omega1 * omega1 * p1.len * (mass1 + mass2)
                                  + g * (mass1 + mass2) * cos(theta1) + omega2 * omega2 * p2.len * mass2 * cos(theta1 - theta2)))
                              / (p2.len * (2 * mass1 + mass2 * (1 - cos(2 * theta1 - 2 * theta2)))));
}

float DoublePendulum::calc_accel_p2_dragging(float theta2, float omega2) const {
    float p1_accel_x = p1.accel * PX_METER_RATIO * p1.len * cos(p1.thetaRaw);
    float p1_accel_y = p1.accel * PX_METER_RATIO * p1.len * sin(p1.thetaRaw);
    return -((cos(theta2) * p1_accel_x + sin(theta2) * p1_accel_y) / p2.len) // motion of anchor point
           - (g / p2.len) * sin(theta2) // vertical gravity factor
           - (damp * omega2 / (mass2 * p2.len * p2.len)); // damping force
}

void DoublePendulum::integrateBoth_Euler(float dt) {
    p1.omega += calculate_accel_p1(p1.thetaRaw, p1.omega, p2.thetaRaw, p2.omega) * dt;
    p2.omega += calculate_accel_p2(p1.thetaRaw, p1.omega, p2.thetaRaw, p2.omega) * dt;
    p1.thetaRaw += p1.omega * dt;
    p2.thetaRaw += p2.omega * dt;
}

void DoublePendulum::integrateP2_Euler(float dt) {
    p2.omega += calc_accel_p2_dragging(p2.thetaRaw, p2.omega) * dt;
    p2.thetaRaw += p2.omega * dt;
}

void DoublePendulum::integrateBoth_RK4(float dt) {
    // k1
    float k1_omega1 = p1.omega;
    float k1_omega2 = p2.omega;
    float k1_accel1 = calculate_accel_p1(p1.thetaRaw, p1.omega, p2.thetaRaw, p2.omega);
    float k1_accel2 = calculate_accel_p2(p1.thetaRaw, p1.omega, p2.thetaRaw, p2.omega);

    // k2
    float k2_theta1 = p1.thetaRaw + 0.5 * k1_omega1 * dt;
    float k2_theta2 = p2.thetaRaw + 0.5 * k1_omega2 * dt;
    float k2_omega1 = p1.omega + 0.5 * k1_accel1 * dt;
    float k2_omega2 = p2.omega + 0.5 * k1_accel2 * dt;
    float k2_accel1 = calculate_accel_p1(k2_theta1, k2_omega1, k2_theta2, k2_omega2);
    float k2_accel2 = calculate_accel_p2(k2_theta1, k2_omega1, k2_theta2, k2_omega2);

    // k3
    float k3_theta1 = p1.thetaRaw + 0.5 * k2_omega1 * dt;
    float k3_theta2 = p2.thetaRaw + 0.5 * k2_omega2 * dt;
    float k3_omega1 = p1.omega + 0.5 * k2_accel1 * dt;
    float k3_omega2 = p2.omega + 0.5 * k2_accel2 * dt;
    float k3_accel1 = calculate_accel_p1(k3_theta1, k3_omega1, k3_theta2, k3_omega2);
    float k3_accel2 = calculate_accel_p2(k3_theta1, k3_omega1, k3_theta2, k3_omega2);

    // k4
    float k4_theta1 = p1.thetaRaw + k3_omega1 * dt;
    float k4_theta2 = p2.thetaRaw + k3_omega2 * dt;
    float k4_omega1 = p1.omega + k3_accel1 * dt;
    float k4_omega2 = p2.omega + k3_accel2 * dt;
    float k4_accel1 = calculate_accel_p1(k4_theta1, k4_omega1, k4_theta2, k4_omega2);
    float k4_accel2 = calculate_accel_p2(k4_theta1, k4_omega1, k4_theta2, k4_omega2);

    // final integration step
    p1.thetaRaw += (dt / 6.0) * (k1_omega1 + 2 * k2_omega1 + 2 * k3_omega1 + k4_omega1);
    p1.thetaNormal = -1.0f * p1.thetaRaw;
    Utility::normalize_angle(p1.thetaNormal);
    p1.omega += (dt / 6.0) * (k1_accel1 + 2 * k2_accel1 + 2 * k3_accel1 + k4_accel1);

    p2.thetaRaw += (dt / 6.0) * (k1_omega2 + 2 * k2_omega2 + 2 * k3_omega2 + k4_omega2);
    p2.thetaNormal = -1.0f * p2.thetaRaw;
    Utility::normalize_angle(p2.thetaNormal);
    p2.omega += (dt / 6.0) * (k1_accel2 + 2 * k2_accel2 + 2 * k3_accel2 + k4_accel2);

    preventOverflow();
}

void DoublePendulum::integrateP2_RK4(float dt) {
    // k1
    float k1_omega2 = p2.omega;
    float k1_accel2 = calc_accel_p2_dragging(p2.thetaRaw, p2.omega);

    // k2
    float k2_theta2 = p2.thetaRaw + 0.5 * k1_omega2 * dt;
    float k2_omega2 = p2.omega + 0.5 * k1_accel2 * dt;
    float k2_accel2 = calc_accel_p2_dragging(k2_theta2, k2_omega2);

    // k3
    float k3_theta2 = p2.thetaRaw + 0.5 * k2_omega2 * dt;
    float k3_omega2 = p2.omega + 0.5 * k2_accel2 * dt;
    float k3_accel2 = calc_accel_p2_dragging( k3_theta2, k3_omega2);

    // k4
    float k4_theta2 = p2.thetaRaw + k3_omega2 * dt;
    float k4_omega2 = p2.omega + k3_accel2 * dt;
    float k4_accel2 = calc_accel_p2_dragging( k4_theta2, k4_omega2);

    p2.thetaRaw += (dt / 6.0) * (k1_omega2 + 2 * k2_omega2 + 2 * k3_omega2 + k4_omega2);
    p2.thetaNormal = -1.0f * p2.thetaRaw;
    Utility::normalize_angle(p2.thetaNormal);

    p2.omega += (dt / 6.0) * (k1_accel2 + 2 * k2_accel2 + 2 * k3_accel2 + k4_accel2);
    Utility::clamp_speed(p2.omega);
    p2.preventOverflow();
}

void DoublePendulum::updateBoth() {
    float dt = 1.0f / FPS;

    integrateBoth_RK4(dt);
    //integrateBoth_Euler(dt);

    p1.omega *= p1.friction;
    p2.omega *= p2.friction;
}

void DoublePendulum::updateP2() {
    float dt = 1.0f / FPS;

    integrateP2_RK4(dt);
    //integrateP2_Euler(dt);
    //p2.omega *= p2.friction; // Damping factor already included in forced accel calculation**
}

void DoublePendulum::preventOverflow() {
    p1.preventOverflow();
    p2.preventOverflow();
}

void DoublePendulum::draw(sf::RenderWindow &win, int centerX, int centerY, bool paused) {
    p1.setCoords(win, centerX, centerY);

    float p1X = centerX - PX_METER_RATIO * p1.len * sin(p1.thetaRaw);
    float p1Y = centerY + PX_METER_RATIO * p1.len * cos(p1.thetaRaw);

    // Set position of p2 relative to p1's arm endpoint
    p2.setCoords(win, p1X, p1Y);

    win.draw(p1.arm);
    win.draw(p2.arm);
    win.draw(p1.weight);
    win.draw(p2.weight);
}

void DoublePendulum::update_p1_forced_accel(const sf::Vector2f &mousePos, float dt) {
    // Get current position in pixels
    float pX = CENTER_X - PX_METER_RATIO * p1.len * sin(p1.thetaRaw);
    float pY = CENTER_Y - PX_METER_RATIO * p1.len * cos(p1.thetaRaw);

    // Calculate velocity components from omega
    float v_x = p1.omega * PX_METER_RATIO * p1.len * cos(p1.thetaRaw);
    float v_y = p1.omega * PX_METER_RATIO * p1.len * sin(p1.thetaRaw);

    // Calculate accelerations
    float a_x = -v_x * damp + stiffness * (mousePos.x - pX);
    float a_y = -v_y * damp + stiffness * (mousePos.y - pY);

    // Calculate angular acceleration
    float r = p1.len * PX_METER_RATIO;
    p1.accel = (a_x * cos(p1.thetaRaw) + a_y * sin(p1.thetaRaw)) / r;
}



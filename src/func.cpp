#include "func.h"
#include "simulation.h"


float Utility::rad_to_deg(float rad) {
    return rad * 180 / PI;
}

void Utility::normalize_angle(float &angle) {
    while (angle < 0)
        angle += 2.0f * PI; // Add 2π until it's non-negative
    while (angle >= 2 * PI)
        angle -= 2.0f * PI; // Subtract 2π until it's less than 2π
}

void Utility::clamp_speed(float &v) {
    if (std::abs(v) > MAX_SPEED) {
        v = (v < 0) ? -MAX_SPEED : MAX_SPEED;
    }
}


#include "func.h"
#include "simulation.h"


float Utility::rad_to_deg(float rad) {
    return rad * 180 / PI;
}

void Utility::normalize_angle(float &angle) { // Keep within [-pi, pi]
    while (angle < -1 * PI)
        angle += 2.0f * PI;
    while (angle > PI)
        angle -= 2.0f * PI;
}

void Utility::clamp_speed(float &v) {
    if (std::abs(v) > MAX_SPEED) {
        v = (v < 0) ? -MAX_SPEED : MAX_SPEED;
    }
}


#include "func.h"
#include "simulation.h"


float Utility::rad_to_deg(float rad) {
    return rad * 180 / PI;
}

float Utility::deg_to_rad(float deg) {
    return deg * PI / 180;
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

float Utility::distance(float x1, float y1, float x2, float y2) {
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}


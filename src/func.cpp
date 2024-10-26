#include "func.h"

float Utility::rad_to_deg(float rad) {
    return rad * 180 / PI;
}

float Utility::normalize_angle(float angle) {
    while (angle < 0)
        angle += 2 * PI; // Add 2π until it's non-negative
    while (angle >= 2 * PI)
        angle -= 2 * PI; // Subtract 2π until it's less than 2π
    return angle;
}


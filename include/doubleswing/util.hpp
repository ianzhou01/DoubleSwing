#pragma once
#include <cmath>

namespace ds {

inline constexpr double PI = 3.14159265358979323846;
inline constexpr long double PI_L = 3.14159265358979323846264338327950288420L;

inline double rad_to_deg(double rad) { return rad * 180.0 / PI; }
inline double deg_to_rad(double deg) { return deg * PI / 180.0; }

// Keep within [-pi, pi]
inline double normalize_angle(double a) {
    while (a < -PI) a += 2.0 * PI;
    while (a >  PI) a -= 2.0 * PI;
    return a;
}

inline double clamp_abs(double v, double vmax) {
    if (std::abs(v) > vmax) return (v < 0) ? -vmax : vmax;
    return v;
}

inline double distance(double x1, double y1, double x2, double y2) {
    const double dx = x2 - x1, dy = y2 - y1;
    return std::sqrt(dx*dx + dy*dy);
}

} // namespace ds

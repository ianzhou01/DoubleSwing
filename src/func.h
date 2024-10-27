#pragma once
const float PI = 3.14159265359f;

class Utility {
public:
    static float rad_to_deg(float rad);
    static float deg_to_rad(float deg);
    static void normalize_angle(float &rad);
    static void clamp_speed(float &v);
};


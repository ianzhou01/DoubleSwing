#pragma once
const float PI = 3.14159265359f;

class Utility {
public:
    static float rad_to_deg(float rad);
    static void normalize_angle(float &rad);
    static void clamp_speed(float &v);
};


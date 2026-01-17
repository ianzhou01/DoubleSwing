#include <doubleswing/drag.hpp>
#include <doubleswing/util.hpp>
#include <algorithm>

namespace ds {

    double unwrap_delta(double new_theta, double prev_theta) {
        // return delta in [-pi, pi]
        double d = std::fmod(new_theta - prev_theta, 2.0 * PI);
        if (d >  PI) d -= 2.0 * PI;
        if (d < -PI) d += 2.0 * PI;
        return d;
    }

    double DragFilter::update(double theta, double dt, double alpha, double omega_max) {
        if (dt <= 0.0) return omega;

        theta = normalize_angle(theta);

        if (!has_prev) {
            prev_theta = theta;
            omega = 0.0;
            has_prev = true;
            return omega;
        }

        // raw omega from unwrapped delta / dt
        const double dtheta = unwrap_delta(theta, prev_theta);
        double omega_raw = dtheta / dt;

        // clamp raw omega (this is the single biggest anti-explosion control)
        omega_raw = clamp_abs(omega_raw, omega_max);

        // low-pass filter: omega = (1-alpha)*omega + alpha*omega_raw
        alpha = std::clamp(alpha, 0.0, 1.0);
        omega = (1.0 - alpha) * omega + alpha * omega_raw;

        prev_theta = theta;
        return omega;
    }

} // namespace ds

#pragma once

namespace ds {

    // delta in [-pi, pi]
    double unwrap_delta(double new_theta, double prev_theta);

    struct DragFilter {
        double omega = 0.0;
        double prev_theta = 0.0;
        bool has_prev = false;

        // alpha in [0,1], clamp in rad/s.
        // returns filtered omega (also stored in omega)
        double update(double theta, double dt, double alpha, double omega_max);

        void reset(double theta) {
            prev_theta = theta;
            omega = 0.0;
            has_prev = true;
        }
    };

} // namespace ds

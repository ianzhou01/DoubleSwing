#pragma once

namespace ds {

    struct Params {
        double l1, l2;
        double m1, m2;
        double g = 9.80665;
        double damping = 0.0; // simple viscous damping on omegas (rad/s^2 term)
    };

    struct State {
        double th1, w1;
        double th2, w2;
    };

    class Engine {
    public:
        Params p;
        State  s;

        explicit Engine(const Params& p, const State& s0);

        void step(double dt);

        [[nodiscard]] double energy() const;
        void bob_positions(double& x1, double& y1, double& x2, double& y2) const;

        // When th1, w1, a1 are externally imposed (dragging bob1),
        // integrate only (th2, w2) with moving-pivot dynamics.
        void step_drag_p1(double dt, double th1, double w1, double a1);

    private:
        // returns angular accelerations (th1dd, th2dd) for given state
        void accel(const State& st, double& a1, double& a2) const;

        // RK4 step on (th1,w1,th2,w2)
        void rk4(State& st, double dt) const;

        // theta2'' when pivot (bob1) has cartesian acceleration (xdd,ydd)
        double accel_theta2_moving_pivot(double th2, double w2, double xdd, double ydd) const;

        // RK4 integrate only (th2, w2) given pivot accel
        void rk4_th2(double& th2, double& w2, double dt, double xdd, double ydd) const;

        // bob1 cartesian acceleration from (th1,w1,a1)
        static void bob1_cart_accel(const Params& p, double th1, double w1, double a1,
                                    double& xdd, double& ydd);
    };

} // namespace ds

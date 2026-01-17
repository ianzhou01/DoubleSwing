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

    private:
        // returns angular accelerations (th1dd, th2dd) for given state
        void accel(const State& st, double& a1, double& a2) const;

        // RK4 step on (th1,w1,th2,w2)
        void rk4(State& st, double dt) const;
    };

} // namespace ds

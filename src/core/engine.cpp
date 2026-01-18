#include <doubleswing/engine.hpp>
#include <doubleswing/util.hpp>
#include <cmath>
#include <algorithm>

namespace ds {

Engine::Engine(const Params& params, const State& s0) : p(params), s(s0) {
    // keep angles sane at construction
    s.th1 = normalize_angle(s.th1);
    s.th2 = normalize_angle(s.th2);
}

void Engine::accel(const State& st, double& a1, double& a2) const {
    // Angles measured from vertical
    const double th1 = st.th1;
    const double th2 = st.th2;
    const double w1  = st.w1;
    const double w2  = st.w2;

    const double m1 = p.m1;
    const double m2 = p.m2;
    const double l1 = p.l1;
    const double l2 = p.l2;
    const double g  = p.g;

    const double dth = th1 - th2;

    const double denom = (2.0*m1 + m2 - m2*std::cos(2.0*dth));

    // Protect against denom ~ 0 spikes
    const double eps = 1e-12;
    const double denom1 = std::max(std::abs(denom), eps) * (denom < 0 ? -1.0 : 1.0);

    // th1''
    a1 = (-g*(2.0*m1 + m2)*std::sin(th1)
          - m2*g*std::sin(th1 - 2.0*th2)
          - 2.0*std::sin(dth)*m2*(w2*w2*l2 + w1*w1*l1*std::cos(dth)))
         / (l1 * denom1);

    // th2''
    a2 = ( 2.0*std::sin(dth) *
          ( w1*w1*l1*(m1 + m2)
            + g*(m1 + m2)*std::cos(th1)
            + w2*w2*l2*m2*std::cos(dth) ) )
         / (l2 * denom1);

    // Simple viscous damping on angular velocities
    // TODO: Not yet “physical” damping derived from Lagrangian dissipation.
    if (p.damping != 0.0) {
        a1 -= p.damping * w1;
        a2 -= p.damping * w2;
    }
}

void Engine::bob1_cart_accel(const ds::Params& p,
                                    double th1, double w1, double a1,
                                    double& xdd, double& ydd) {
    // x = l1*sin(th1), y = l1*cos(th1)
    // xdd = l1*(a1*cos(th1) - w1^2*sin(th1))
    // ydd = l1*(-a1*sin(th1) - w1^2*cos(th1))
    const double s = std::sin(th1);
    const double c = std::cos(th1);
    xdd = p.l1 * (a1 * c - (w1*w1) * s);
    ydd = p.l1 * (-a1 * s - (w1*w1) * c);
}

double Engine::accel_theta2_moving_pivot(double th2, double w2, double xdd, double ydd) const {
    // Equation for pendulum angle from vertical when pivot accelerates:
    // th2'' = -(g/l2) * sin(th2) - (xdd*cos(th2) + ydd*sin(th2))/l2 - damping*w2
    const double s2 = std::sin(th2);
    const double c2 = std::cos(th2);

    double a2 = -(p.g / p.l2) * s2
                - (xdd * c2 + ydd * s2) / p.l2;

    if (p.damping != 0.0) a2 -= p.damping * w2;
    return a2;
}

void Engine::rk4_th2(double& th2, double& w2, double dt, double xdd, double ydd) const {
    auto deriv = [&](double th, double w) {
        // d/dt th = w, d/dt w = a(th,w)
        const double a = accel_theta2_moving_pivot(th, w, xdd, ydd);
        return std::pair<double,double>(w, a);
    };

    const auto [k1_th, k1_w] = deriv(th2, w2);

    const auto [k2_th, k2_w] = deriv(th2 + 0.5*dt*k1_th, w2 + 0.5*dt*k1_w);

    const auto [k3_th, k3_w] = deriv(th2 + 0.5*dt*k2_th, w2 + 0.5*dt*k2_w);

    const auto [k4_th, k4_w] = deriv(th2 + dt*k3_th, w2 + dt*k3_w);

    th2 += (dt/6.0) * (k1_th + 2*k2_th + 2*k3_th + k4_th);
    w2  += (dt/6.0) * (k1_w  + 2*k2_w  + 2*k3_w  + k4_w);

    th2 = normalize_angle(th2);
}

void Engine::step_drag_p1(double dt, double th1, double w1, double a1) {
    dt = std::clamp(dt, 0.0, 1.0/15.0);

    // impose th1 dynamics from mouse
    s.th1 = normalize_angle(th1);
    s.w1  = w1;

    // compute bob1 cartesian acceleration due to imposed motion
    double xdd, ydd;
    bob1_cart_accel(p, s.th1, s.w1, a1, xdd, ydd);

    // integrate only th2,w2 under moving-pivot equation
    double th2 = s.th2;
    double w2  = s.w2;
    rk4_th2(th2, w2, dt, xdd, ydd);

    s.th2 = th2;
    s.w2  = w2;
}

void Engine::rk4(State& st, double dt) const {
    // State vector: [th1, w1, th2, w2]
    auto deriv = [&](const State& x) -> State {
        double a1, a2;
        accel(x, a1, a2);
        return State{ x.w1, a1, x.w2, a2 };
    };

    const State k1 = deriv(st);

    State tmp = st;
    tmp.th1 += 0.5 * dt * k1.th1;
    tmp.w1  += 0.5 * dt * k1.w1;
    tmp.th2 += 0.5 * dt * k1.th2;
    tmp.w2  += 0.5 * dt * k1.w2;
    const State k2 = deriv(tmp);

    tmp = st;
    tmp.th1 += 0.5 * dt * k2.th1;
    tmp.w1  += 0.5 * dt * k2.w1;
    tmp.th2 += 0.5 * dt * k2.th2;
    tmp.w2  += 0.5 * dt * k2.w2;
    const State k3 = deriv(tmp);

    tmp = st;
    tmp.th1 += dt * k3.th1;
    tmp.w1  += dt * k3.w1;
    tmp.th2 += dt * k3.th2;
    tmp.w2  += dt * k3.w2;
    const State k4 = deriv(tmp);

    st.th1 += (dt / 6.0) * (k1.th1 + 2.0*k2.th1 + 2.0*k3.th1 + k4.th1);
    st.w1  += (dt / 6.0) * (k1.w1  + 2.0*k2.w1  + 2.0*k3.w1  + k4.w1);
    st.th2 += (dt / 6.0) * (k1.th2 + 2.0*k2.th2 + 2.0*k3.th2 + k4.th2);
    st.w2  += (dt / 6.0) * (k1.w2  + 2.0*k2.w2  + 2.0*k3.w2  + k4.w2);

    // Bound angles to avoid precision blowups over long runs
    st.th1 = normalize_angle(st.th1);
    st.th2 = normalize_angle(st.th2);
}

void Engine::step(double dt) {
    // cap dt so tab-outs don't explode
    dt = std::clamp(dt, 0.0, 1.0/15.0);

    rk4(s, dt);
}

void Engine::bob_positions(double& x1, double& y1, double& x2, double& y2) const {
    // coords in meters, pivot at (0,0), +y downward for convenience in screen space
    // Note: for +y up, flip signs in the renderer
    x1 =  p.l1 * std::sin(s.th1);
    y1 =  p.l1 * std::cos(s.th1);

    x2 = x1 + p.l2 * std::sin(s.th2);
    y2 = y1 + p.l2 * std::cos(s.th2);
}

double Engine::energy() const {
    // Note: Kinetic + potential (pivot at 0 potential). +y downward means potential uses -cos terms
    // Assumes point masses at the bobs
    const double th1 = s.th1, th2 = s.th2;
    const double w1 = s.w1, w2 = s.w2;

    const double m1 = p.m1, m2 = p.m2;
    const double l1 = p.l1, l2 = p.l2;
    const double g  = p.g;

    const double v1_sq = (l1*w1)*(l1*w1);
    const double v2_sq = v1_sq + (l2*w2)*(l2*w2) + 2.0*l1*l2*w1*w2*std::cos(th1 - th2);

    const double T = 0.5*m1*v1_sq + 0.5*m2*v2_sq;

    // With y down: height is -y, so potential increases when y decreases.
    // y1 = l1*cos(th1), y2 = y1 + l2*cos(th2)
    const double U = (m1 + m2)*g*l1*(1 - cos(th1)) + m2*g*l2*(1 - cos(th2));

    return T + U;
}

} // namespace ds

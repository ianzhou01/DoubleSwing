#include <doubleswing/engine.hpp>

extern "C" {

    struct EngineHandle { ds::Engine eng; };

    EngineHandle* ds_create(double l1, double l2, double m1, double m2, double g, double damping,
                            double th1, double w1, double th2, double w2) {
        ds::Params p{l1, l2, m1, m2, g, damping};
        ds::State  s{th1, w1, th2, w2};
        return new EngineHandle{ ds::Engine(p, s) };
    }

    void ds_destroy(EngineHandle* h) { delete h; }
    void ds_step(EngineHandle* h, double dt) { h->eng.step(dt); }
    void ds_step_drag_p1(EngineHandle* h, double dt, double th1, double w1, double a1) {
        h->eng.step_drag_p1(dt, th1, w1, a1); // Default a1 = 0.0
    }

    // Cache positions in static storage (for single-engine demo)
    static double g_x1, g_y1, g_x2, g_y2;

    void ds_update_positions(EngineHandle* h) {
        h->eng.bob_positions(g_x1, g_y1, g_x2, g_y2);
    }
    double ds_x1() { return g_x1; }
    double ds_y1() { return g_y1; }
    double ds_x2() { return g_x2; }
    double ds_y2() { return g_y2; }

    // Interactive API
    void ds_set_th1(EngineHandle* h, double th1) { h->eng.s.th1 = th1; }
    void ds_set_th2(EngineHandle* h, double th2) { h->eng.s.th2 = th2; }
    void ds_set_w1 (EngineHandle* h, double w1)  { h->eng.s.w1  = w1; }
    void ds_set_w2 (EngineHandle* h, double w2)  { h->eng.s.w2  = w2; }

    void ds_reset(EngineHandle* h, double th1, double th2) {
        h->eng.s.th1 = th1; h->eng.s.w1 = 0.0;
        h->eng.s.th2 = th2; h->eng.s.w2 = 0.0;
    }

    // Param setters
    void ds_set_l1(EngineHandle* h, double l1) { h->eng.p.l1 = l1; }
    void ds_set_l2(EngineHandle* h, double l2) { h->eng.p.l2 = l2; }
    void ds_set_m1(EngineHandle* h, double m1) { h->eng.p.m1 = m1; }
    void ds_set_m2(EngineHandle* h, double m2) { h->eng.p.m2 = m2; }
    void ds_set_g(EngineHandle* h, double g) { h->eng.p.g = g; }
    void ds_set_damping(EngineHandle* h, double d) { h->eng.p.damping = d; }

    // State getters (for HUD/readout)
    double ds_th1(EngineHandle* h) { return h->eng.s.th1; }
    double ds_th2(EngineHandle* h) { return h->eng.s.th2; }
    double ds_w1 (EngineHandle* h) { return h->eng.s.w1; }
    double ds_w2 (EngineHandle* h) { return h->eng.s.w2; }

    double ds_ke(EngineHandle* h) { return h->eng.energy_breakdown().ke; }
    double ds_pe(EngineHandle* h) { return h->eng.energy_breakdown().pe; }
    double ds_energy(EngineHandle* h) { auto [ke, pe] = h->eng.energy_breakdown(); return ke + pe;  }
}

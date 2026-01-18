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

    // Cache positions in static storage (for single-engine demo)
    static double g_x1, g_y1, g_x2, g_y2;

    void ds_update_positions(EngineHandle* h) {
        h->eng.bob_positions(g_x1, g_y1, g_x2, g_y2);
    }
    double ds_x1() { return g_x1; }
    double ds_y1() { return g_y1; }
    double ds_x2() { return g_x2; }
    double ds_y2() { return g_y2; }

}

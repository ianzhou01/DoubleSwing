#include <doubleswing/engine.hpp>

extern "C" {

    struct EngineHandle { ds::Engine eng; };

    EngineHandle* ds_create(double l1, double l2, double m1, double m2, double g, double damping,
                            double th1, double w1, double th2, double w2) {
        ds::Params p{l1, l2, m1, m2, g, damping};
        ds::State  s{th1, w1, th2, w2};
        return new EngineHandle{ ds::Engine(p, s) };
    }

    void ds_destroy(const EngineHandle* h) { delete h; }

    void ds_step(EngineHandle* h, double dt) { h->eng.step(dt); }

    void ds_set_state(EngineHandle* h, double th1, double w1, double th2, double w2) {
        h->eng.s = ds::State{th1, w1, th2, w2};
    }

    void ds_get_state(const EngineHandle* h, double* th1, double* w1, double* th2, double* w2) {
        *th1 = h->eng.s.th1; *w1 = h->eng.s.w1;
        *th2 = h->eng.s.th2; *w2 = h->eng.s.w2;
    }

    void ds_bob_positions(const EngineHandle* h, double* x1, double* y1, double* x2, double* y2) {
        h->eng.bob_positions(*x1, *y1, *x2, *y2);
    }

}

#include <SFML/Graphics.hpp>

#include <doubleswing/engine.hpp>
#include <doubleswing/drag.hpp>

#include "sfml_app.hpp"

int main() {
    // Window setup
    constexpr unsigned WIDTH  = 800;
    constexpr unsigned HEIGHT = 800;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "DoubleSwing");
    window.setVerticalSyncEnabled(true);

    // Engine setup
    ds::Params params;
    params.l1 = 4.0;
    params.l2 = 4.0;
    params.m1 = 2.0;
    params.m2 = 2.0;
    params.g  = 9.80665;
    params.damping = 0.02; // start small; tune later

    ds::State s0;
    s0.th1 = 0.0;
    s0.w1  = 0.0;
    s0.th2 = 0.0;
    s0.w2  = 0.0;

    ds::Engine engine(params, s0);

    // Drag filters (frontend -> controls how we estimate omega from mouse)
    ds::DragFilter drag1;
    ds::DragFilter drag2;

    // App wrapper (SFML frontend)
    SfmlApp app(window, engine, drag1, drag2);
    return app.run();
}

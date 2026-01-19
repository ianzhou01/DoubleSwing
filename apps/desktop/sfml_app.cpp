#include "sfml_app.hpp"

#include <doubleswing/util.hpp>
#include <algorithm>
#include <sstream>

SfmlApp::SfmlApp(sf::RenderWindow& win, ds::Engine& eng, ds::DragFilter& d1, ds::DragFilter& d2)
    : window(win), engine(eng), drag1(d1), drag2(d2)
{
    // If window size differs, set pivot to center.
    const auto size = window.getSize();
    pivot_px = sf::Vector2f(size.x * 0.5f, size.y * 0.5f);

    // Basic HUD
    if (font.loadFromFile("assets/fonts/DejaVuSans.ttf")) {
        hud.setFont(font);
        hud.setCharacterSize(16);
        hud.setFillColor(sf::Color::Black);
        hud.setPosition(10.f, 10.f);
    }
}

int SfmlApp::run() {
    window.setVerticalSyncEnabled(true);

    sf::Clock frameClock;

    while (window.isOpen()) {
        handle_events();

        const double dt = std::clamp(frameClock.restart().asSeconds(), 1.0f/2400.0f, 1.0f/15.0f);

        update(dt);
        render();
    }
    return 0;
}

void SfmlApp::handle_events() {
    sf::Event e{};
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            window.close();
            return;
        }

        if (e.type == sf::Event::Resized) {
            // Keep pivot centered on resize
            pivot_px = sf::Vector2f(e.size.width * 0.5f, e.size.height * 0.5f);
        }

        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mouse((float)e.mouseButton.x, (float)e.mouseButton.y);

            // hit test bobs (in pixels)
            const float grabRadius1 = 30.f; // tune
            const float grabRadius2 = 25.f;

            const sf::Vector2f b1 = bob1_px();
            const sf::Vector2f b2 = bob2_px();

            const double d1 = ds::distance(mouse.x, mouse.y, b1.x, b1.y);
            const double d2 = ds::distance(mouse.x, mouse.y, b2.x, b2.y);

            // Prefer grabbing bob2 if you're closer to it
            if (d2 <= grabRadius2) {
                dragging2 = true;
                dragging1 = false;
                // initialize drag filter at current angle to avoid a big first delta
                drag2.reset(engine.s.th2);
            } else if (d1 <= grabRadius1) {
                dragging1 = true;
                dragging2 = false;
                drag1.reset(engine.s.th1);
            }
        }

        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
            dragging1 = false;
            dragging2 = false;
            // no zero velocities for "fling" effect.
        }

        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::R) {
                // quick reset
                engine.s.th1 = 0.0; engine.s.w1 = 0.0;
                engine.s.th2 = 0.0; engine.s.w2 = 0.0;
                drag1.reset(engine.s.th1);
                drag2.reset(engine.s.th2);
            }
        }
    }
}

void SfmlApp::update(double dt) {
    // TODO: Option B parameters (tune)
    const double alpha = 0.15;           // low-pass strength
    const double omega_max = 10.0;       // rad/s clamp (old MAX_SPEED=15-ish)
    // Note: if explosions happen, lower omega_max first.

    const sf::Vector2f mouse = (sf::Vector2f)sf::Mouse::getPosition(window);

    // If dragging, constrain the dragged angle to the mouse and set omega via filter.
    // The rest of the state evolves via engine.step(), but we keep the constrained angle in sync.
    if (dragging1) {
        const double th1 = theta_from_mouse_about_pivot(mouse);

        // Filtered omega from mouse motion (Option B)
        double w1 = drag1.update(th1, dt, alpha, omega_max);
        if (std::abs(w1) < 0.05) w1 = 0.0; // deadband

        double a1 = 0.0;
        // Note: estimating tangential angular acceleration with mouse input is too noisy for accuracy.
        engine.step_drag_p1(dt, th1, w1, a1);
    } else if (dragging2) {
        const double th = theta_from_mouse_about_bob1(mouse);
        engine.s.th2 = th;
        engine.s.w2  = drag2.update(th, dt, alpha, omega_max);

        const double saved_th2 = engine.s.th2;
        engine.step(dt);
        engine.s.th2 = saved_th2;
    } else {
        engine.step(dt);
    }

    // HUD
    if (hud.getFont()) {
        std::ostringstream ss;
        ss.setf(std::ios::fixed); ss.precision(2);
        ss << "th1=" << ds::rad_to_deg(engine.s.th1) << " deg"
           << "  w1=" << engine.s.w1 << " rad/s\n"
           << "th2=" << ds::rad_to_deg(engine.s.th2) << " deg"
           << "  w2=" << engine.s.w2 << " rad/s\n"
           << "E=";

        if (!dragging1 && !dragging2) {
            auto [ke, pe] = engine.energy_breakdown();
            ss << ke + pe;
        }
        else ss << " (driven)";
        ss << (dragging1 ? "  [dragging P1]" : (dragging2 ? "  [dragging P2]" : ""));
        ss << "\nR: reset";
        hud.setString(ss.str());
    }
}

void SfmlApp::render() {
    window.clear(sf::Color::White);

    // Get bob positions in meters, convert to pixels
    double x1m, y1m, x2m, y2m;
    engine.bob_positions(x1m, y1m, x2m, y2m);

    const sf::Vector2f p0 = pivot_px;
    const sf::Vector2f p1 = sf::Vector2f(p0.x + (float)(x1m * px_per_meter),
                                         p0.y + (float)(y1m * px_per_meter));
    const sf::Vector2f p2 = sf::Vector2f(p0.x + (float)(x2m * px_per_meter),
                                         p0.y + (float)(y2m * px_per_meter));

    // draw rods
    auto drawLine = [&](sf::Vector2f a, sf::Vector2f b) {
        sf::Vertex line[] = { sf::Vertex(a, sf::Color(50,50,50)),
                              sf::Vertex(b, sf::Color(50,50,50)) };
        window.draw(line, 2, sf::Lines);
    };
    drawLine(p0, p1);
    drawLine(p1, p2);

    // draw pivot and bobs
    auto drawCircle = [&](sf::Vector2f c, float r, sf::Color fill) {
        sf::CircleShape s(r);
        s.setOrigin(r, r);
        s.setPosition(c);
        s.setFillColor(fill);
        s.setOutlineThickness(2.f);
        s.setOutlineColor(sf::Color::Black);
        window.draw(s);
    };

    drawCircle(p0, 8.f, sf::Color(60, 200, 80));
    drawCircle(p1, 14.f, sf::Color::White);
    drawCircle(p2, 12.f, sf::Color::White);

    if (hud.getFont())
        window.draw(hud);

    window.display();
}

sf::Vector2f SfmlApp::bob1_px() const {
    double x1m, y1m, x2m, y2m;
    engine.bob_positions(x1m, y1m, x2m, y2m);
    return sf::Vector2f(pivot_px.x + (float)(x1m * px_per_meter),
                        pivot_px.y + (float)(y1m * px_per_meter));
}

sf::Vector2f SfmlApp::bob2_px() const {
    double x1m, y1m, x2m, y2m;
    engine.bob_positions(x1m, y1m, x2m, y2m);
    return sf::Vector2f(pivot_px.x + (float)(x2m * px_per_meter),
                        pivot_px.y + (float)(y2m * px_per_meter));
}

// Angle conventions: engine uses theta from vertical with +y down.
// With bob_positions: x = L*sin(theta), y = L*cos(theta).
// Given a mouse vector (dx,dy) in screen coords (+y down), theta = atan2(dx,dy).
double SfmlApp::theta_from_mouse_about_pivot(const sf::Vector2f& mouse) const {
    const float dx = mouse.x - pivot_px.x;
    const float dy = mouse.y - pivot_px.y;
    return ds::normalize_angle(std::atan2((double)dx, (double)dy));
}

double SfmlApp::theta_from_mouse_about_bob1(const sf::Vector2f& mouse) const {
    const sf::Vector2f b1 = bob1_px();
    const float dx = mouse.x - b1.x;
    const float dy = mouse.y - b1.y;
    return ds::normalize_angle(std::atan2((double)dx, (double)dy));
}

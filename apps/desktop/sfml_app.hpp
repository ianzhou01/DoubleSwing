#pragma once
#include <SFML/Graphics.hpp>
#include <doubleswing/engine.hpp>
#include <doubleswing/drag.hpp>

class SfmlApp {
public:
    SfmlApp(sf::RenderWindow& win, ds::Engine& eng, ds::DragFilter& d1, ds::DragFilter& d2);
    int run();

private:
    sf::RenderWindow& window;
    ds::Engine& engine;
    ds::DragFilter& drag1;
    ds::DragFilter& drag2;

    bool dragging1 = false;
    bool dragging2 = false;

    // rendering/config
    float px_per_meter = 40.0f;
    sf::Vector2f pivot_px {400.0f, 400.0f};

    // UI
    sf::Font font;
    sf::Text hud;

    void handle_events();
    void update(double dt);
    void render();

    // helpers
    sf::Vector2f bob1_px() const;
    sf::Vector2f bob2_px() const;
    double theta_from_mouse_about_pivot(const sf::Vector2f& mouse) const;
    double theta_from_mouse_about_bob1(const sf::Vector2f& mouse) const;
};

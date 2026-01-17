#pragma once
#include <SFML/Graphics.hpp>
#include <doubleswing/engine.hpp>
#include <doubleswing/drag.hpp>

class SfmlApp {
public:
    SfmlApp(sf::RenderWindow& win, ds::Engine& eng, ds::DragFilter& d1, ds::DragFilter& d2);

    // return code
    int run();

private:
    sf::RenderWindow& window;
    ds::Engine& engine;
    ds::DragFilter& drag1;
    ds::DragFilter& drag2;
};

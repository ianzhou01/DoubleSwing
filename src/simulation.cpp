#include "simulation.h"


Simulation::Simulation() : win(sf::VideoMode(WIDTH, HEIGHT), "DoubleSwing"),
                           doublePendulum(0.f, 4.f, 2.f, 0.f, 4.f, 2.f, 0.9995f), // Example initial random state
                           pivot(PIVOT_RAD), isDraggingP1(false), isDraggingP2(false),
                           showDragText(true), paused(false) {

    // Set pivot orb style
    if (!dragFont.loadFromFile("resources/RubikDirt-Regular.ttf")) {
        // Handle error loading dragFont
        throw std::runtime_error("Error loading fonts!");
    }
    if (!menuFont.loadFromFile("resources/Galindo-Regular.ttf")) {
        throw std::runtime_error("Error loading fonts!");
    }

    dragText.setFont(dragFont);
    dragText.setString("Drag Me!");
    dragText.setCharacterSize(50); // Set size of the text
    dragText.setFillColor(sf::Color::Black);
    dragText.setPosition(CENTER_X - dragText.getGlobalBounds().width / 2, CENTER_Y - 200);

    menuBar.setSize(sf::Vector2f(WIDTH, 40));
    menuBar.setFillColor(sf::Color(200, 200, 200));
    menuBar.setPosition(0, 0);

    // Initialize the "Reset" button
    resetButton.setSize(sf::Vector2f(80, 30));
    resetButton.setFillColor(sf::Color(180, 180, 180));
    resetButton.setPosition(WIDTH - 100, 5);

    // Initialize the reset text
    resetText.setFont(menuFont);  // Assuming `font` is loaded somewhere
    resetText.setString("Reset");
    resetText.setCharacterSize(20);
    resetText.setFillColor(sf::Color::Black);
    resetText.setPosition(WIDTH - 90, 8);

    pivot.setFillColor(sf::Color::Green);
    pivot.setOrigin(PIVOT_RAD, PIVOT_RAD);
    pivot.setPosition(CENTER_X, CENTER_Y);


    // Set frame rate limit
    win.setFramerateLimit(FPS);
//    // Set random initial speeds (opposite directions)
//    std::random_device rd;
//    std::mt19937 mt(rd());
//    std::uniform_int_distribution<int> dist(-1500000, 1500000);
//    float speed1 = static_cast<float>(dist(mt) / 100000.f), speed2 = static_cast<float>(dist(mt) / 100000.f);
//    doublePendulum.p1.setSpeed(speed1);
//    doublePendulum.p2.setSpeed((speed1 <= 0 != speed2 <= 0) ? speed2 : -1.f * speed2);
//
//    std::cout << "Speed 1: " << doublePendulum.p1.getSpeed() << "\nSpeed 2: " << doublePendulum.p2.getSpeed() << std::endl;
}

void Simulation::operator()() {
    dt = 1.0f / FPS;
    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                win.close();

            checkMenuClick(event);

            handle_click(event);

            handle_mouse_release(event);

            // Handle dragging
            handle_mouse_move(event);
        }

        // Update pendulum (skip updating during drag)
        if (!isDraggingP1 && !isDraggingP2) {
            doublePendulum.updateBoth();
        }
        else if (!isDraggingP2 && isDraggingP1)
            doublePendulum.updateP2();

        win.clear(sf::Color::White);

        draw_all();

        win.display();
    }
}

void Simulation::draw_all() {
    win.draw(pivot);  // Draw pivot point
    doublePendulum.draw(win, CENTER_X, CENTER_Y, false);  // Draw pendulum

    if (showDragText) {
        win.draw(dragText);
    }
    // Future option to hide menu (?)
    win.draw(menuBar);
    win.draw(resetButton);
    win.draw(resetText);
}

float Simulation::distance(float x1, float y1, float x2, float y2) {
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void Simulation::handle_click(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(win);

        // Position of p1 weight
        float p1X = (float)(CENTER_X + PX_METER_RATIO * doublePendulum.p1.getLen() * sin(doublePendulum.p1.getTheta()));
        float p1Y = (float)(CENTER_Y - PX_METER_RATIO * doublePendulum.p1.getLen() * cos(doublePendulum.p1.getTheta()));

        // Position of p2 weight
        float p2X = (float)(p1X + PX_METER_RATIO * doublePendulum.p2.getLen() * sin(doublePendulum.p2.getTheta()));
        float p2Y = (float)(p1Y - PX_METER_RATIO * doublePendulum.p2.getLen() * cos(doublePendulum.p2.getTheta()));

        float distP1 = distance(mousePos.x, mousePos.y, p1X, HEIGHT - p1Y);
        if (distP1 <= doublePendulum.p1.weight.getRadius() * 8) {
            isDraggingP1 = true; // Set dragging state without tracking mouse position here
            showDragText = false;
            return;
        }

        float distP2 = distance(mousePos.x, mousePos.y, p2X, HEIGHT - p2Y);
        if (distP2 <= doublePendulum.p2.weight.getRadius() * 4) {
            isDraggingP2 = true; // Set dragging state for second pendulum
            showDragText = false;
        }
    }
}


void Simulation::handle_mouse_move(sf::Event &event) {
    if (isDraggingP1 || isDraggingP2) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(win);
        sf::Vector2f currentMousePos((float)mousePos.x, (float)mousePos.y);
        float currentTime = clock.getElapsedTime().asSeconds();

        if (isDraggingP1) {
            // Update theta1 based on mouse position
            float dx = currentMousePos.x - CENTER_X;
            float dy = currentMousePos.y - CENTER_Y;
            float newTheta = atan2f(dy, dx) - PI / 2;

            float newSpeed = (newTheta - doublePendulum.p1.getTheta()) / dt;

            Utility::clamp_speed(newSpeed);    // To not go crazy
            doublePendulum.p1.setSpeed(newSpeed); // Update speed for release physics
            doublePendulum.p1.setTheta(newTheta); // Update position while dragging
        }

        if (isDraggingP2) {
            // Calculate angle theta2 based on mouse position
            float p1X = (float)(CENTER_X + PX_METER_RATIO * doublePendulum.p1.getLen() * sin(doublePendulum.p1.getTheta()));
            float p1Y = (float)(CENTER_Y - PX_METER_RATIO * doublePendulum.p1.getLen() * cos(doublePendulum.p1.getTheta()));

            float dx = currentMousePos.x - p1X;
            float dy = currentMousePos.y - (HEIGHT - p1Y);

            float newTheta = atan2f(dy, dx) - PI / 2;
            float newSpeed = (newTheta - doublePendulum.p2.getTheta()) / dt;

            Utility::clamp_speed(newSpeed);
            doublePendulum.p2.setSpeed(newSpeed);
            doublePendulum.p2.setTheta(newTheta);
        }
    }
}

void Simulation::handle_mouse_release(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        // Reset dragging state and mouse history
        isDraggingP1 = false;
        isDraggingP2 = false;
    }
}

void Simulation::resetAll() {
    showDragText = true;
    doublePendulum.p1.setTheta(0.0f);
    doublePendulum.p1.setSpeed(0.0f);
    doublePendulum.p2.setTheta(0.0f);
    doublePendulum.p2.setSpeed(0.0f);
}

void Simulation::checkMenuClick(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(win);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        if (resetButton.getGlobalBounds().contains(mousePosF)) {
            resetAll();
        }
    }
}


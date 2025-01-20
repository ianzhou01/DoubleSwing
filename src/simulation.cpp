#include "simulation.h"


Simulation::Simulation() : win(sf::VideoMode(WIDTH, HEIGHT), "DoubleSwing"),
                           doublePendulum(0.f, 4.f, 2.f, 0.f, 4.f, 2.f, 0.9995f), // Example initial random state
                           pivot(PIVOT_RAD), isDraggingP1(false), isDraggingP2(false),
                           showDragText(true), paused(false), secondClock(false) {

    // Set pivot orb style
    if (!dragFont.loadFromFile("resources/RubikDirt-Regular.ttf")) {
        // Handle error loading dragFont
        throw std::runtime_error("Error loading fonts!");
    }
    if (!menuFont.loadFromFile("resources/Galindo-Regular.ttf")) {
        throw std::runtime_error("Error loading fonts!");
    }
    if (!greekFont.loadFromFile("resources/DejaVuSans.ttf")) {
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

    debugVel1.setFont(greekFont);
    debugVel1.setString("V1: 0.00");
    debugVel1.setCharacterSize(20);
    debugVel1.setFillColor(sf::Color::Black);
    debugVel1.setPosition(50, 8);

    debugVel2.setFont(greekFont);
    debugVel2.setString("V2: 0.00");
    debugVel2.setCharacterSize(20);
    debugVel2.setFillColor(sf::Color::Black);
    debugVel2.setPosition(180, 8);

    pos1.setFont(greekFont);
    pos1.setString("A1: 0.00\xB0");
    pos1.setCharacterSize(20);
    pos1.setFillColor(sf::Color::Black);
    pos1.setPosition(310, 8);

    pos2.setFont(greekFont);
    pos2.setString("A2: 0.00\xB0");
    pos2.setCharacterSize(20);
    pos2.setFillColor(sf::Color::Black);
    pos2.setPosition(440, 8);

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
        }
        handle_mouse_move();

        // Update pendulum (skip updating during drag)
        if (!isDraggingP1 && !isDraggingP2) {
            doublePendulum.updateBoth();
        }
        else if (!isDraggingP2 && isDraggingP1)
            doublePendulum.updateP2(); // TODO: implement functionality

        win.clear(sf::Color::White);

        draw_all();

        win.display();
        secondClock = !secondClock;
    }
}

void Simulation::updatePhysicsText() {
    std::ostringstream ss;
    ss.precision(2);
    ss << std::fixed;

    if (!isDraggingP1) {
        ss.str("");  // Clear stream
        ss << "V1: " << ((doublePendulum.p1.getSpeed() != 0) ? -1.0f * doublePendulum.p1.getSpeed()
                                                             : doublePendulum.p1.getSpeed()); // To reflect counterclockwise convention
        debugVel1.setString(ss.str());
    }
    else
        debugVel1.setString("V1: Held");

    if (!isDraggingP2) {
        ss.str("");
        ss << "V2: " << ((doublePendulum.p2.getSpeed() != 0) ? -1.0f * doublePendulum.p2.getSpeed()
                                                             : doublePendulum.p2.getSpeed());
        debugVel2.setString(ss.str());
    }
    else
        debugVel2.setString("V2: Held");

    ss.str("");
    ss << "P1: " << static_cast<int>(Utility::rad_to_deg(doublePendulum.p1.getThetaNorm())) << "\xB0";
    pos1.setString(ss.str());

    ss.str("");
    ss << "P2: " << static_cast<int>(Utility::rad_to_deg(doublePendulum.p2.getThetaNorm())) << "\xB0";
    pos2.setString(ss.str());
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

    if (secondClock)
        updatePhysicsText(); // Update every second frame
    win.draw(debugVel1);
    win.draw(debugVel2);
    win.draw(pos1);
    win.draw(pos2);
}



void Simulation::handle_click(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(win);

        // Position of p1 weight
        float p1X = (float)(CENTER_X - PX_METER_RATIO * doublePendulum.p1.getLen() * sin(doublePendulum.p1.getThetaRaw()));
        float p1Y = (float)(CENTER_Y - PX_METER_RATIO * doublePendulum.p1.getLen() * cos(doublePendulum.p1.getThetaRaw()));

        // Position of p2 weight
        float p2X = (float)(p1X - PX_METER_RATIO * doublePendulum.p2.getLen() * sin(doublePendulum.p2.getThetaRaw()));
        float p2Y = (float)(p1Y - PX_METER_RATIO * doublePendulum.p2.getLen() * cos(doublePendulum.p2.getThetaRaw()));

        float distP1 = Utility::distance(mousePos.x, mousePos.y, p1X, HEIGHT - p1Y);
        if (distP1 <= doublePendulum.p1.weight.getRadius() * 8) {
            isDraggingP1 = true; // Set dragging state without tracking mouse position here
            showDragText = false;
            return;
        }

        float distP2 = Utility::distance(mousePos.x, mousePos.y, p2X, HEIGHT - p2Y);
        if (distP2 <= doublePendulum.p2.weight.getRadius() * 4) {
            isDraggingP2 = true; // Set dragging state for second pendulum
            showDragText = false;
        }
    }
}


void Simulation::handle_mouse_move() {
    if (isDraggingP1 || isDraggingP2) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(win);
        sf::Vector2f currentMousePos((float)mousePos.x, (float)mousePos.y);
        float currentTime = clock.getElapsedTime().asSeconds();

        if (isDraggingP1) {
            // Update theta1 based on mouse position
            float dx = currentMousePos.x - CENTER_X;
            float dy = currentMousePos.y - CENTER_Y;
            float newTheta = atan2f(dy, dx) - PI / 2;

            Utility::normalize_angle(newTheta);

            // Calculate raw theta change (accounting for jumps)
            float deltaTheta = fmod(newTheta - doublePendulum.p1.getThetaRaw(), 2 * PI);
            if (deltaTheta > PI)
                deltaTheta -= 2 * PI;
            else if (deltaTheta < -PI)
                deltaTheta += 2 * PI;
            float newSpeed = deltaTheta / dt;

            Utility::clamp_speed(newSpeed);    // To not go crazy

            // Find acceleration
            doublePendulum.p1.setSpeed(newSpeed); // Update speed for release physics
            doublePendulum.p1.setTheta(newTheta); // Update position while dragging
            //doublePendulum.update_p1_forced_accel(currentMousePos, dt);
        }

        if (isDraggingP2) {
            // Calculate angle theta2 based on mouse position
            float p1X = (float)(CENTER_X - PX_METER_RATIO * doublePendulum.p1.getLen() * sin(
                    doublePendulum.p1.getThetaRaw()));
            float p1Y = (float)(CENTER_Y - PX_METER_RATIO * doublePendulum.p1.getLen() * cos(
                    doublePendulum.p1.getThetaRaw()));

            float dx = currentMousePos.x - p1X;
            float dy = currentMousePos.y - (HEIGHT - p1Y);

            float newTheta = atan2f(dy, dx) - PI / 2;
            Utility::normalize_angle(newTheta);

            float deltaTheta = fmod(newTheta - doublePendulum.p2.getThetaRaw() + PI, 2 * PI) - PI;
            float newSpeed = deltaTheta / dt;

            Utility::clamp_speed(newSpeed);

            doublePendulum.p2.setSpeed(newSpeed);
            doublePendulum.p2.setTheta(newTheta);
        }
    }
}

void Simulation::handle_mouse_release(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        // Reset dragging state
        if (isDraggingP1) // TODO: possible fix
            doublePendulum.p1.setAccel(0.0f); // reset to default values when not tracked
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

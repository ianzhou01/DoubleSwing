#include "simulation.h"

Simulation::Simulation() : win(sf::VideoMode(WIDTH, HEIGHT), "WEE"),
                           doublePendulum(0.f, 4.f, 2.f, 0.f, 4.f, 2.f, 0.9995f), // Example initial random state
                           pivot(PIVOT_RAD), isDraggingP1(false), isDraggingP2(false) {

    // Set pivot orb style
    pivot.setFillColor(sf::Color::Green);
    pivot.setOrigin(PIVOT_RAD, PIVOT_RAD);
    pivot.setPosition(CENTER_X, CENTER_Y);

    // Set frame rate limit
    win.setFramerateLimit(FPS);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(-1500000, 1500000);

    float speed1 = static_cast<float>(dist(mt) / 100000.f), speed2 = static_cast<float>(dist(mt) / 100000.f);

    // Set initial speeds (arbitary)
    doublePendulum.p1.setSpeed(speed1);
    doublePendulum.p2.setSpeed((speed1 <= 0 != speed2 <= 0) ? speed2 : -1.f * speed2);
    std::cout << "Speed 1: " << doublePendulum.p1.getSpeed() << "\nSpeed 2: " << doublePendulum.p2.getSpeed() << std::endl;
}

void Simulation::operator()() {
    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                win.close();

            handle_click(event);

            handle_mouse_release(event);

            // Handle dragging
            handle_mouse_move(event);
        }
//        dt = clock.restart().asSeconds();

        // Update pendulum (skip updating during drag)
        if (!isDraggingP1 && !isDraggingP2) {
            doublePendulum.update();
        }
//        else if (!isDraggingP2 && isDraggingP1)
//            doublePendulum.p2.update();

        win.clear(sf::Color::White);

        draw_all();

        win.display();
    }
}

void Simulation::draw_all() {
    win.draw(pivot);  // Draw pivot point
    doublePendulum.draw(win, CENTER_X, CENTER_Y, false);  // Draw pendulum
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
            return;
        }

        float distP2 = distance(mousePos.x, mousePos.y, p2X, HEIGHT - p2Y);
        if (distP2 <= doublePendulum.p2.weight.getRadius() * 4) {
            isDraggingP2 = true; // Set dragging state for second pendulum
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
            doublePendulum.p1.setTheta(atan2f(dy, dx) - PI / 2);

//            mouseHistoryP1.push_back({currentMousePos, currentTime});
//            if (mouseHistoryP1.size() > MAX_MOUSE_HISTORY)
//                mouseHistoryP1.erase(mouseHistoryP1.begin());  // Keep history small

        }

        if (isDraggingP2) {
            // Calculate angle theta2 based on mouse position
            float p1X = (float)(CENTER_X + PX_METER_RATIO * doublePendulum.p1.getLen() * sin(doublePendulum.p1.getTheta()));
            float p1Y = (float)(CENTER_Y - PX_METER_RATIO * doublePendulum.p1.getLen() * cos(doublePendulum.p1.getTheta()));

            float dx = currentMousePos.x - p1X;
            float dy = currentMousePos.y - (HEIGHT - p1Y);
            doublePendulum.p2.setTheta(atan2f(dy, dx) - PI / 2);

//            mouseHistoryP2.push_back({currentMousePos, currentTime});
//            if (mouseHistoryP2.size() > MAX_MOUSE_HISTORY)
//                mouseHistoryP2.erase(mouseHistoryP2.begin());
        }
    }
}

void Simulation::handle_mouse_release(sf::Event &event) {
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isDraggingP1) {
            doublePendulum.p1.setSpeed(0.f);
        }
        else if (isDraggingP2) {
            doublePendulum.p2.setSpeed(0.f);
        }
//        float velocity = 0.f;
//
//        // Calculate velocity for p1
//        if (isDraggingP1 && mouseHistoryP1.size() > 1) {
//            // Use the difference between the first and last stored positions
//            sf::Vector2f startPos = mouseHistoryP1.front().first;
//            sf::Vector2f endPos = mouseHistoryP1.back().first;
//            float startTime = mouseHistoryP1.front().second;
//            float endTime = mouseHistoryP1.back().second;
//
//            float dx = (endPos.x - startPos.x) / PX_METER_RATIO;
//            float dy = (endPos.y - startPos.y) / PX_METER_RATIO;
//            float distanceMoved = sqrt(dx * dx + dy * dy);
//            float timeElapsed = endTime - startTime;
//
//            if (timeElapsed > 0) {
//                velocity = distanceMoved / (timeElapsed * doublePendulum.p1.getLen());
//                if (velocity > 15.f) velocity = 15.f;
//                std::cout << "Velocity 1: " << velocity << std::endl;
//                doublePendulum.p1.setSpeed(velocity);
//            }
//        }
//
//        // Calculate velocity for p2
//        if (isDraggingP2 && mouseHistoryP2.size() > 1) {
//            sf::Vector2f startPos = mouseHistoryP2.front().first;
//            sf::Vector2f endPos = mouseHistoryP2.back().first;
//            float startTime = mouseHistoryP2.front().second;
//            float endTime = mouseHistoryP2.back().second;
//
//            float dx = (endPos.x - startPos.x) / PX_METER_RATIO;
//            float dy = (endPos.y - startPos.y) / PX_METER_RATIO;
//            float distanceMoved = sqrt(dx * dx + dy * dy);
//            float timeElapsed = endTime - startTime;
//
//            if (timeElapsed > 0) {
//                velocity = distanceMoved / (timeElapsed * doublePendulum.p2.getLen());
//                if (velocity > 15.f) velocity = 15.f;
//                std::cout << "Velocity 2: " << velocity << std::endl;
//                doublePendulum.p2.setSpeed(velocity);
//            }
//        }
        // Reset dragging state and mouse history
        isDraggingP1 = false;
        isDraggingP2 = false;
//        mouseHistoryP1.clear();
//        mouseHistoryP2.clear();
    }
}


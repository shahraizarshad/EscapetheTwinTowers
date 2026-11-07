// ESCAPE THE TWIN TOWERS - intro screen 
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>

const float TOWER_X = 800, PLANE_Y = 330;       // left wall of the tower, height of the plane
const int N = 120;                              // number of explosion particles
float px[N], py[N], vx[N], vy[N], life[N];      // explosion particles (life 0 = unused)
float sx[40], sy[40], slife[40];                // white smoke puffs behind the plane

float rnd(float a, float b) { return a + (b - a) * (rand() % 1001) / 1000.f; }

// ---------- small drawing helpers ----------
void box(sf::RenderWindow& w, float x, float y, float wd, float ht, sf::Color c) {
    sf::RectangleShape r({wd, ht});  r.setPosition({x, y});  r.setFillColor(c);  w.draw(r);
}
void ball(sf::RenderWindow& w, float x, float y, float rad, sf::Color c) {
    sf::CircleShape s(rad);  s.setOrigin({rad, rad});  s.setPosition({x, y});  s.setFillColor(c);  w.draw(s);
}
void poly(sf::RenderWindow& w, int n, const float* p, sf::Color c) {   // p = x1,y1,x2,y2,...
    sf::ConvexShape s(n);
    for (int i = 0; i < n; i++) s.setPoint(i, {p[2 * i], p[2 * i + 1]});
    s.setFillColor(c);  w.draw(s);
}
void center(sf::RenderWindow& w, sf::Font& font, const char* str, int size, float cx, float cy, sf::Color c) {
    sf::Text t(font, str, size);
    t.setFillColor(c);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x / 2, b.position.y + b.size.y / 2});
    t.setPosition({cx, cy});
    w.draw(t);
}

 // scenery  ..
void cloud(sf::RenderWindow& w, float x, float y, float s) {     // flat-bottom cartoon cloud
    sf::Color white(255, 255, 255);
    box(w, x + 15 * s, y - 30 * s, 90 * s, 30 * s, white);
    ball(w, x + 15 * s, y - 15 * s, 15 * s, white);
    ball(w, x + 105 * s, y - 15 * s, 15 * s, white);
    ball(w, x + 35 * s, y - 32 * s, 22 * s, white);
    ball(w, x + 62 * s, y - 42 * s, 28 * s, white);
    ball(w, x + 88 * s, y - 32 * s, 20 * s, white);
}
void tower(sf::RenderWindow& w) {
    box(w, TOWER_X + 97, 165, 6, 40, sf::Color(70, 75, 85));          // antenna
    box(w, TOWER_X + 40, 205, 120, 25, sf::Color(95, 105, 120));      // top block
    box(w, TOWER_X, 230, 200, 320, sf::Color(112, 122, 138));         // steel body
    box(w, TOWER_X, 230, 8, 320, sf::Color(85, 95, 110));             // darker left edge
    for (int r = 0; r < 6; r++)                                       // window grid: 6 rows x 5 columns
        for (int c = 0; c < 5; c++)
            box(w, TOWER_X + 18 + c * 36, 250 + r * 48, 22, 30,
                (r + c) % 3 == 0 ? sf::Color(255, 235, 150) : sf::Color(170, 215, 240));
}
void plane(sf::RenderWindow& w, float x, float y) {                   // (x, y) = tip of the nose
    sf::Color body(235, 235, 245), red(200, 40, 40), blue(90, 160, 215);
    float fin[]  = {x - 125, y - 12, x - 140, y - 48, x - 112, y - 48, x - 90, y - 12};
    float nose[] = {x - 25, y - 12, x, y, x - 25, y + 12};
    float wing[] = {x - 85, y + 8, x - 50, y + 8, x - 85, y + 45, x - 115, y + 45};
    poly(w, 4, fin, red);
    box(w, x - 130, y - 12, 105, 24, body);                           // fuselage
    poly(w, 3, nose, body);
    box(w, x - 130, y + 2, 105, 5, red);                              // stripe
    for (int i = 0; i < 5; i++) ball(w, x - 115 + i * 16, y - 4, 4, blue);   // windows
    poly(w, 4, wing, sf::Color(175, 185, 205));
    box(w, x - 95, y + 26, 30, 10, sf::Color(70, 80, 100));           // engine
}

// title: 4 words, each does ONE up-and-down sine wave, one after another 
void title(sf::RenderWindow& w, sf::Font& font, float t0) {
    const char* words[4] = {"ESCAPE", "THE", "TWIN", "TOWERS"};
    float width[4], total = 3 * 20;                                   // 20 px gap between words
    for (int i = 0; i < 4; i++) {
        sf::Text s(font, words[i], 44);
        width[i] = s.getLocalBounds().size.x;
        total += width[i];
    }
    float x = (1000 - total) / 2;                                     // centred on the screen
    for (int i = 0; i < 4; i++) {
        float t = t0 - i * 0.25f;                                     // this word's own timer
        float dy = (t > 0 && t < 0.8f) ? 25 * std::sin(6.2832f * t / 0.8f) : 0;
        sf::Text s(font, words[i], 44);
        s.setFillColor(sf::Color(25, 20, 110));                       // deep navy / indigo
        s.setOutlineColor(sf::Color::White);
        s.setOutlineThickness(3);
        s.setPosition({x, 100 - dy});
        w.draw(s);
        x += width[i] + 20;
    }
}

int main() {
    srand((unsigned)time(0));
    sf::RenderWindow window(sf::VideoMode({1000u, 600u}), "Escape The Twin Towers");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.openFromFile("font.ttf") && !font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        std::cout << "Font not found! Run: sudo apt install fonts-dejavu-core\n";
        return 1;
    }
    int state = 0;                                  // 0 = intro, 1 = title + PLAY button, 2 = game screen
    float noseX = -20, boomTime = 0, menuTime = 0, smokeTimer = 0;
    bool crashed = false;
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        bool hover = mouse.x > 370 && mouse.x < 630 && mouse.y > 295 && mouse.y < 365;   // PLAY button area

        //  input 
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                if (key->code == sf::Keyboard::Key::Escape && state == 2) state = 1;
            if (event->getIf<sf::Event::MouseButtonPressed>())
                if (state == 1 && menuTime > 1.3f && hover) state = 2;
        }

        // update
        if (state == 0) {
            for (int i = 0; i < 40; i++)                              // smoke puffs fade out
                if (slife[i] > 0) { slife[i] -= 0.8f * dt; sy[i] -= 12 * dt; }
            for (int i = 0; i < N; i++)                               // explosion: gravity + movement
                if (life[i] > 0) { vy[i] += 300 * dt; px[i] += vx[i] * dt; py[i] += vy[i] * dt; life[i] -= dt; }
            if (!crashed) {
                noseX += 300 * dt;                                    // plane flies left -> right
                smokeTimer += dt;
                if (smokeTimer > 0.03f) {
                    smokeTimer = 0;
                    for (int i = 0; i < 40; i++)
                        if (slife[i] <= 0) { sx[i] = noseX - 100; sy[i] = PLANE_Y + 30; slife[i] = 1; break; }
                }
                if (noseX >= TOWER_X) {                               // boom stopping
                    noseX = TOWER_X;
                    crashed = true;
                    for (int i = 0; i < N; i++) {
                        float angle = rnd(0, 6.2832f), speed = rnd(60, 400);
                        px[i] = TOWER_X;  py[i] = PLANE_Y;
                        vx[i] = std::cos(angle) * speed;  vy[i] = std::sin(angle) * speed;
                        life[i] = rnd(0.8f, 1.7f);
                    }
                }
            } else {
                boomTime += dt;
                if (boomTime >= 1.8f) { state = 1; menuTime = 0; }    // explosion over -> title comes
            }
        } else menuTime += dt;

        // draw 
        window.clear(sf::Color(140, 210, 240));                       // light sky blue
        cloud(window, 60, 130, 1.0f);   cloud(window, 330, 90, 1.2f);   cloud(window, 600, 150, 0.9f);
        cloud(window, 120, 480, 1.1f);  cloud(window, 450, 520, 1.3f);
        tower(window);
        box(window, 0, 550, 1000, 50, sf::Color(90, 170, 90));        // ground
        if (state == 0) {
            for (int i = 0; i < 40; i++)
                if (slife[i] > 0) ball(window, sx[i], sy[i], 5 + (1 - slife[i]) * 14, sf::Color(255, 255, 255, (int)(slife[i] * 200)));
            if (!crashed || boomTime < 0.15f) plane(window, noseX, PLANE_Y);   // plane is destroyed by the blast
            if (crashed && boomTime < 0.12f) ball(window, TOWER_X, PLANE_Y, 120, sf::Color(255, 255, 200, 220));   // flash
            for (int i = 0; i < N; i++) {
                if (life[i] <= 0) continue;
                float a = life[i] > 1 ? 1 : life[i];                  // fades in the last second
                sf::Color c = (i % 3 == 0) ? sf::Color(255, 100, 0) : (i % 3 == 1) ? sf::Color(255, 210, 0) : sf::Color(55, 55, 60);
                c.a = (int)(a * 255);
                ball(window, px[i], py[i], (i % 3 == 2 ? 14 : 8) * (0.4f + 0.6f * a), c);   // orange, gold, gray smoke
            }
            if (crashed && boomTime < 1.2f) {                         // BOOM sign
                float k = boomTime < 0.15f ? boomTime / 0.15f : 1 + 0.05f * std::sin(boomTime * 30);
                sf::Text t(font, "BOOM!", 100);
                t.setFillColor(sf::Color(255, 200, 0));
                t.setOutlineColor(sf::Color(200, 30, 0));
                t.setOutlineThickness(6);
                sf::FloatRect b = t.getLocalBounds();
                t.setOrigin({b.position.x + b.size.x / 2, b.position.y + b.size.y / 2});
                t.setPosition({500, 200});
                t.setScale({k, k});
                window.draw(t);
            }
        } else if (state == 1) {
            title(window, font, menuTime);
            if (menuTime > 1.3f) {                                    // PLAY button appears after the dance
                box(window, 367, 292, 266, 76, sf::Color(25, 25, 112));                                 // border
                box(window, 370, 295, 260, 70, hover ? sf::Color(255, 200, 40) : sf::Color(60, 70, 160));
                center(window, font, "PLAY", 36, 500, 330, hover ? sf::Color(25, 25, 112) : sf::Color::White);
            }
        } else {
            center(window, font, "GAME SCREEN - your game goes here", 32, 400, 250, sf::Color(25, 20, 110));
            center(window, font, "Press ESC to go back", 24, 400, 300, sf::Color(25, 20, 110));
        }
        window.display();
    }
    return 0;
}

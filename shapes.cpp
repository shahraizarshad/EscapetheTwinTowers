#include <SFML/Graphics.hpp>
using namespace std;

const float TOWER_X = 800.f;        // the tower starts at x = 800

void box(sf::RenderWindow& w, float x, float y,     // draws a rectangle
    float wd, float ht, sf::Color c) {

    sf::RectangleShape r({ wd, ht });
    r.setPosition({ x, y });
    r.setFillColor(c);
    w.draw(r);
}

void ball(sf::RenderWindow& w, float x, float y,    // draws a circle
    float rad, sf::Color c) {

    sf::CircleShape s(rad);
    s.setOrigin({ rad, rad });      // x, y is now the center
    s.setPosition({ x, y });
    s.setFillColor(c);
    w.draw(s);
}

// draws a shape with n corners. p isa list of x,y numbers
void poly(sf::RenderWindow& w, int n,
    const float* p, sf::Color c) {

    sf::ConvexShape s(n);
    for (int i = 0; i < n; i++) {
        s.setPoint(i, { p[2 * i], p[2 * i + 1] });  // corner i: x = p[2i], y = p[2i+1]
    }

    s.setFillColor(c);
    w.draw(s);
}

// Cloud = 1 rectangle + 5 circles. s = size
void cloud(sf::RenderWindow& w, float x, float y, float s) {

    sf::Color white(255, 255, 255);

    box(w, x + 15 * s, y - 30 * s, 90 * s, 30 * s, white);

    ball(w, x + 15 * s, y - 15 * s, 15 * s, white);
    ball(w, x + 105 * s, y - 15 * s, 15 * s, white);
    ball(w, x + 35 * s, y - 32 * s, 22 * s, white);
    ball(w, x + 62 * s, y - 42 * s, 28 * s, white);
    ball(w, x + 88 * s, y - 32 * s, 20 * s, white);
}

void tower(sf::RenderWindow& w) {

    box(w, TOWER_X + 97, 165, 6, 40, sf::Color(70, 75, 85));        // antenna
    box(w, TOWER_X + 40, 205, 120, 25, sf::Color(95, 105, 120));    // roof
    box(w, TOWER_X, 230, 200, 320, sf::Color(112, 122, 138));       // body
    box(w, TOWER_X, 230, 8, 320, sf::Color(85, 95, 110));           // dark left edge

    // windows. 6 rows, 5 columns: 30 windows
    // the counter tells how many steps to move from the first window
    // step is the window size plus the gap
    // position of windows = start + counter(c,r)x step
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 5; c++) {
             box(w,TOWER_X + 18 + c * 36,      // moves right for each column
                250 + r * 48,22,30,               // moves down for each row
                (r + c) % 3 == 0
                ? sf::Color(255, 235, 150)  // yellow = true
                : sf::Color(170, 215, 240)); // blue = false
        }
    }
}

// the plane. x, y = the nose tip. All parts are measured back from it
void plane(sf::RenderWindow& w, float x, float y) {

    sf::Color body(235, 235, 245);      // white grey
    sf::Color red(200, 40, 40);
    sf::Color blue(90, 160, 215);

    float fin[] = {                     // tail fin:4 corners (x, y pairs)
        x - 125, y - 12,
        x - 140, y - 48,
        x - 112, y - 48,
        x - 90, y - 12
    };

    float nose[] = {                    // nose:3 corners
        x - 25, y - 12,                 // top corner
        x, y,                           // tip
        x - 25, y + 12                  // bottom corner
    };

    float wing[] = {                    // wing: 4 corners
        x - 85, y + 8,
        x - 50, y + 8,
        x - 85, y + 45,
        x - 115, y + 45
    };

    poly(w, 4, fin, red);               // fin is drawn first, so it stays behind the body

    box(w, x - 130, y - 12, 105, 24, body);     // body
    poly(w, 3, nose, body);                     // nose

    box(w, x - 130, y + 2, 105, 5, red);        // red stripe

    for (int i = 0; i < 5; i++) {               // 5 windows: start + counter x step
        ball(w, x - 115 + i * 16, y - 4, 4, blue);  // start = x - 115, step = 16
    }

    poly(w, 4, wing, sf::Color(175, 185, 205)); // wing (drawn after the body, so it is in front)

    box(w, x - 95, y + 26, 30, 10, sf::Color(70, 80, 100));  // engine
}

int main() {

    sf::RenderWindow window(            // create the game window
        sf::VideoMode({ 1000u, 500u }), // 1000 wide, 500 high ('u' = unsigned)
        "Escape The Twin Towers"        // title of the window
    );

    window.setFramerateLimit(60);       // 60 frames per second

    float planeX = 260.f;               // plane position:nose tip
    float planeY = 300.f;

    while (window.isOpen()) {           // game loop:runs until the window is closed

        while (const auto event = window.pollEvent()) {     // checks events
            if (event->is<sf::Event::Closed>()) {
                window.close();         // close button was clicked
            }
        }

        window.clear(sf::Color(140, 210, 240));     // blue sky

        // 5 clouds (x, y, size)
        cloud(window, 60, 130, 1.0f);
        cloud(window, 330, 90, 1.2f);
        cloud(window, 600, 150, 0.9f);

        cloud(window, 120, 480, 1.1f);
        cloud(window, 450, 520, 1.3f);

        tower(window);

        plane(window, planeX, planeY);  // drawn last,so it is in front of everything

        window.display();               // shows everything we drew in this frame
    }

    return 0;
}
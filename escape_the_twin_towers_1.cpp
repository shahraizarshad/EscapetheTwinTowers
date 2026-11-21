// ESCAPE THE TWIN TOWERS - complete game (intro + menu + settings + game)
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>       // for the crash sound (needs the sfml-audio library linked)
#include <iostream>
#include <fstream>              // file handling for the high score
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <optional>
#include <string>
#include <cstdint>

using namespace std;

const float TOWER_X = 800.f;        // left wall of the tower
const float PLANE_Y = 330.f;        // height of the plane in the intro

// game states
const int INTRO = 0, MENU = 1, PLAYING = 2, GAMEOVER = 3, SETTINGS = 4, PAUSED = 5, WON = 6;

const int N = 120;                                  // explosion particles
float px[N], py[N], vx[N], vy[N], life[N];          // life 0 = unused
float sx[40], sy[40], slife[40];                    // smoke puffs behind the intro plane

// ---------- settings chosen by the player ----------
bool night = false;                                 // false = day theme, true = night theme
int planeChoice = 0;                                // 0..3 index into planeColors
int difficulty = 1;                                 // 0 easy, 1 medium, 2 hard

const sf::Color planeColors[4] = {
    sf::Color(200, 40, 40), sf::Color(40, 160, 70),
    sf::Color(240, 140, 20), sf::Color(130, 60, 190) };
const char* planeNames[4] = { "Red", "Green", "Orange", "Purple" };
const char* diffNames[3] = { "Easy", "Medium", "Hard" };
const float baseInterval[3] = { 1.25f, 1.0f, 0.75f };   // seconds between storm clouds
const float speedMult[3] = { 0.8f, 1.0f, 1.3f };        // cloud speed multiplier

const int WIN_DODGED = 50;                          // dodge this many clouds to escape (level 5 cleared)

// ---------- high score file handling ----------
const char* HS_FILE = "highscore.txt";

// Reads the high score. NEVER fails: a missing file, empty file, letters ("ABCD"),
// negative numbers, mixed text ("12abc") or a huge number all give 0.
int loadHighScore() {
    ifstream f(HS_FILE);
    if (!f.is_open()) return 0;             // file does not exist yet
    string s;
    if (!(f >> s)) return 0;                // file is empty
    if (s.size() > 9) return 0;             // too long: would overflow an int
    for (char c : s)
        if (c < '0' || c > '9') return 0;   // anything that is not a digit is rejected
    return stoi(s);                         // safe: 1 to 9 digits only
}

void saveHighScore(int v) {
    ofstream f(HS_FILE);
    if (f.is_open()) f << v << endl;        // if the file cannot be written, we just skip it
}

// ---------- small helpers ----------
float rnd(float a, float b) {
    return a + (b - a) * (rand() % 1001) / 1000.f;
}

struct Rect { float x, y, w, h; };          // a button area

bool inside(sf::Vector2f m, Rect r) {
    return m.x > r.x && m.x < r.x + r.w && m.y > r.y && m.y < r.y + r.h;
}

sf::Color ink() {                           // text color: dark on day, light on night
    return night ? sf::Color(240, 240, 255) : sf::Color::Black;
}

void box(sf::RenderWindow& w, float x, float y, float wd, float ht, sf::Color c) {
    sf::RectangleShape r({ wd, ht });
    r.setPosition({ x, y });
    r.setFillColor(c);
    w.draw(r);
}

void ball(sf::RenderWindow& w, float x, float y, float rad, sf::Color c) {
    sf::CircleShape s(rad);
    s.setOrigin({ rad, rad });
    s.setPosition({ x, y });
    s.setFillColor(c);
    w.draw(s);
}

void poly(sf::RenderWindow& w, int n, const float* p, sf::Color c) {   // p = x1,y1,x2,y2,...
    sf::ConvexShape s(n);
    for (int i = 0; i < n; i++)
        s.setPoint(i, { p[2 * i], p[2 * i + 1] });
    s.setFillColor(c);
    w.draw(s);
}

void center(sf::RenderWindow& w, sf::Font& font, const char* str, int size,
    float cx, float cy, sf::Color c) {
    sf::Text t(font, str, size);
    t.setFillColor(c);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({ b.position.x + b.size.x / 2, b.position.y + b.size.y / 2 });
    t.setPosition({ cx, cy });
    w.draw(t);
}

void label(sf::RenderWindow& w, sf::Font& font, const string& s, int size,
    float x, float y, sf::Color c) {         // text whose top-left corner is at (x, y)
    sf::Text t(font, s, size);
    t.setFillColor(c);
    t.setPosition({ x, y });
    w.draw(t);
}

// a clickable button; selected = this option is currently chosen
void button(sf::RenderWindow& w, sf::Font& font, const char* txt, Rect r, bool hover, bool selected) {
    sf::Color navy(25, 25, 112);
    box(w, r.x - 3, r.y - 3, r.w + 6, r.h + 6, navy);
    sf::Color fill = selected ? sf::Color(255, 200, 40) : (hover ? sf::Color(90, 105, 200) : sf::Color(60, 70, 160));
    box(w, r.x, r.y, r.w, r.h, fill);
    center(w, font, txt, 26, r.x + r.w / 2, r.y + r.h / 2, selected ? navy : sf::Color::White);
}

// ---------- scenery ----------
void cloud(sf::RenderWindow& w, float x, float y, float s, sf::Color c) {   // flat-bottom cartoon cloud
    box(w, x + 15 * s, y - 30 * s, 90 * s, 30 * s, c);
    ball(w, x + 15 * s, y - 15 * s, 15 * s, c);
    ball(w, x + 105 * s, y - 15 * s, 15 * s, c);
    ball(w, x + 35 * s, y - 32 * s, 22 * s, c);
    ball(w, x + 62 * s, y - 42 * s, 28 * s, c);
    ball(w, x + 88 * s, y - 32 * s, 20 * s, c);
}

void tower(sf::RenderWindow& w) {
    box(w, TOWER_X + 97, 165, 6, 40, sf::Color(70, 75, 85));
    box(w, TOWER_X + 40, 205, 120, 25, sf::Color(95, 105, 120));
    box(w, TOWER_X, 230, 200, 320, sf::Color(112, 122, 138));
    box(w, TOWER_X, 230, 8, 320, sf::Color(85, 95, 110));
    for (int r = 0; r < 6; r++)
        for (int c = 0; c < 5; c++) {
            sf::Color win;
            if (night) win = ((r + c) % 2 == 0) ? sf::Color(255, 235, 150) : sf::Color(50, 60, 85);
            else       win = ((r + c) % 3 == 0) ? sf::Color(255, 235, 150) : sf::Color(170, 215, 240);
            box(w, TOWER_X + 18 + c * 36, 250 + r * 48, 22, 30, win);
        }
}

void plane(sf::RenderWindow& w, float x, float y, sf::Color accent) {   // (x, y) = tip of the nose
    sf::Color body(235, 235, 245), blue(90, 160, 215);
    float fin[] = { x - 125, y - 12, x - 140, y - 48, x - 112, y - 48, x - 90, y - 12 };
    float nose[] = { x - 25, y - 12, x, y, x - 25, y + 12 };
    float wing[] = { x - 85, y + 8, x - 50, y + 8, x - 85, y + 45, x - 115, y + 45 };
    poly(w, 4, fin, accent);
    box(w, x - 130, y - 12, 105, 24, body);
    poly(w, 3, nose, body);
    box(w, x - 130, y + 2, 105, 5, accent);
    for (int i = 0; i < 5; i++) ball(w, x - 115 + i * 16, y - 4, 4, blue);
    poly(w, 4, wing, sf::Color(175, 185, 205));
    box(w, x - 95, y + 26, 30, 10, sf::Color(70, 80, 100));
}

struct Hazard {         // one storm cloud
    float x, y, speed, scale;
};

struct Item {           // one falling pickup
    float x, y, speed;
    int type;           // 0 = coin, 1 = speed booster
};

void stormCloud(sf::RenderWindow& w, float x, float y, float s) {
    sf::Color shade = night ? sf::Color(95, 105, 130) : sf::Color(65, 75, 95);
    ball(w, x - 20 * s, y, 25 * s, shade);
    ball(w, x + 10 * s, y - 10 * s, 32 * s, shade);
    ball(w, x + 40 * s, y, 24 * s, shade);
    ball(w, x + 10 * s, y + 15 * s, 22 * s, shade);
}

void coinShape(sf::RenderWindow& w, float x, float y) {
    ball(w, x, y, 15, sf::Color(220, 160, 0));
    ball(w, x, y, 11, sf::Color(255, 215, 0));
    ball(w, x, y, 5, sf::Color(220, 160, 0));
}

void boosterShape(sf::RenderWindow& w, float x, float y) {   // blue ball with a lightning bolt
    ball(w, x, y, 17, sf::Color(0, 150, 220));
    float a[] = { x + 4, y - 12, x - 6, y + 2, x + 1, y + 2 };
    float b[] = { x - 4, y + 12, x + 6, y - 3, x - 1, y - 3 };
    poly(w, 3, a, sf::Color::White);
    poly(w, 3, b, sf::Color::White);
}

void heart(sf::RenderWindow& w, float x, float y, float s, sf::Color c) {   // (x, y) = centre
    ball(w, x - s * 0.5f, y - s * 0.3f, s * 0.55f, c);
    ball(w, x + s * 0.5f, y - s * 0.3f, s * 0.55f, c);
    float p[] = { x - s * 1.02f, y - s * 0.1f, x + s * 1.02f, y - s * 0.1f, x, y + s * 1.1f };
    poly(w, 3, p, c);
}

bool collision(float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh) {
    if (ax + aw < bx || bx + bw < ax) return false;
    if (ay + ah < by || by + bh < ay) return false;
    return true;
}

// title: 4 words, each does ONE up-and-down sine wave, one after another
void title(sf::RenderWindow& w, sf::Font& font, float t0) {
    const char* words[4] = { "ESCAPE", "THE", "TWIN", "TOWERS" };
    float width[4], total = 3 * 20;
    for (int i = 0; i < 4; i++) {
        sf::Text s(font, words[i], 44);
        width[i] = s.getLocalBounds().size.x;
        total += width[i];
    }
    float x = (1000 - total) / 2;
    for (int i = 0; i < 4; i++) {
        float t = t0 - i * 0.25f;
        float dy = (t > 0 && t < 0.8f) ? 25 * sin(6.2832f * t / 0.8f) : 0;
        sf::Text s(font, words[i], 44);
        s.setFillColor(night ? sf::Color::White : sf::Color(25, 20, 110));
        s.setOutlineColor(night ? sf::Color(25, 20, 110) : sf::Color::White);
        s.setOutlineThickness(3);
        s.setPosition({ x, 100 - dy });
        w.draw(s);
        x += width[i] + 20;
    }
}

string timeText(float t) {                  // 75.3 seconds -> "1:15"
    int s = static_cast<int>(t);
    string sec = to_string(s % 60);
    if (s % 60 < 10) sec = "0" + sec;
    return to_string(s / 60) + ":" + sec;
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({ 1000u, 600u }), "Escape The Twin Towers");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("font.ttf") &&
        !font.openFromFile("C:/Windows/Fonts/arial.ttf") &&
        !font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        cout << "Font not found! Put font.ttf next to the program." << endl;
        return 1;
    }

    // crash sound: if crash.wav is missing the game simply runs without sound
    sf::SoundBuffer crashBuffer;
    optional<sf::Sound> crashSound;
    if (crashBuffer.loadFromFile("crash.wav")) crashSound.emplace(crashBuffer);
    else cout << "crash.wav not found - playing without sound." << endl;

    // night sky stars
    float starX[50], starY[50];
    for (int i = 0; i < 50; i++) { starX[i] = rnd(0, 1000); starY[i] = rnd(0, 330); }

    int state = INTRO;
    float noseX = -20.f, boomTime = 0.f, menuTime = 0.f, smokeTimer = 0.f, totalTime = 0.f;
    bool crashed = false;

    // game variables
    float planeX = 260.f, planeY = 300.f;
    float obstacleTimer = 0.f, coinTimer = 2.f, boosterTimer = 8.f;
    float boostTime = 0.f, invTimer = 0.f, bannerTimer = 0.f, playTime = 0.f;
    int score = 0, dodged = 0, coins = 0, lives = 3, bannerLevel = 1;
    int highScore = loadHighScore();
    bool newRecord = false;
    vector<Hazard> obstacles;
    vector<Item> items;

    // starts a fresh run
    auto startGame = [&]() {
        planeX = 260.f;  planeY = 300.f;
        obstacles.clear();  items.clear();
        obstacleTimer = 0.f;  coinTimer = 2.f;  boosterTimer = 8.f;
        boostTime = 0.f;  invTimer = 0.f;  bannerTimer = 0.f;  playTime = 0.f;
        score = 0;  dodged = 0;  coins = 0;  lives = 3;
        newRecord = false;
        state = PLAYING;
    };

    // ends a run (lost or won) and saves the high score if it was beaten
    auto finishGame = [&](bool won) {
        newRecord = score > highScore;
        if (newRecord) {
            highScore = score;
            saveHighScore(highScore);
        }
        state = won ? WON : GAMEOVER;
    };

    // button areas
    Rect btnPlay = { 370, 295, 260, 70 };
    Rect btnSettings = { 370, 385, 260, 60 };
    Rect btnBack = { 400, 470, 200, 55 };
    Rect btnTheme[2] = { {350, 125, 150, 50}, {520, 125, 150, 50} };
    Rect btnDiff[3] = { {300, 345, 160, 50}, {480, 345, 160, 50}, {660, 345, 160, 50} };
    auto planeRect = [](int i) { return Rect{ 350.f + i * 110.f, 215.f, 100.f, 70.f }; };

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        totalTime += dt;

        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        bool menuReady = (state == MENU && menuTime > 1.3f);

        // ---------- input ----------
        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::FocusLost>() && state == PLAYING)
                state = PAUSED;                                   // auto-pause when the window loses focus

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    if (state == PLAYING) state = PAUSED;
                    else if (state == PAUSED) state = PLAYING;
                    else if (state == GAMEOVER || state == WON || state == SETTINGS) {
                        state = MENU;
                        menuTime = 1.4f;
                    }
                }
                if (key->code == sf::Keyboard::Key::R && (state == GAMEOVER || state == WON))
                    startGame();
                if (key->code == sf::Keyboard::Key::M && state == PAUSED) {
                    state = MENU;
                    menuTime = 1.4f;
                }
            }

            const auto* mb = event->getIf<sf::Event::MouseButtonPressed>();
            if (mb && mb->button == sf::Mouse::Button::Left) {
                if (menuReady) {
                    if (inside(mouse, btnPlay)) startGame();
                    else if (inside(mouse, btnSettings)) state = SETTINGS;
                }
                else if (state == SETTINGS) {
                    if (inside(mouse, btnTheme[0])) night = false;
                    if (inside(mouse, btnTheme[1])) night = true;
                    for (int i = 0; i < 4; i++)
                        if (inside(mouse, planeRect(i))) planeChoice = i;
                    for (int i = 0; i < 3; i++)
                        if (inside(mouse, btnDiff[i])) difficulty = i;
                    if (inside(mouse, btnBack)) { state = MENU; menuTime = 1.4f; }
                }
            }
        }

        // ---------- update ----------
        if (state == INTRO) {
            for (int i = 0; i < 40; i++)                          // smoke puffs fade out
                if (slife[i] > 0) { slife[i] -= 0.8f * dt; sy[i] -= 12 * dt; }
            for (int i = 0; i < N; i++)                           // explosion: gravity + movement
                if (life[i] > 0) { vy[i] += 300 * dt; px[i] += vx[i] * dt; py[i] += vy[i] * dt; life[i] -= dt; }
            if (!crashed) {
                noseX += 300 * dt;
                smokeTimer += dt;
                if (smokeTimer > 0.03f) {
                    smokeTimer = 0;
                    for (int i = 0; i < 40; i++)
                        if (slife[i] <= 0) { sx[i] = noseX - 100; sy[i] = PLANE_Y + 30; slife[i] = 1; break; }
                }
                if (noseX >= TOWER_X) {                           // BOOM
                    noseX = TOWER_X;
                    crashed = true;
                    for (int i = 0; i < N; i++) {
                        float angle = rnd(0, 6.2832f), spd = rnd(60, 400);
                        px[i] = TOWER_X;  py[i] = PLANE_Y;
                        vx[i] = cos(angle) * spd;  vy[i] = sin(angle) * spd;
                        life[i] = rnd(0.8f, 1.7f);
                    }
                    if (crashSound) crashSound->play();
                }
            }
            else {
                boomTime += dt;
                if (boomTime >= 1.8f) { state = MENU; menuTime = 0; }
            }
        }
        else if (state == MENU) {
            menuTime += dt;
        }
        else if (state == PLAYING) {
            playTime += dt;                  // the timer ONLY runs here: not paused, not game over, not won

            float speed = (boostTime > 0) ? 420.f : 280.f;
            if (boostTime > 0) boostTime -= dt;
            if (invTimer > 0) invTimer -= dt;
            if (bannerTimer > 0) bannerTimer -= dt;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                planeX -= speed * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                planeX += speed * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                planeY -= speed * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                planeY += speed * dt;

            if (planeX < 140.f) planeX = 140.f;
            if (planeX > 1000.f) planeX = 1000.f;
            if (planeY < 50.f) planeY = 50.f;
            if (planeY > 505.f) planeY = 505.f;

            int level = dodged / 10 + 1;     // a new level every 10 dodged clouds

            // spawn a storm cloud (faster and harder with level and difficulty)
            float interval = baseInterval[difficulty] * (1.f - 0.1f * (level - 1));
            if (interval < 0.35f) interval = 0.35f;
            obstacleTimer += dt;
            if (obstacleTimer >= interval) {
                Hazard h;
                h.x = 70.f + static_cast<float>(rand() % 820);
                h.y = -60.f;
                h.speed = (160.f + static_cast<float>(rand() % 100)) * speedMult[difficulty] * (1.f + 0.08f * (level - 1));
                h.scale = 0.5f + static_cast<float>(rand() % 50) / 100.f;
                obstacles.push_back(h);
                obstacleTimer = 0.f;
            }

            // spawn pickups
            coinTimer -= dt;
            if (coinTimer <= 0) {
                items.push_back({ 70.f + static_cast<float>(rand() % 820), -20.f, 150.f, 0 });
                coinTimer = rnd(2.0f, 4.0f);
            }
            boosterTimer -= dt;
            if (boosterTimer <= 0) {
                items.push_back({ 70.f + static_cast<float>(rand() % 820), -20.f, 150.f, 1 });
                boosterTimer = rnd(10.0f, 16.0f);
            }

            float aircraftLeft = planeX - 140.f;
            float aircraftTop = planeY - 48.f;

            // storm clouds
            for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--) {
                Hazard& h = obstacles[i];
                h.y += h.speed * dt;

                float cloudLeft = h.x - 45.f * h.scale;
                float cloudTop = h.y - 42.f * h.scale;
                float cloudWidth = 110.f * h.scale;
                float cloudHeight = 80.f * h.scale;

                if (invTimer <= 0 &&
                    collision(aircraftLeft, aircraftTop, 140.f, 93.f, cloudLeft, cloudTop, cloudWidth, cloudHeight)) {
                    lives--;                                  // a heart breaks
                    invTimer = 1.5f;                          // short protection so one cloud cannot hit twice
                    obstacles.erase(obstacles.begin() + i);
                    if (crashSound) crashSound->play();
                    if (lives <= 0) { finishGame(false); break; }
                    continue;
                }

                if (h.y - 42.f * h.scale > 600.f) {           // cloud left the screen = dodged
                    obstacles.erase(obstacles.begin() + i);
                    dodged++;
                    score++;
                    if (dodged % 10 == 0) {                   // level up
                        bannerLevel = dodged / 10 + 1;
                        bannerTimer = 1.5f;
                    }
                }
            }

            // pickups
            if (state == PLAYING) {
                for (int i = static_cast<int>(items.size()) - 1; i >= 0; i--) {
                    Item& it = items[i];
                    it.y += it.speed * dt;
                    if (collision(aircraftLeft, aircraftTop, 140.f, 93.f, it.x - 16, it.y - 16, 32.f, 32.f)) {
                        if (it.type == 0) { coins++; score += 5; }
                        else boostTime = 5.f;
                        items.erase(items.begin() + i);
                    }
                    else if (it.y > 620.f) {
                        items.erase(items.begin() + i);
                    }
                }
            }

            if (state == PLAYING && dodged >= WIN_DODGED)
                finishGame(true);
        }

        // ---------- draw ----------
        sf::Color sky = night ? sf::Color(15, 20, 50) : sf::Color(140, 210, 240);
        window.clear(sky);

        if (night) {
            for (int i = 0; i < 50; i++) {
                float a = 150 + 100 * sin(totalTime * 2 + i);
                box(window, starX[i], starY[i], 2, 2, sf::Color(255, 255, 255, static_cast<uint8_t>(a)));
            }
            ball(window, 880, 70, 30, sf::Color(240, 240, 200));        // moon
            ball(window, 893, 62, 26, sky);                              // cut-out makes a crescent
        }
        else {
            ball(window, 880, 70, 32, sf::Color(255, 220, 60));         // sun
        }

        sf::Color cloudColor = night ? sf::Color(70, 80, 115) : sf::Color(255, 255, 255);
        cloud(window, 60, 130, 1.0f, cloudColor);
        cloud(window, 330, 90, 1.2f, cloudColor);
        cloud(window, 600, 150, 0.9f, cloudColor);
        cloud(window, 120, 480, 1.1f, cloudColor);
        cloud(window, 450, 520, 1.3f, cloudColor);

        if (state == INTRO || state == MENU) tower(window);

        box(window, 0, 550, 1000, 50, night ? sf::Color(30, 70, 45) : sf::Color(90, 170, 90));   // ground

        // draws clouds, pickups, plane and (optionally) the HUD
        auto drawPlay = [&](bool hud) {
            for (const auto& h : obstacles) stormCloud(window, h.x, h.y, h.scale);
            for (const auto& it : items) {
                if (it.type == 0) coinShape(window, it.x, it.y);
                else boosterShape(window, it.x, it.y);
            }
            bool blink = (invTimer > 0 && static_cast<int>(invTimer * 10) % 2 == 0);
            if (!blink) {
                if (boostTime > 0) {                                      // flame behind the plane
                    ball(window, planeX - 142, planeY, 8, sf::Color(255, 140, 0));
                    ball(window, planeX - 154, planeY, 5, sf::Color(255, 230, 80));
                }
                plane(window, planeX, planeY, planeColors[planeChoice]);
            }
            if (!hud) return;
            label(window, font, "Score: " + to_string(score), 24, 20, 15, ink());
            label(window, font, "Level: " + to_string(dodged / 10 + 1) + " / 5", 20, 20, 48, ink());
            label(window, font, "Time: " + timeText(playTime), 20, 20, 75, ink());
            for (int i = 0; i < 3; i++)
                heart(window, 800 + i * 45, 32, 14, i < lives ? sf::Color(220, 30, 50) : sf::Color(90, 90, 100));
            coinShape(window, 810, 75);
            label(window, font, "x " + to_string(coins), 22, 830, 62, ink());
            if (boostTime > 0) {
                label(window, font, "BOOST", 18, 20, 105, sf::Color(0, 150, 220));
                box(window, 20, 130, 150.f * (boostTime / 5.f), 10, sf::Color(0, 150, 220));
            }
            if (bannerTimer > 0)
                center(window, font, ("LEVEL " + to_string(bannerLevel)).c_str(), 60, 500, 200, sf::Color(255, 140, 0));
            center(window, font, "Arrow Keys / WASD to move   |   ESC to pause", 18, 500, 575, ink());
            };

        if (state == INTRO) {
            for (int i = 0; i < 40; i++)
                if (slife[i] > 0)
                    ball(window, sx[i], sy[i], 5 + (1 - slife[i]) * 14, sf::Color(255, 255, 255, static_cast<uint8_t>(slife[i] * 200)));
            if (!crashed || boomTime < 0.15f) plane(window, noseX, PLANE_Y, planeColors[planeChoice]);
            if (crashed && boomTime < 0.12f) ball(window, TOWER_X, PLANE_Y, 120, sf::Color(255, 255, 200, 220));
            for (int i = 0; i < N; i++) {
                if (life[i] <= 0) continue;
                float a = life[i] > 1 ? 1 : life[i];
                sf::Color c = (i % 3 == 0) ? sf::Color(255, 100, 0) : (i % 3 == 1) ? sf::Color(255, 210, 0) : sf::Color(55, 55, 60);
                c.a = static_cast<uint8_t>(a * 255);
                ball(window, px[i], py[i], (i % 3 == 2 ? 14 : 8) * (0.4f + 0.6f * a), c);
            }
            if (crashed && boomTime < 1.2f) {
                float k = boomTime < 0.15f ? boomTime / 0.15f : 1 + 0.05f * sin(boomTime * 30);
                sf::Text t(font, "BOOM!", 100);
                t.setFillColor(sf::Color(255, 200, 0));
                t.setOutlineColor(sf::Color(200, 30, 0));
                t.setOutlineThickness(6);
                sf::FloatRect b = t.getLocalBounds();
                t.setOrigin({ b.position.x + b.size.x / 2, b.position.y + b.size.y / 2 });
                t.setPosition({ 500, 200 });
                t.setScale({ k, k });
                window.draw(t);
            }
        }
        else if (state == MENU) {
            title(window, font, menuTime);
            if (menuTime > 1.3f) {
                button(window, font, "PLAY", btnPlay, inside(mouse, btnPlay), false);
                button(window, font, "SETTINGS", btnSettings, inside(mouse, btnSettings), false);
                center(window, font, ("High Score: " + to_string(highScore)).c_str(), 26, 500, 480, ink());
                center(window, font, (string("Difficulty: ") + diffNames[difficulty]).c_str(), 20, 500, 515, ink());
            }
        }
        else if (state == SETTINGS) {
            center(window, font, "SETTINGS", 48, 500, 55, ink());

            center(window, font, "Theme", 30, 200, 150, ink());
            button(window, font, "Day", btnTheme[0], inside(mouse, btnTheme[0]), !night);
            button(window, font, "Night", btnTheme[1], inside(mouse, btnTheme[1]), night);

            center(window, font, "Plane", 30, 200, 250, ink());
            for (int i = 0; i < 4; i++) {
                Rect r = planeRect(i);
                if (i == planeChoice) box(window, r.x - 5, r.y - 5, r.w + 10, r.h + 10, sf::Color(255, 200, 40));
                box(window, r.x - 2, r.y - 2, r.w + 4, r.h + 4, sf::Color(25, 25, 112));
                box(window, r.x, r.y, r.w, r.h, planeColors[i]);
                center(window, font, planeNames[i], 20, r.x + r.w / 2, r.y + r.h + 18, ink());
            }
            plane(window, 960, 250, planeColors[planeChoice]);           // preview

            center(window, font, "Difficulty", 30, 180, 370, ink());
            for (int i = 0; i < 3; i++)
                button(window, font, diffNames[i], btnDiff[i], inside(mouse, btnDiff[i]), difficulty == i);
            center(window, font, "Harder = faster storm clouds, more of them", 18, 500, 425, ink());

            button(window, font, "BACK", btnBack, inside(mouse, btnBack), false);
        }
        else if (state == PLAYING) {
            drawPlay(true);
        }
        else if (state == PAUSED) {
            drawPlay(true);
            box(window, 0, 0, 1000, 600, sf::Color(0, 0, 0, 140));
            center(window, font, "PAUSED", 60, 500, 200, sf::Color::White);
            center(window, font, "Press ESC to resume", 26, 500, 290, sf::Color::White);
            center(window, font, "Press M for the menu", 26, 500, 340, sf::Color::White);
        }
        else if (state == GAMEOVER || state == WON) {
            drawPlay(false);
            if (state == GAMEOVER) center(window, font, "GAME OVER", 55, 500, 120, sf::Color::Red);
            else center(window, font, "YOU ESCAPED THE TOWERS!", 48, 500, 120, sf::Color(30, 170, 60));
            center(window, font, ("Final Score: " + to_string(score)).c_str(), 30, 500, 200, ink());
            center(window, font, ("Time: " + timeText(playTime) + "     Coins: " + to_string(coins)).c_str(), 24, 500, 245, ink());
            if (newRecord) center(window, font, "NEW HIGH SCORE!", 32, 500, 295, sf::Color(255, 140, 0));
            else center(window, font, ("High Score: " + to_string(highScore)).c_str(), 24, 500, 295, ink());
            center(window, font, "Press R to Restart", 25, 500, 370, ink());
            center(window, font, "Press ESC to return to Menu", 20, 500, 420, ink());
        }

        window.display();
    }

    return 0;
}

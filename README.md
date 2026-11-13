# EscapetheTwinTowers
**A 2D game coded in c++ with SFML.**

## Major Contributions, as of now :

**1. Shahraiz Arshad (26L-0678)** coded for the basic structure of game using rectangles and simple shapes initially. Later, updated those blocks of code using structs that made the game look closer to the desired shapes. In the code pushed so far, the frame/ game window is static which will later be merged with code of other contributor to make it move along the aircraft that is already moving. Moreover, the obstacles' shape will be improved from dark clouds to something else.

**2. Fatima Khan (L26-0731)** coded the visual scene using three basic functions: box for rectangles, ball for circles and poly for shapes with any number of corners. Using these, she built clouds with adjustable size, a tower with 30 windows made by loops, and a plane drawn from its nose tip so it can be moved by changing two values. The plane is drawn last, so it appears in front of everything.

**3. Abdullah Zahid (26L-0685)** worked on the structure and basic framework of game where, in the intro, a plane flies left to right trailing smoke until it hits the tower, triggering 120 explosion particles and a “BOOM!” before the menu appears. Obstacles (towers, storm clouds, birds, wind, gates, lightning, cranes, drones) share one struct and vector, spawn from timers at the screen edges, and unlock as the tier (score / 200) rises. The screen movement is a parallax background of skylines, clouds, ground and rain scrolling at different speeds, while the sky blends from day through sunset, storm, night and dawn. Each obstacle passed gives +10 points, and hitting one ends the run and saves the best score to best.txt. Pressing Play, Retry or Restart calls startGame(), which resets the score, speed, plane position and sky, then clears all obstacles and restarts their spawn timers.

# Pong Game: Web Arcade Edition

A completely reimagined version of the classic 1972 arcade game, initially built in C++/OpenGL and now **fully ported to the Web using WebAssembly (Emscripten)**.

This project evolved from a simple table tennis clone into a high-octane **Arcade/RPG hybrid**, featuring a robust skill system, dynamic settings, retro CRT visuals, and smart AI!

## ✨ Features

- **Web-Ready Execution:** The entire C++ OpenGL engine runs smoothly inside your browser. No downloads or `.exe` files required!
- **Retro Arcade Aesthetic:** Fully revamped UI using monochromatic grayscale, CRT scanline effects, and the classic `Press Start 2P` font.
- **Dynamic Key Bindings:** Customize all inputs (Up, Down, Forward, Backward, Special, Pause) directly from the in-game Settings menu. Preferences are saved automatically to your browser's `localStorage`.
- **Intelligent AI:** Play against human opponents (PVP) or face off against the AI (PVE) across 3 difficulty levels: Easy, Medium, and Hard. The Medium and Hard AI will randomly pick and aggressively use skill loadouts against you!

## 🎮 Game Modes

1. **CLASSIC:** First player to reach 10 points wins. Pure, nostalgic table tennis.
2. **DEAD ZONE:** An intense survival mode where the arena slowly shrinks vertically over time, bordered by pixel-art red skulls. The less space you have, the more chaotic it gets!

## ⚔️ 6-Skill Loadout System

Before every match, players enter the **Skill Loadout Menu** to build their custom arsenal. You choose exactly **3 skills** and map them to your directional inputs (`Forward`, `Backward`, `Neutral`).

When your `CHARGE` bar reaches 100%, hit the corresponding mapped key to unleash chaos:
1. **SMASH (Straight Fast):** The ball redirects instantly into a perfectly straight, high-speed line towards the opponent.
2. **SHIELD (Bigger Paddle):** Your paddle doubles in height for 5 seconds.
3. **GHOST (Invisible Ball):** The ball becomes completely invisible to the opponent for 1.2 seconds!
4. **BLINK (Teleport Ball):** The ball teleports 200 pixels straight forward instantly!
5. **FREEZE (Slow Opponent):** The opponent's paddle speed is cut by 50% for 4 seconds.
6. **EMP (Reverse Controls):** The opponent's Up and Down controls are completely reversed for 4 seconds!

## ⚙️ How to Build (For Developers)

If you want to modify the C++ physics engine (`Files/main.cpp`), you must recompile the game into WebAssembly (`web/pong.js` and `web/pong.wasm`) using Emscripten.

We use **Docker** to ensure the build environment is exactly the same on any machine.
1. Ensure the Docker Daemon is running.
2. Open your terminal in the root folder of the project.
3. Run the npm build script:
```bash
npm run build
```
4. Open `web/index.html` in your web browser (or use a local server like Live Server) to play the game!

> *Created with love, C++, and WebAssembly.*

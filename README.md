# Pong Game: Web Arcade Edition

A comprehensive reimagining of the classic 1972 arcade game, originally engineered in C++ using the OpenGL graphics API and the GLUT toolkit. Through the power of WebAssembly (Emscripten), the game's core C++ physics and rendering systems have been seamlessly integrated into modern web browsers, eliminating the need for local executable files or complex installations.

This project evolved significantly from a simple table tennis clone into a highly competitive, fast-paced Arcade/RPG hybrid. It features a robust dynamic skill system, completely customizable settings, retro CRT visual effects, and a highly capable artificial intelligence for single-player engagements.

## Core Features

- **Web-Ready Architecture:** The entire C++ OpenGL engine runs directly within your browser's execution environment. The bridge between C++ and JavaScript enables real-time HTML5 UI rendering coupled with high-performance C++ physics calculations.
- **Retro Arcade Aesthetic:** The visual identity of the game has been meticulously overhauled to emulate vintage arcade cabinets. This includes a monochromatic grayscale palette, authentic CRT scanline effects, dynamic rendering, and the integration of the classic "Press Start 2P" typography.
- **Dynamic Key Bindings:** Players have absolute control over their inputs (Up, Down, Forward, Backward, Neutral Special, Pause) via an intuitive in-game Settings interface. All control schemes are automatically serialized and persisted directly to the browser's `localStorage` for future sessions.
- **Intelligent AI Integration:** The game accommodates both local multiplayer (PVP) and single-player (PVE) experiences against an AI opponent. The AI operates across three distinct difficulty tiers (Easy, Medium, Hard). At higher difficulties, the AI evaluates its position and proactively utilizes the skill system to secure an advantage over the player.

## Game Modes

1. **CLASSIC:** A traditional competitive mode where the first player to reach 10 points secures victory. It relies entirely on foundational reflexes and positioning.
2. **DEAD ZONE:** A high-stakes, time-sensitive survival mode. As the match progresses, the arena dynamically shrinks vertically, bordered by pixel-art warning skulls. The reduced playing field significantly increases the ball's ricochet rate, leading to intense and chaotic final rallies.

## The 6-Skill Loadout System

The most significant addition to the Web Arcade Edition is the comprehensive Skill Loadout System. Before a match begins, players enter a preparation screen to configure their customized loadout. Players must select precisely 3 distinct abilities from a pool of 6, and individually assign them to their directional inputs (Forward, Backward, and Neutral).

During gameplay, a special gauge charges continuously. Once the meter reaches 100%, players can execute one of their three equipped abilities by pressing the corresponding assigned input, instantly changing the momentum of the game.

The available abilities are:
1. **SMASH (Straight Fast):** The ball instantly breaks its current trajectory and fires in a perfectly straight, high-velocity line toward the opponent's side of the field.
2. **SHIELD (Bigger Paddle):** The player's paddle expands to double its standard height for a duration of 5 seconds, significantly improving defensive coverage.
3. **GHOST (Invisible Ball):** The ball enters a cloaked state, rendering it completely invisible to the opponent's visual rendering for 1.2 seconds, forcing them to predict its current trajectory.
4. **BLINK (Teleport Ball):** The ball manipulates space, instantly teleporting 200 pixels forward along the X-axis. If timed correctly, it bypasses the opponent's defenses entirely.
5. **FREEZE (Slow Opponent):** A debuff that targets the opponent's movement capabilities, severely restricting their paddle speed by 50% for 4 seconds.
6. **EMP (Reverse Controls):** A devastating tactical strike that scrambles the opponent's input logic. For 4 seconds, the opponent's Up and Down controls are completely inverted.

## Build Instructions (For Developers)

For developers wishing to modify the core C++ physics engine or engine architecture (`Files/main.cpp`), the source code must be recompiled into WebAssembly (`web/pong.js` and `web/pong.wasm`) using the Emscripten toolchain.

To ensure consistency across environments, we utilize Docker for the compilation process:

1. Ensure the Docker Daemon is actively running on your local machine.
2. Open a terminal and navigate to the root directory of the project repository.
3. Execute the provided NPM build script:
```bash
npm run build
```
4. Once the build completes, open `web/index.html` in a modern web browser (using a local server such as Live Server is recommended to prevent CORS issues) to play the compiled game.

*Developed with C++, OpenGL, WebAssembly, and a passion for retro gaming.*

# DoubleSwing

**DoubleSwing** is an interactive, physics-accurate double pendulum simulation written in **C++** and compiled to **WebAssembly (WASM)** for real-time browser interaction.

The project supports **desktop (SFML)** and **web (Canvas + WASM)** frontends backed by a shared physics engine.

This demo was first inspired by the work of a friend:
https://github.com/blahnikkai/double-pendulum-simulator

---

## 🌐 Live Demo

➡️ **https://ianzhou01.github.io/DoubleSwing/**  
*(Desktop and touch input supported)*

---

## ✨ Les Fonctionnalités

### Physics Engine
- Rigid-rod **double pendulum** derived from Euler–Lagrange equations
- **RK4 (Runge–Kutta 4th order)** integration for stability and accuracy
- Configurable parameters:
    - Rod lengths *(L₁, L₂)*
    - Bob masses *(m₁, m₂)*
    - Gravitational acceleration *(g)*
    - Linear damping
- Energy-aware design:
    - Total energy
    - Kinetic vs. potential energy split (visualized in UI)

### Interaction
- **Direct manipulation**:
    - Drag bob 1 or bob 2 during motion
    - Velocity estimated via filtered angular derivatives
- Touch + mouse support via **Pointer Events**
- Real-time parameter editing with sane clamping
- Reset and default presets

### Web (WASM) Frontend
- C++ compiled with Emscripten
- Canvas-based rendering
- Fixed-timestep physics loop decoupled from rendering
- Sprite-based bobs with graceful fallback
- Responsive layout with rescaling viewport
- Dark / light theme toggle
- Visual KE/PE energy bar

### Desktop Frontend
- Native C++ app using **SFML**
- Shares the same physics core as the web build
- Useful for debugging and high-precision testing

---

You can drag the system violently without numerical explosions. If it explodes, that’s a bug (kindly create an issue (pls)).

---

## 🛠️ Build Instructions

### Desktop (SFML)

#### Dependencies

- CMake ≥ 3.21
- C++17 compiler
- SFML 2.6+

```text
mkdir build
cd build
cmake ..
cmake --build .
```
Run:
```text
./doubleswing_sfml
```

### Web (WASM)

#### Dependencies

- Emscripten SDK (emsdk)
- Node.js (bundled with emsdk)

```text
emcmake cmake -S . -B build-web
cmake --build build-web
```

To serve locally:

```text
cd web
python -m http.server
```
Open:
```
http://localhost:8000
```
⚠️ You must use a local server — browsers block WASM over file://.

## 🧑‍💻 Author

**Ian Zhou**

Computer Science & Mathematics, University of Florida

Built as a personal exploration at the expense of a couple nights out.
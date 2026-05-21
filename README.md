# Multi-Threaded Retro Space Shooter with Hardware Controller Integration

A high-performance, multi-threaded retro arcade shooter game built natively in LabWindows/CVI. The system leverages real-time multi-threading for physics and graphics rendering, interfaces with high-fidelity audio assets via the BASS sound engine, and features an external physical hardware controller driven by an embedded Arduino over RS-232/UART serial communication.

## 🚀 Key Features

* **Multi-Threaded Rendering Architecture:** Implements a thread pool framework (`CmtThreadFunctionID`) to segregate execution blocks for non-blocking operations:
    * `drawplayer_treadfunc`: Independent player control loop handling positional state.
    * `drawenemy_threadfunc`: Independent enemy movement matrix processing AI trajectories and physics.
    * `damage_threadfunc`: Discrete collision detection and health/hitbox evaluation.
* **Embedded Hardware Controller Integration:** Custom physical input deck utilizing an Arduino microcontroller. Digital inputs are monitored, debounced, and serialized via a 9600-baud UART protocol mapped to COM4 on a 100ms interval loop.
* **Audio Pipeline Integration:** Integrated with the `BASS` audio library engine for background loop streaming (`BASS_SAMPLE_LOOP`) and immediate low-latency sound effect triggers for game events (firing, hits, and game-over soundscapes).

## 🛠 System Architecture & Flow

1. **Hardware Layer (Arduino):** Reads structural push-button matrices (Fire, Move Left, Move Right). It serializes states into characters (`1`, `9`, `0`) appended with a firing status string flag (`y` / `n`) and writes them directly to the serial buffer.
2. **Communication Layer (RS-232):** The CVI application configures and opens COM4 (`OpenComConfig` at 9600 baud). The main input handler queries the port buffer using non-blocking asynchronous reads (`ComRd`) and parses string metrics using string manipulation algorithms (`strstr`).
3. **Application & Graphics Layer (CVI Canvas):** Decoded packets directly alter state variables, triggering immediate multi-threaded updates to draw elements across the active graphics UI engine canvas.

## 📂 Project Structure

* `projecton.c` - Main application layer containing game infrastructure, threading pools, and serial communication parsers.
* `projecton.h` - User Interface Resource layout mappings.
* `controller.ino` - Embedded C++ sketch managing digital pin configuration, internal timers, and UART packet composition.
* `projecton.prj` - LabWindows/CVI deployment workspace project definition.
* `bass.h` / `bass.lib` / `bass.dll` - External audio integration headers and binaries.
* `ספר פרוייקטון.pdf` - Complete documentation, system analysis, and project handbook.

## ⚙️ How to Compile & Run

1. Open `projecton.prj` within the **National Instruments LabWindows/CVI** workspace environment.
2. Flash `controller.ino` to your target Arduino development board and link your physical buttons to the defined input digital pins (`Pin 7`, `Pin 5`, `Pin 4`).
3. Connect the board via USB, verify it maps directly to `COM4` on your machine, build, and run the CVI project environment workspace.

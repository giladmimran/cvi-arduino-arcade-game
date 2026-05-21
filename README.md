# CVI Arduino Arcade Game

A high-performance, multi-threaded retro arcade space shooter built natively within LabWindows/CVI. The system leverages independent background worker threads for real-time physics and rendering, utilizes the BASS sound engine for immersive audio assets, and integrates an external physical hardware controller deck driven by an embedded Arduino over RS-232/UART serial communication.

## 📸 System Previews

![Main Gameplay Interface](gameplay.jpg)<img width="1288" height="926" alt="gameplay" src="https://github.com/user-attachments/assets/709685b9-3dba-4e23-9422-577bdaedcb6e" />
![System Controls and Status Screen](controller.jpg)<img width="948" height="800" alt="controller" src="https://github.com/user-attachments/assets/b3df9126-56bb-476e-8d84-f66a35ac2431" />
https://github.com/user-attachments/assets/2bffe8e9-20c4-4293-a421-e3fa9c99647f

## 🚀 Key Technical Features

* **Multi-Threaded Architecture:** Implements a localized thread pool framework (`CmtThreadFunctionID`) to divide intensive application tasks into concurrent, non-blocking execution flows:
    * `drawplayer_treadfunc`: Manages the player's aircraft position, refresh loops, and localized movement matrix.
    * `drawenemy_threadfunc`: Processes non-player character (NPC) trajectory logic, velocity shifts, and spawning matrices.
    * `damage_threadfunc`: Executes microsecond-accurate collision matrix filtering and health depletion logic.
* **Embedded Controller Optimization:** Custom hardware controller utilizing an Arduino MCU. Monitors digital push-button matrices (Fire, Move Left, Move Right), debounces inputs, and writes serial packet vectors (`1`, `9`, `0` paired with `y`/`n` state flags) over a fixed 100ms hardware timer interval.
* **Audio Pipeline Layering:** Deep integration with the low-latency `BASS` audio library engine to drive dynamic multi-channel soundscapes, streaming background music tracking (`BASS_SAMPLE_LOOP`) alongside high-priority event sound effects for weapons and explosions.

## 🔌 System Architecture & Signal Flow

1. **Physical Input:** The user triggers physical buttons on the external controller. The Arduino reads the digital pins, bundles the structural data packet, and passes it to the hardware UART buffer.
2. **Serial Communication:** The LabWindows/CVI framework initiates communication via COM4 (`OpenComConfig` configured at a standard 9600 baud rate). 
3. **Data Parsing:** The processing loop polls the interface via asynchronous operations (`ComRd`). It reads into an allocated buffer array, drops string null-terminators (`\0`), and performs pattern-matching filters (`strstr`) to instantly update active character vectors.
4. **Canvas Draw Engine:** Updates update coordinates in parallel threads and renders the graphics primitives directly to the user interface resource (`.uir`) canvas panel.

## 📂 Project Structure

* `projecton.c` - Core game engine logic, serial communication protocol parsing, and multi-threading execution setups.
* `projecton.h` - User Interface Resource control element layout addresses and UI panel mappings.
* `controller.ino` - Embedded C++ sketch running on the Arduino for pin state serialization and timing synchronization.
* `projecton.prj` - Workspace management configuration profile for National Instruments LabWindows/CVI.
* `ספר פרוייקטון.pdf` - Comprehensive academic engineering handbook detailing the math, design constraints, and project code breakdown.
* `bass.h` / `bass.lib` / `bass.dll` - External audio processing engine dependencies.
* `sounds/` - Directory containing external high-fidelity audio assets (`.mp3`).

## ⚙️ How to Compile & Deploy

1. Connect your physical Arduino control unit to your PC via USB and ensure it is allocated directly to hardware mapping interface `COM4`.
2. Open the workspace environment using `projecton.prj` within **NI LabWindows/CVI**.
3. Compile and flash `controller.ino` to your targeted microcontroller development board.
4. Build and run the project executable configuration directly from the CVI IDE workspace panel.

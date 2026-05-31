# LEADER DD-2 - Vintage 12-Bit Digital Delay Emulation
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Welcome to **LEADER DD-2**, a premium, production-ready VST3/AU audio plugin built with C++ and the JUCE 7+ framework. This plugin is a high-fidelity physical and mathematical emulation of the legendary **Boss DD-2** compact digital delay pedal released in 1983—the world's first compact digital delay stompbox.

To avoid trademark infringements while preserving the iconic 1980s aesthetic, the plugin is legally rebranded under the name **LEADER**, an authentic tribute to vintage Japanese high-end test equipment and synthesizer engineering from the golden age of electronics.

![LEADER DD-2 Interface](https://i.imgur.com/EaLWhSB.png)

---

## I. KEY SPECIFICATIONS & DSP IMPROVEMENTS

Unlike generic digital delay plugins which repeat audio transparently, the **LEADER DD-2** models the unique hardware limitations, analog support circuits, and digital converters of the 1983 vintage unit:

### 1. Retro 12-Bit / 32kHz Digital Core
*   **Sample-and-Hold Emulation:** The input to the delay line is decimated using a Zero-Order Hold (ZOH) technique at a strict sample rate of **32,000 Hz**, regardless of the host DAW sample rate (supporting up to 192kHz).
*   **12-Bit Quantization:** The audio amplitude is strictly quantized to a 12-bit dynamic range ($2^{12} = 4096$ discrete steps), scaling the signal between $[-2048, 2047]$ and back.

### 2. Premium ADC Thermal Noise & MSB Glitch Emulation
*   **LSB Thermal Dither (-65 dB):** Emulates the analog input thermal noise floor of early converters. By injecting a quiet white noise before the quantizer, it causes the Least Significant Bit (LSB) to toggle randomly, creating an authentic vintage digital background hiss on the tail of notes.
*   **MSB Crossover Distortion (MSB Glitch):** Vintage 12-bit DAC chips (like the PCM53) exhibited significant non-linear switching errors around zero-crossing during Most Significant Bit (MSB) switching. We modeled this with a localized sine-shaped crossover distortion for signals below $0.07$ amplitude. As your notes fade into silence, they dissolve into a **gorgeous gritty, crunchy, and metallic digital texture** that is highly sought-after and instantly recognizable!

### 3. Feedback NE570 Compander Emulation
*   **True Feedback Compressor (2:1):** Early companded delays used the NE570 chip to reduce noise. The original hardware uses a *feedback* compressor topology, where the gain envelope follower is connected to the compressor's *output*. We implemented this using a sample-by-sample feedback loop with a strict $4\text{ ms}$ attack and $10\text{ ms}$ release time. This generates **beautiful transient overshoots (pick attack punch)** and dynamic breathing/pumping action.
*   **Feedforward Expander (1:2):** After the delay line, the signal is expanded dynamically back using a feedforward envelope detector on the wet path, completing the dynamic companding cycle.

### 4. High-Slope Cascaded Simper SVF Filters
*   **Pre-ADC Anti-Aliasing (7kHz):** A steep 4th-order active low-pass filter (implemented as two stable 2nd-order Simper State Variable Filters in series) removes high frequencies before decimation.
*   **Post-DAC Reconstruction (7kHz):** Another cascaded 4th-order Simper SVF low-pass filter at 7kHz smooths out the 12-bit stair-steps, preventing unwanted digital harshness while giving the repeats their warm, rounded tone.
*   **Feedback Loop Damping (3kHz):** A gentle 2nd-order SVF low-pass filter inside the feedback loop progressively darkens each repeat.

### 5. Stabilized Self-Oscillation
*   The **F.Back** knob ranges from $0.0$ to $1.15$, allowing the plugin to enter rich self-oscillation.
*   We implemented a non-linear soft-clipper ($\tanh$) inside the feedback loop to stabilize the signal, creating saturated, warm analog-like harmonic overloads at high feedback settings.

---

## II. HARDWARE CONTROLS & MODES

*   **E.Level (Effect Level):** Standard Boss blend control (Dry path is kept at unity gain, while E.Level injects $0\%$ to $100\%$ of the Wet signal).
*   **F.Back (Feedback):** Controls the number of repeats, from single-echo to self-oscillating overload.
*   **D.Time (Delay Time):** Adjusts the delay time continuously within the range of the selected mode.
*   **Mode Selector (4-Position Rotary):**
    *   **Mode 1:** 12.5ms to 50ms (Short Delay/Doubling)
    *   **Mode 2:** 50ms to 200ms (Medium Slapback)
    *   **Mode 3:** 200ms to 800ms (Long Digital Echo)
    *   **Mode 4 (HOLD):** Loops the digital memory frozen at the time the foot-switch was pressed.
*   **HOLD Pedal (Foot-Switch):** Toggles the loop capture in Mode 4, allowing perfect digital looping in RAM without any filter degradation.

---

## III. DYNAMIC USER PRESET MANAGER (ON-SCREEN UI)

To bypass host-specific VST3 factory preset limitations (such as in Ableton Live), we built an **independent, fully autonomic User Preset Manager** directly inside the stompbox UI:
*   **Preset Selector (Top-Left):** A custom dropdown (`juce::ComboBox`) styled in a retro-industrial look showing both Factory and User presets.
*   **Bidirectional Synchronization:** Seamlessly communicates with the DAW host and instantly animates all the graphical knobs to their correct values when changed.
*   **SAVE Button (Top-Right):** Opens an interactive pop-up dialog to name and save your current parameter values as a persistent XML file on your disk (`C:\Users\<User>\AppData\Roaming\DD2Emulation\UserPresets\`).
*   **DEL Button (Top-Right):** Safely deletes user presets (automatically disabled/grayed out for the 5 factory presets to prevent accidental deletion) and returns to the default preset.

### 5 Built-in Factory Presets:
1.  **Classic Slapback:** ~100ms slapback delay with low feedback, perfect for rockabilly or vocals.
2.  **Warm Analog Echo:** ~350ms warm delay with high low-pass damping, emulating BBD analog circuits.
3.  **Dreamy Oscillation:** ~440ms delay with feedback set to 1.05, creating beautiful, self-stabilized ambient swells.
4.  **Retro 12-Bit Grit:** ~300ms delay with prominent 12-bit noise and MSB crossover distortion.
5.  **Infinite Hold Loop:** Pre-setup for loop capturing using the HOLD foot-switch.

---

## IV. COMPILING & INSTALLATION

The project is fully structured and compile-ready. The source code is organized inside the `Source/` folder:
*   `Source/DD2Engine.h` & `DD2Engine.cpp` (The DSP Core)
*   `Source/PluginProcessor.h` & `Source/PluginProcessor.cpp` (Processor Wrapper)
*   `Source/PluginEditor.h` & `Source/PluginEditor.cpp` (Stompbox UI Editor)

### Option A: Using Projucer
1.  Open the Projucer and select **Open Existing Project...**
2.  Open `NewProject/NewProject.jucer`.
3.  Ensure the **Project Type** is set to **Audio Plug-In** (under Project Settings).
4.  Delete `Main.cpp` from the files explorer under the `Source/` folder if present (a plugin uses the processor entry point).
5.  Save the project (**Ctrl+S**) and launch your IDE (Visual Studio, Xcode).
6.  Select your target (VST3, AU, or Standalone) and run **Rebuild**.

### Option B: Using CMake
Add the files to your `CMakeLists.txt` and build:
```cmake
juce_add_plugin(DD2Emulation
    COMPANY_NAME "Antigravity"
    PLUGIN_MANUFACTURER_CODE "Antg"
    PLUGIN_CODE "Dd2e"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "LEADER DD-2"
)

target_sources(DD2Emulation PRIVATE
    Source/DD2Engine.h
    Source/DD2Engine.cpp
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)
```

Enjoy the legendary grit and dynamic warmth of the **LEADER DD-2**!

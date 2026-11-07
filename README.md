# AcidLine 303 — open-source-inspired TB-303-style synth (v0.1)

An independent JUCE C++ **VST3/Standalone source project**. The name/UI/code do not use Roland branding, samples, firmware, binaries or proprietary code.

**Implemented:** saw/square oscillators with PolyBLEP discontinuity correction; monophonic pitch glide; three-stage nonlinear resonant low-pass model with approximate 18 dB/oct slope; filter and amp envelopes; accent response; 16-step sequencer with rests, accents and slides; host-tempo 16th-note sync; MIDI-playable alternative; output drive; state/preset persistence via DAW.

**Important:** this is a *first functional approximation*, **not** a transistor-level/circuit-calibrated clone of the original hardware. The resonant filter is a simplified saturating three-stage model, not a solved transistor diode ladder; phase, envelope, slide, accent interaction and sequencer timing need reference measurements and listening comparisons. No A/B audio validation has been performed against a TB-303. The VST3 itself is **not precompiled** in this zip.

## Build on Windows (for Ableton Live)
1. Install **Visual Studio 2022** with Desktop Development with C++ and **CMake**; ensure Git is installed.
2. Unzip this source. Open a terminal in the `AcidLine303` folder.
3. Run `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` then `cmake --build build --config Release --target AcidLine303_VST3`.
4. JUCE is downloaded automatically at configuration time if not installed locally. Alternatively place a JUCE checkout in `JUCE/`.
5. Locate `build/AcidLine303_artefacts/Release/VST3/AcidLine 303.vst3` (exact directory may vary with CMake/JUCE), copy the entire `.vst3` bundle to `C:\Program Files\Common Files\VST3`, and rescan plugins in Ableton Live.

## Build on macOS
With Xcode command-line tools and CMake: `cmake -S . -B build -G Xcode && cmake --build build --config Release --target AcidLine303_VST3`. Install built `.vst3` bundle into `~/Library/Audio/Plug-Ins/VST3/` and rescan your DAW.

## Operation
- Add the instrument to a MIDI track. Turn **Host-synced sequencer** on (default) and start DAW transport; it advances one step per 16th note using host BPM.
- Set step pitch (C3 through C5), ON/rest, ACC and SLIDE. SLIDE means **slide from this step into next**, sustaining the envelope and gliding pitch.
- Turn off the sequencer to play from a MIDI keyboard or clip. A legato transition occurs when a new note-on arrives while a previous note is held. Velocity >= 110 acts as accent.
- Tune spans +/- 7 semitones, with midpoint zero.
- Use the DAW's normal preset/project save to persist all parameters and pattern steps.

## Limitations / known work for v0.2
- Sequencer timing is host-BPM-synced but not aligned to absolute bar/PPQ position, so transport seeking mid-play may desynchronise the pattern.
- No pattern chaining, shuffle, external DIN sync, automation-to-MIDI output, swing, per-note length, or multiple patterns.
- Internal 2x substeps reduce some nonlinear aliasing but are not a full high-quality oversampling solution.
- Mono synth, mono signal duplicated to stereo outputs.
- Editor not yet responsive to narrow host windows; fixed 1030x420.
- JUCE-compatible C++ code created here; **JUCE project build was not verified** in this environment because the JUCE framework/SDK is not installed here. The independent DSP unit test was compiled and run successfully with GCC.

## Test independent DSP
`c++ -std=c++17 -O2 tests/test_dsp.cpp -o test_dsp && ./test_dsp`

## Research references
- Roland original TB-303 service notes (1982): https://www.manualslib.com/manual/4220660/Roland-Tb-303.html
- Original-style workflow explanation: https://www.musicradar.com/news/producers-guide-to-the-roland-tb-303-and-clones
- JUCE CMake project reference: https://github.com/juce-framework/JUCE/tree/master/examples/CMake/AudioPlugin

## JUCE licensing
Review JUCE's current license before distributing compiled plugins, especially for closed-source/commercial use: https://juce.com/legal/

#pragma once
#include <algorithm>
#include <cmath>

// Independent DSP core. Intentional 303-inspired approximation, not circuit-identical emulation.
namespace acid {
constexpr double pi = 3.14159265358979323846;
inline double clamp(double x, double a, double b) { return std::max(a, std::min(b, x)); }
inline double midiHz(double note) { return 440.0 * std::pow(2.0, (note - 69.0) / 12.0); }

class Voice {
public:
    void prepare(double sampleRate) { sr = std::max(8000.0, sampleRate); reset(); }
    void reset() { phase = 0; amp = env = pitch = target = 0; gate = false; accented = false; z1 = z2 = z3 = 0; dcIn = dcOut = 0; }
    // legato slides retain amp/filter envelope state and interpolate pitch.
    void noteOn(int midi, bool accent, bool legato) {
        target = clamp((double)midi, 0, 127);
        if (!legato || !gate) { pitch = target; env = 1.0; amp = 0; }
        accented = accent;
        gate = true;
    }
    void noteOff() { gate = false; }
    bool isGated() const { return gate; }
    // Controls normalized 0..1 except tuning (-1..1) and drive (0..1).
    float render(float cutoff, float resonance, float envmod, float decay,
                 float accentAmount, float tune, bool square, float drive) {
        const double dt = 1.0 / sr;
        const double glideCoeff = std::exp(-dt / 0.055);
        pitch = target + (pitch - target) * glideCoeff;
        const double freq = midiHz(pitch + (double)tune * 7.0);
        const double attack = 1.0 - std::exp(-dt / 0.002);
        const double release = std::exp(-dt / 0.075);
        amp = gate ? amp + (1.0 - amp) * attack : amp * release;
        const double decaySec = 0.075 + std::pow((double)decay, 1.7) * 1.8;
        env *= std::exp(-dt / decaySec);
        const double accentLift = accented ? (double)accentAmount : 0.0;
        // In hardware an accent interacts with multiple analog envelopes/circuits.
        const double filterCV = 0.05 + (double)cutoff * 0.88
                              + env * ((double)envmod * 0.68 + accentLift * 0.22);
        const double fc = clamp(45.0 * std::pow(2.0, filterCV * 8.4), 30.0, 0.37 * sr);
        const double resonanceFeedback = 2.8 * (double)resonance;
        double sample = 0;
        // 2x internal substeps lower aliasing from oscillator/filter nonlinearity.
        for (int k = 0; k < 2; ++k) {
            const double inc = std::min(freq / (sr * 2.0), 0.49);
            phase += inc;
            if (phase >= 1.0) phase -= 1.0;
            // PolyBLEP band-limits each discontinuity (including square's midpoint).
            auto blep = [](double t, double d) {
                if (t < d) { const double x = t/d; return x+x-x*x-1.0; }
                if (t > 1.0-d) { const double x = (t-1.0)/d; return x*x+x+x+1.0; }
                return 0.0;
            };
            double osc;
            if (square) {
                osc = (phase < 0.5 ? 1.0 : -1.0) + blep(phase, inc)
                    - blep(std::fmod(phase + 0.5, 1.0), inc);
            } else {
                osc = 2.0 * phase - 1.0 - blep(phase, inc);
            }
            // Three saturating one-pole stages in resonant feedback loop:
            // 18 dB/oct *character*, not an exact diode-ladder circuit solution.
            const double g = 1.0 - std::exp(-2.0 * pi * fc / (sr * 2.0));
            const double input = std::tanh(1.5 * (osc - resonanceFeedback * z3));
            z1 += g * (input - std::tanh(z1));
            z2 += g * (std::tanh(z1) - std::tanh(z2));
            z3 += g * (std::tanh(z2) - std::tanh(z3));
            sample += z3;
        }
        sample *= 0.5;
        const double ampAccent = 1.0 + accentLift * 0.9;
        sample *= amp * ampAccent * 0.9;
        sample = std::tanh(sample * (1.0 + 10.0 * (double)drive));
        // DC blocker
        const double hp = sample - dcIn + 0.995 * dcOut;
        dcIn = sample; dcOut = hp;
        return (float)(0.72 * hp);
    }
private:
    double sr = 44100, phase = 0, amp = 0, env = 0, pitch = 0, target = 0;
    double z1 = 0, z2 = 0, z3 = 0, dcIn = 0, dcOut = 0;
    bool gate = false, accented = false;
};
}

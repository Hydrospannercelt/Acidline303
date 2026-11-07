#pragma once
#include <algorithm>
#include <cmath>

// AcidLine 303: original 303-inspired DSP, not a component-accurate Roland model.
// 4x oversampled, nonlinear resonant three-stage zero-delay-feedback approximation.
namespace acid {
constexpr double pi = 3.14159265358979323846;
inline double clamp(double x, double a, double b) { return std::max(a, std::min(b, x)); }
inline double midiHz(double note) { return 440.0 * std::pow(2.0, (note - 69.0) / 12.0); }

class Voice {
public:
    void prepare(double sampleRate) { sr = std::max(8000.0, sampleRate); reset(); }
    void reset() {
        phase = amp = env = accentEnv = pitch = target = 0.0;
        gate = accented = false;
        z1 = z2 = z3 = dcIn = dcOut = 0.0;
    }
    void noteOn(int midi, bool accent, bool legato) {
        target = clamp((double)midi, 0.0, 127.0);
        if (!legato || !gate) {
            pitch = target;
            env = 1.0;
            amp = 0.0;
        }
        // Accent has its own brief contour; a legato accent adds a new transient
        // without restarting the filter's main decay or amplitude envelope.
        accented = accent;
        if (accent) accentEnv = 1.0;
        gate = true;
    }
    void noteOff() { gate = false; }
    bool isGated() const { return gate; }

    float render(float cutoff, float resonance, float envmod, float decay,
                 float accentAmount, float tune, bool square, float drive) {
        const double dt = 1.0 / sr;
        const double r = clamp(resonance, 0.0, 1.0);
        const double e = clamp(envmod, 0.0, 1.0);
        const double a = accented ? clamp(accentAmount, 0.0, 1.0) : 0.0;
        const double glideCoeff = std::exp(-dt / 0.060);
        pitch = target + (pitch - target) * glideCoeff;
        const double freq = midiHz(pitch + (double)tune * 7.0);
        const double attack = 1.0 - std::exp(-dt / 0.0025);
        const double release = std::exp(-dt / 0.070);
        amp = gate ? amp + (1.0 - amp) * attack : amp * release;

        const double decaySec = 0.085 + std::pow(clamp(decay, 0.0, 1.0), 1.7) * 1.55;
        env *= std::exp(-dt / decaySec);
        accentEnv *= std::exp(-dt / 0.115);

        // Express cutoff and envelope motion in octaves so resonant sweeps stay
        // lively even at low cutoff settings. Accent creates a short extra sweep.
        const double cutoffOctaves = 0.9 + clamp(cutoff, 0.0, 1.0) * 7.5;
        const double envOctaves = env * (0.15 + 4.6 * e)
                                + a * accentEnv * (1.0 + 1.6 * e);
        const double fc = clamp(28.0 * std::pow(2.0, cutoffOctaves + envOctaves),
                                22.0, std::min(18000.0, 0.38 * sr));

        // Three one-pole stages require more feedback than a four-pole ladder
        // to build a strong resonant peak. A saturating feedback path limits it.
        const double feedback = 7.35 * std::pow(r, 1.65);
        constexpr double oversample = 4.0;
        const double internalRate = sr * oversample;
        const double g = std::tan(pi * fc / internalRate);
        const double G = g / (1.0 + g);
        const double inc = std::min(freq / internalRate, 0.49);
        double sum = 0.0;

        for (int k = 0; k < 4; ++k) {
            phase += inc;
            if (phase >= 1.0) phase -= 1.0;
            const auto blep = [](double t, double d) {
                if (t < d) { const double x = t / d; return x + x - x*x - 1.0; }
                if (t > 1.0 - d) { const double x = (t - 1.0) / d; return x*x + x + x + 1.0; }
                return 0.0;
            };
            const double osc = square
                ? (phase < 0.5 ? 1.0 : -1.0) + blep(phase, inc)
                    - blep(std::fmod(phase + 0.5, 1.0), inc)
                : 2.0 * phase - 1.0 - blep(phase, inc);

            // Compute the feed-through of the cascade and solve the linear
            // feedback approximately. The two nonlinear tanh functions model
            // soft saturation at the input and in the resonant return path.
            const double G3 = G * G * G;
            const double feed = (1.0 - G) * z3
                              + G * (1.0 - G) * z2
                              + G * G * (1.0 - G) * z1;
            double u = (osc - feedback * std::tanh(feed)) / (1.0 + feedback * G3);
            u = std::tanh(1.35 * u);
            // Two refinement iterations tame peaks at fast cutoff transitions.
            for (int iter = 0; iter < 2; ++iter) {
                const double predicted = G3 * u + feed;
                const double desired = std::tanh(1.35 * (osc - feedback * std::tanh(predicted)));
                u += 0.45 * (desired - u);
            }
            const auto pole = [G](double input, double& state) {
                const double y = G * input + (1.0 - G) * state;
                state += 2.0 * G * (input - state);
                return y;
            };
            const double y1 = pole(u, z1);
            const double y2 = pole(y1, z2);
            const double y3 = pole(y2, z3);
            sum += y3;
        }
        // Peak/loudness compensation maintains audibility at high resonance.
        double output = (sum / oversample) * (1.0 + 1.6 * r);
        output *= amp * (0.72 + a * (0.45 + 0.55 * accentEnv));
        output = std::tanh(output * (1.0 + 7.0 * clamp(drive, 0.0, 1.0)));
        const double hp = output - dcIn + 0.995 * dcOut;
        dcIn = output;
        dcOut = hp;
        return (float)(0.72 * hp);
    }
private:
    double sr = 44100.0, phase = 0.0, amp = 0.0, env = 0.0, accentEnv = 0.0;
    double pitch = 0.0, target = 0.0;
    double z1 = 0.0, z2 = 0.0, z3 = 0.0, dcIn = 0.0, dcOut = 0.0;
    bool gate = false, accented = false;
};
}

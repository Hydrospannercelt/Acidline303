#pragma once
#include <JuceHeader.h>
#include "AcidDSP.h"

class AcidLineAudioProcessor final : public juce::AudioProcessor {
public:
    AcidLineAudioProcessor();
    ~AcidLineAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AcidLine 303"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.2; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default Acid"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout& layout) const override {
        return layout.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
            || layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }
    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    static juce::String stepId(int i, const char* suffix) { return "s" + juce::String(i) + suffix; }
private:
    acid::Voice voice;
    double sampleRateHz = 44100.0, samplesUntilStep = 0.0;
    int currentStep = -1, lastMidiNote = -1;
    bool wasPlaying = false, previousStepSlides = false, midiHeld = false;
    void beginStep();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidLineAudioProcessor)
};

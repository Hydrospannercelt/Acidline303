#include "PluginProcessor.h"
#include "PluginEditor.h"

AcidLineAudioProcessor::AcidLineAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "AcidLineState", makeLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout AcidLineAudioProcessor::makeLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto add = [&](const char* id, const char* name, float def) {
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name,
             juce::NormalisableRange<float>{0.0f, 1.0f}, def));
    };
    add("cutoff", "Cutoff", 0.30f); add("resonance", "Resonance", 0.75f);
    add("envmod", "Env Mod", 0.70f); add("decay", "Decay", 0.35f);
    add("accent", "Accent", 0.75f); add("drive", "Drive", 0.18f);
    add("tune", "Tune", 0.50f);
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"wave", 1},
        "Waveform", juce::StringArray{"Saw", "Square"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"seq", 1},
        "Sequencer On", true));
    for (int i = 0; i < 16; ++i) {
        juce::StringArray notes;
        const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        for (int n = 24; n <= 84; ++n) notes.add(juce::String(names[n % 12]) + juce::String(n / 12 - 1));
        const int pattern[] = {12,12,19,12,15,12,22,19,12,24,19,15,12,15,19,10};
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{stepId(i,"note"),1}, "Step " + juce::String(i+1) + " Note", notes, pattern[i] + 24));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{stepId(i,"on"),1}, "Step " + juce::String(i+1) + " Enabled", i != 7 && i != 15));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{stepId(i,"acc"),1}, "Step " + juce::String(i+1) + " Accent", i % 4 == 0));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{stepId(i,"slide"),1}, "Step " + juce::String(i+1) + " Slide To Next", i == 2 || i == 10));
    }
    return layout;
}

void AcidLineAudioProcessor::prepareToPlay(double sampleRate, int) {
    sampleRateHz = sampleRate; voice.prepare(sampleRate); samplesUntilStep = 0;
    currentStep = -1; wasPlaying = false; previousStepSlides = false; midiHeld = false;
}

void AcidLineAudioProcessor::beginStep() {
    currentStep = (currentStep + 1) % 16;
    const auto val = [&](const char* s) { return state.getRawParameterValue(stepId(currentStep,s))->load(); };
    bool on = val("on") > 0.5f;
    bool acc = val("acc") > 0.5f;
    bool slide = val("slide") > 0.5f;
    if (on) voice.noteOn(24 + (int)std::lround(val("note")), acc, previousStepSlides);
    else voice.noteOff();
    previousStepSlides = on && slide;
}

void AcidLineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const auto value = [&](const char* id) { return state.getRawParameterValue(id)->load(); };
    const float cutoff = value("cutoff"), res = value("resonance"), env = value("envmod");
    const float decay = value("decay"), accent = value("accent"), drive = value("drive");
    const float tune = value("tune") * 2.0f - 1.0f;
    const bool square = value("wave") > 0.5f, seqOn = value("seq") > 0.5f;
    double bpm = 120.0;
    bool hostPlaying = false;
    if (auto* ph = getPlayHead()) {
        if (auto p = ph->getPosition()) {
            if (auto b = p->getBpm()) bpm = *b;
            hostPlaying = p->getIsPlaying();
        }
    }
    bpm = acid::clamp(bpm, 40.0, 300.0);
    const double stepSamples = sampleRateHz * 60.0 / (bpm * 4.0); // 16ths
    if (seqOn && hostPlaying && !wasPlaying) {
        currentStep = -1; samplesUntilStep = 0; previousStepSlides = false; voice.noteOff();
    }
    if ((!seqOn || !hostPlaying) && wasPlaying) voice.noteOff();
    wasPlaying = seqOn && hostPlaying;

    juce::MidiBuffer::Iterator iter(midi);
    juce::MidiMessage message;
    int eventSample = 0;
    bool eventAvailable = iter.getNextEvent(message, eventSample);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        if (seqOn && hostPlaying) {
            if (samplesUntilStep <= 0.0) {
                beginStep(); samplesUntilStep += stepSamples;
            }
            const bool slide = state.getRawParameterValue(stepId(currentStep,"slide"))->load() > 0.5f;
            if (!slide && samplesUntilStep < stepSamples * 0.17) voice.noteOff();
            samplesUntilStep -= 1.0;
        } else {
            while (eventAvailable && eventSample <= i) {
                if (message.isNoteOn()) {
                    voice.noteOn(message.getNoteNumber(), message.getVelocity() >= 110,
                                 midiHeld && lastMidiNote >= 0);
                    midiHeld = true; lastMidiNote = message.getNoteNumber();
                } else if (message.isNoteOff() && message.getNoteNumber() == lastMidiNote) {
                    voice.noteOff(); midiHeld = false; lastMidiNote = -1;
                } else if (message.isAllNotesOff() || message.isAllSoundOff()) {
                    voice.noteOff(); midiHeld = false; lastMidiNote = -1;
                }
                eventAvailable = iter.getNextEvent(message, eventSample);
            }
        }
        const float x = voice.render(cutoff, res, env, decay, accent, tune, square, drive);
        for (int c = 0; c < buffer.getNumChannels(); ++c) buffer.setSample(c, i, x);
    }
}

void AcidLineAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    if (auto xml = state.copyState().createXml()) copyXmlToBinary(*xml, dest);
}
void AcidLineAudioProcessor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* AcidLineAudioProcessor::createEditor() { return new AcidLineAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AcidLineAudioProcessor(); }

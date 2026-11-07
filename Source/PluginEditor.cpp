#include "PluginEditor.h"

AcidLineAudioProcessorEditor::AcidLineAudioProcessorEditor(AcidLineAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setSize(1030, 420);
    const char* ids[] = {"cutoff","resonance","envmod","decay","accent","drive","tune"};
    const char* names[] = {"CUTOFF","RESONANCE","ENV MOD","DECAY","ACCENT","DRIVE","TUNE"};
    for (int i = 0; i < 7; ++i) {
        knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 20);
        addAndMakeVisible(knobs[i]);
        labels[i].setText(names[i], juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
        addAndMakeVisible(labels[i]);
        knobAttachments[i] = std::make_unique<SliderAttach>(p.state, ids[i], knobs[i]);
    }
    waveform.addItem("SAW", 1); waveform.addItem("SQUARE", 2);
    addAndMakeVisible(waveform);
    waveAttachment = std::make_unique<ComboAttach>(p.state, "wave", waveform);
    addAndMakeVisible(seqButton);
    seqAttachment = std::make_unique<ButtonAttach>(p.state,"seq",seqButton);
    for (int i = 0; i < 16; ++i) {
        for (int n = 0; n <= 24; ++n) {
            const char* pitch[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            const int midi = n + 48;
            notes[i].addItem(juce::String(pitch[midi % 12]) + juce::String(midi / 12 - 1), n+1);
        }
        addAndMakeVisible(notes[i]);
        noteAttachments[i] = std::make_unique<ComboAttach>(p.state, AcidLineAudioProcessor::stepId(i,"note"),notes[i]);
        active[i].setButtonText(""); accents[i].setButtonText(""); slides[i].setButtonText("");
        addAndMakeVisible(active[i]); addAndMakeVisible(accents[i]); addAndMakeVisible(slides[i]);
        activeAttachments[i] = std::make_unique<ButtonAttach>(p.state,AcidLineAudioProcessor::stepId(i,"on"),active[i]);
        accentAttachments[i] = std::make_unique<ButtonAttach>(p.state,AcidLineAudioProcessor::stepId(i,"acc"),accents[i]);
        slideAttachments[i] = std::make_unique<ButtonAttach>(p.state,AcidLineAudioProcessor::stepId(i,"slide"),slides[i]);
    }
}
void AcidLineAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour::fromRGB(31, 33, 35));
    g.setColour(juce::Colour::fromRGB(245, 166, 45));
    g.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    g.drawText("ACIDLINE 303", 22, 12, 360, 42, juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Independent 1980s-style acid bass instrument", 390, 24, 420, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour::fromRGB(245,166,45));
    g.drawLine(22.0f, 200.0f, 1008.0f, 200.0f, 2.0f);
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.0f);
    g.drawText("PITCH", 0, 245, 78, 20, juce::Justification::centredRight);
    g.drawText("ON", 0, 285, 78, 20, juce::Justification::centredRight);
    g.drawText("ACC", 0, 323, 78, 20, juce::Justification::centredRight);
    g.drawText("SLIDE", 0, 360, 78, 20, juce::Justification::centredRight);
    for (int i = 0; i < 16; ++i) {
        g.setColour(juce::Colour::fromRGB(245,166,45));
        g.drawText(juce::String(i + 1), 89 + i*57, 215, 52, 20, juce::Justification::centred);
    }
    g.setColour(juce::Colours::grey);
    g.drawText("Slide applies to the NEXT step. Start DAW playback to run sequence; disable SEQ for MIDI notes.",
               22, 393, 985, 17, juce::Justification::centredLeft);
}
void AcidLineAudioProcessorEditor::resized() {
    for (int i = 0; i < 7; ++i) {
        const int x = 16 + i*109;
        knobs[i].setBounds(x, 80, 98, 98);
        labels[i].setBounds(x, 61, 98, 20);
    }
    waveform.setBounds(810, 100, 110, 26);
    seqButton.setBounds(810, 145, 200, 28);
    for (int i = 0; i < 16; ++i) {
        const int x = 89 + i*57;
        notes[i].setBounds(x, 244, 54, 26);
        active[i].setBounds(x+18, 285, 25, 24);
        accents[i].setBounds(x+18, 323, 25, 24);
        slides[i].setBounds(x+18, 361, 25, 24);
    }
}

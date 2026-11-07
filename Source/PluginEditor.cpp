#include "PluginEditor.h"

AcidLineAudioProcessorEditor::AcidLineAudioProcessorEditor(AcidLineAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setSize(1030, 483);
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
    for (const auto& name : patternNames()) patternPicker.addItem(name, patternPicker.getNumItems() + 1);
    addAndMakeVisible(patternPicker);
    patternPicker.setSelectedId(1, juce::dontSendNotification);
    loadPatternButton.setButtonText("LOAD PATTERN");
    addAndMakeVisible(loadPatternButton);
    loadPatternButton.onClick = [this] { loadSelectedPattern(); };
    for (int i = 0; i < 16; ++i) {
        for (int n = 0; n <= 60; ++n) {
            const char* pitch[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            const int midi = n + 24;
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
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText("30 ORIGINAL ACID PATTERNS", 24, 417, 250, 30, juce::Justification::centredLeft);
    g.setFont(12.0f);
    g.drawText("Choose a pattern and click LOAD. Replaces the current 16 steps. Save your Ableton set to keep edits.",
               24, 455, 970, 20, juce::Justification::centredLeft);
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
    patternPicker.setBounds(290, 416, 350, 30);
    loadPatternButton.setBounds(655, 416, 170, 30);
    for (int i = 0; i < 16; ++i) {
        const int x = 89 + i*57;
        notes[i].setBounds(x, 244, 54, 26);
        active[i].setBounds(x+18, 285, 25, 24);
        accents[i].setBounds(x+18, 323, 25, 24);
        slides[i].setBounds(x+18, 361, 25, 24);
    }
}

// Original patterns composed for AcidLine 303, not copied from a commercial preset library.
// MIDI note 24 = C1; '-' is a rest. Accent and slide are 16-bit step masks.
namespace {
struct AcidPattern { const char* name; std::array<int, 16> pitch; unsigned accent, slide; };
constexpr AcidPattern bank[] = {
    {"01  Warehouse Roller", {36,36,43,36,-1,48,43,36,39,36,43,-1,48,43,36,34}, 0x4529, 0x2C44},
    {"02  Liquid Spiral",   {36,43,48,43,39,36,39,43,48,51,48,43,39,43,36,-1}, 0x9292, 0x5654},
    {"03  Glasgow Afterhours",{33,33,-1,45,40,33,36,40,45,40,33,-1,36,40,45,33},0x4919,0x3464},
    {"04  Rubber Duck",     {36,-1,36,48,43,36,-1,39,36,48,43,-1,36,39,43,36},0x254D,0x6214},
    {"05  Night Tram",      {38,38,45,50,-1,45,41,38,41,45,38,50,45,-1,41,38},0x8925,0x484A},
    {"06  Submerged",       {31,31,43,38,31,-1,43,41,38,31,38,43,46,43,38,-1},0x44A1,0x3194},
    {"07  Berlin Steps",    {36,36,36,43,39,36,48,-1,36,39,43,48,46,43,39,36},0xA149,0x2A18},
    {"08  Silver Machine",  {40,47,52,47,40,-1,44,47,52,54,52,47,44,40,47,-1},0x514A,0x4B44},
    {"09  Deep Current",    {28,40,35,28,-1,35,40,28,31,35,40,43,40,35,31,28},0xA224,0x214C},
    {"10  Plastic Funk",    {36,43,36,39,48,43,-1,36,43,48,51,48,43,39,36,-1},0x4A95,0x4892},
    {"11  Neon Tunnel",    {34,46,41,34,46,41,38,-1,34,41,46,50,46,41,38,34},0x9524,0x3268},
    {"12  Acid Sunrise",   {36,39,43,48,51,48,43,39,36,-1,48,43,39,36,31,36},0xA521,0x2564},
    {"13  Midnight Pressure", {24,24,36,31,-1,27,36,39,24,31,36,-1,39,36,27,24}, 0x4449, 0x1224},
    {"14  Toxic Bubbles", {29,41,36,32,29,41,-1,36,32,44,41,36,29,-1,32,41}, 0x1221, 0x0912},
    {"15  Rotterdam Rumble", {26,26,38,33,29,-1,38,41,26,33,38,45,41,38,-1,26}, 0x0891, 0x4444},
    {"16  Chrome Serpent", {36,48,43,42,36,-1,48,55,43,42,36,48,-1,43,39,36}, 0x888A, 0x0505},
    {"17  Rain on Concrete", {31,-1,38,43,31,34,38,-1,43,46,43,38,34,31,26,31}, 0x2221, 0x4444},
    {"18  Rubber Cathedral", {33,45,40,36,33,45,48,40,-1,36,33,40,45,48,40,33}, 0x2849, 0x1222},
    {"19  Underground Pulse", {24,24,-1,31,36,31,27,24,24,-1,39,36,31,27,31,24}, 0x8511, 0x2848},
    {"20  Fluorescent Fever", {40,47,52,54,52,47,44,40,47,-1,56,52,47,44,40,35}, 0x4489, 0x1124},
    {"21  Slippery Business", {28,40,35,32,28,-1,40,44,35,32,28,35,40,-1,32,28}, 0x8885, 0x1242},
    {"22  Harbour Acid", {34,34,41,46,41,38,-1,34,46,50,46,41,38,34,29,-1}, 0x2109, 0x1224},
    {"23  Clandestine Rave", {36,39,48,43,36,51,48,-1,39,36,43,48,55,48,39,36}, 0x1429, 0x2212},
    {"24  Darkroom Drip", {25,37,32,29,-1,37,41,32,29,25,32,37,44,41,37,-1}, 0x2451, 0x1222},
    {"25  Broken Neon", {38,45,-1,50,47,42,38,35,38,42,47,-1,54,50,45,38}, 0x1109, 0x4444},
    {"26  Jellyfish Jam", {27,39,34,39,46,-1,43,39,34,27,31,34,39,46,43,27}, 0x2112, 0x4889},
    {"27  Acid at Dawn", {36,43,48,51,55,51,48,43,39,36,-1,48,43,39,31,36}, 0x8891, 0x1224},
    {"28  Pressure Valve", {30,30,42,37,33,30,-1,45,42,37,33,30,42,45,37,-1}, 0x1089, 0x2122},
    {"29  Sub Bass Mutant", {24,-1,24,36,31,27,24,39,36,-1,31,24,36,43,39,24}, 0x2889, 0x4444},
    {"30  Last Train to Acid", {35,42,47,42,38,35,47,50,-1,42,38,35,42,47,54,35}, 0x4891, 0x2222},
};
}

juce::StringArray AcidLineAudioProcessorEditor::patternNames() {
    juce::StringArray names;
    for (const auto& pat : bank) names.add(pat.name);
    return names;
}

void AcidLineAudioProcessorEditor::loadSelectedPattern() {
    const int index = patternPicker.getSelectedId() - 1;
    if (index < 0 || index >= static_cast<int>(std::size(bank))) return;
    const auto& pat = bank[index];
    auto set = [this](const juce::String& id, float normalized) {
        if (auto* parameter = processor.state.getParameter(id)) {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, normalized));
            parameter->endChangeGesture();
        }
    };
    for (int i=0; i<16; ++i) {
        const bool enabled = pat.pitch[i] >= 0;
        set(AcidLineAudioProcessor::stepId(i,"note"), static_cast<float>(juce::jlimit(24,84, enabled ? pat.pitch[i] : 24) - 24) / 60.0f);
        set(AcidLineAudioProcessor::stepId(i,"on"), enabled ? 1.0f : 0.0f);
        set(AcidLineAudioProcessor::stepId(i,"acc"), (pat.accent & (1u << i)) ? 1.0f : 0.0f);
        set(AcidLineAudioProcessor::stepId(i,"slide"), (pat.slide & (1u << i)) ? 1.0f : 0.0f);
    }
}

#pragma once
#include "PluginProcessor.h"
#include <array>

class AcidLineAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit AcidLineAudioProcessorEditor(AcidLineAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    AcidLineAudioProcessor& processor;
    std::array<juce::Slider, 7> knobs;
    std::array<juce::Label, 7> labels;
    std::array<std::unique_ptr<SliderAttach>, 7> knobAttachments;
    juce::ComboBox waveform;
    std::unique_ptr<ComboAttach> waveAttachment;
    juce::ToggleButton seqButton{"Host-synced sequencer"};
    std::unique_ptr<ButtonAttach> seqAttachment;
    std::array<juce::ComboBox,16> notes;
    std::array<std::unique_ptr<ComboAttach>,16> noteAttachments;
    std::array<juce::ToggleButton,16> active, accents, slides;
    std::array<std::unique_ptr<ButtonAttach>,16> activeAttachments, accentAttachments, slideAttachments;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidLineAudioProcessorEditor)
};

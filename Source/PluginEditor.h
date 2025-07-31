#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    JUCE Audio Generator Plugin Editor

    This class implements the AudioProcessorEditor interface to create the
    plugin's graphical user interface. It provides controls for the plugin
    parameters and integrates with the host's automation system through
    parameter attachments.
*/
class JUCEAudioGeneratorEditor : public juce::AudioProcessorEditor,
                                 public juce::Timer
{
public:
    JUCEAudioGeneratorEditor (JUCEAudioGeneratorProcessor&);
    ~JUCEAudioGeneratorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JUCEAudioGeneratorProcessor& audioProcessor;

    // GUI Components
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Slider frequencySlider;
    juce::Label frequencyLabel;
    juce::ComboBox waveTypeCombo;
    juce::Label waveTypeLabel;
    juce::Label statusLabel;

    // Parameter attachments for automatic synchronization with plugin parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveTypeAttachment;

    // Helper methods
    void setupComponents();
    void setupStyling();
    void updateStatus();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JUCEAudioGeneratorEditor)
};

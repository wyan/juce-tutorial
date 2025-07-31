#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"

class MainComponent : public juce::Component,
                     public juce::Slider::Listener,
                     public juce::ComboBox::Listener,
                     public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Listener interfaces
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

    // Timer callback for audio status updates
    void timerCallback() override;

private:
    // Helper methods
    void setupComponents();
    void setupStyling();
    void updateStatus();
    void handlePlayStopButton();
    void handleFrequencyChange();
    void updateAudioStatus();

    // GUI Components
    juce::Label titleLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::TextButton playStopButton;
    juce::ComboBox frequencyCombo;
    juce::Label frequencyLabel;
    juce::TextEditor statusEditor;
    juce::Label audioStatusLabel;

    // Audio Engine
    std::unique_ptr<AudioEngine> audioEngine;

    // State variables
    bool isPlaying = false;
    double currentVolume = 0.5;
    int selectedFrequency = 440;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

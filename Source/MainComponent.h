#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::Component,
                     public juce::Slider::Listener,
                     public juce::ComboBox::Listener
{
public:
    MainComponent();
    ~MainComponent() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Listener interfaces
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

private:
    // Helper methods
    void setupComponents();
    void setupStyling();
    void updateStatus();
    void handlePlayStopButton();
    void handleFrequencyChange();

    // GUI Components
    juce::Label titleLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::TextButton playStopButton;
    juce::ComboBox frequencyCombo;
    juce::Label frequencyLabel;
    juce::TextEditor statusEditor;

    // State variables
    bool isPlaying = false;
    double currentVolume = 0.5;
    int selectedFrequency = 440;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

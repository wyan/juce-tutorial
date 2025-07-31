#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"

//==============================================================================
/**
    Shared Audio Component

    This component can be used in both standalone applications and plugin editors.
    It provides a common interface for controlling audio parameters and displaying
    status information.
*/
class SharedAudioComponent : public juce::Component,
                            public juce::Timer
{
public:
    //==============================================================================
    SharedAudioComponent();
    ~SharedAudioComponent() override;

    //==============================================================================
    // Component interface
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    //==============================================================================
    // Audio engine integration
    void setAudioEngine(AudioEngine* engine);

    // Parameter control (for standalone mode)
    void setVolumeSliderCallback(std::function<void(float)> callback);
    void setFrequencySliderCallback(std::function<void(float)> callback);
    void setWaveTypeCallback(std::function<void(int)> callback);

    // Parameter attachments (for plugin mode)
    void attachToParameters(juce::AudioProcessorValueTreeState& parameters);

    // Manual parameter updates (for standalone mode)
    void updateVolumeDisplay(float volume);
    void updateFrequencyDisplay(float frequency);
    void updateWaveTypeDisplay(int waveType);

private:
    //==============================================================================
    // GUI Components
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Slider frequencySlider;
    juce::Label frequencyLabel;
    juce::ComboBox waveTypeCombo;
    juce::Label waveTypeLabel;
    juce::Label statusLabel;
    juce::Label titleLabel;

    // Audio engine reference
    AudioEngine* audioEngine = nullptr;

    // Callbacks for standalone mode
    std::function<void(float)> volumeCallback;
    std::function<void(float)> frequencyCallback;
    std::function<void(int)> waveTypeCallback;

    // Parameter attachments for plugin mode
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveTypeAttachment;

    // Helper methods
    void setupComponents();
    void setupStyling();
    void updateStatus();

    // Slider listeners for standalone mode
    void sliderValueChanged(juce::Slider* slider);
    void comboBoxChanged(juce::ComboBox* comboBox);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SharedAudioComponent)
};

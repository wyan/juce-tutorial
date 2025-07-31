#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

#if TARGET_AUV3

#include "../iOS/TouchInterface.h"
#include "../SharedComponents/ParameterManager.h"

//==============================================================================
/**
 * AUv3Editor - iOS/macOS optimized editor for AUv3 plugins
 */
class AUv3Editor : public juce::AudioProcessorEditor,
                   public juce::Timer,
                   public iOS::TouchInterface::Listener
{
public:
    AUv3Editor(JUCEAudioGeneratorProcessor& processor);
    ~AUv3Editor() override;

    // AudioProcessorEditor interface
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Timer for efficient updates
    void timerCallback() override;

    // Touch interface support
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // TouchInterface::Listener
    void touchGestureDetected(const iOS::TouchInterface::TouchGesture& gesture) override;

    // iOS-specific lifecycle
    void viewWillAppear();
    void viewDidDisappear();
    void handleMemoryWarning();
    void handleOrientationChange();

private:
    JUCEAudioGeneratorProcessor& audioProcessor;

    // Touch-optimized components
    std::unique_ptr<iOS::TouchSlider> volumeSlider;
    std::unique_ptr<iOS::TouchSlider> frequencySlider;
    std::unique_ptr<iOS::TouchButton> waveTypeButton;

    // Labels
    std::unique_ptr<juce::Label> volumeLabel;
    std::unique_ptr<juce::Label> frequencyLabel;
    std::unique_ptr<juce::Label> waveTypeLabel;
    std::unique_ptr<juce::Label> titleLabel;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;

    // Touch interface
    iOS::TouchInterface touchInterface;

    // Mobile-specific features
    bool isTouchDevice = false;
    float touchScaleFactor = 1.0f;
    juce::Point<float> lastTouchPosition;

    // Performance optimization
    bool isVisible = false;
    int updateCounter = 0;

    // Current wave type
    int currentWaveType = 0;

    // Layout management
    void layoutForOrientation();
    void updateTouchScaling();
    void setupComponents();
    void setupParameterAttachments();
    void setupStyling();

    // iOS integration
    void setupiOSSpecificFeatures();
    void updateWaveTypeDisplay();

    // Gesture handling
    void handleTapGesture(const juce::Point<float>& position);
    void handlePinchGesture(float scale, const juce::Point<float>& center);
    void handleSwipeGesture(const juce::Point<float>& direction);

    // Accessibility
    void setupAccessibility();
    void announceParameterChange(const juce::String& parameterName, float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AUv3Editor)
};

#endif // TARGET_AUV3

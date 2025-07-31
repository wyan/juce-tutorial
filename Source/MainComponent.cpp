#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Initialize audio engine
    audioEngine = std::make_unique<AudioEngine>();

    setupComponents();
    setupStyling();

    // Initialize audio
    if (audioEngine->initializeAudio())
    {
        DBG("Audio initialized successfully");
    }
    else
    {
        DBG("Failed to initialize audio");
    }

    // Start timer for status updates
    startTimer(100);  // Update every 100ms

    setSize (800, 700);  // Increased height for audio status
}

MainComponent::~MainComponent()
{
    stopTimer();
    audioEngine.reset();
}

void MainComponent::setupComponents()
{
    // Title Label
    titleLabel.setText("JUCE Audio Generator", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(24.0f).withStyle("Bold")));
    addAndMakeVisible(titleLabel);

    // Volume Slider and Label
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(currentVolume, juce::dontSendNotification);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);

    // Play/Stop Button
    playStopButton.setButtonText("Play");
    playStopButton.onClick = [this]() { handlePlayStopButton(); };
    addAndMakeVisible(playStopButton);

    // Frequency ComboBox and Label
    frequencyCombo.addItem("440 Hz (A4)", 1);
    frequencyCombo.addItem("523 Hz (C5)", 2);
    frequencyCombo.addItem("659 Hz (E5)", 3);
    frequencyCombo.addItem("784 Hz (G5)", 4);
    frequencyCombo.setSelectedId(1, juce::dontSendNotification);
    frequencyCombo.addListener(this);
    addAndMakeVisible(frequencyCombo);

    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(frequencyLabel);

    // Audio Status Label
    audioStatusLabel.setText("Audio Status", juce::dontSendNotification);
    audioStatusLabel.setJustificationType(juce::Justification::centredLeft);
    audioStatusLabel.setFont(juce::Font(juce::FontOptions().withHeight(16.0f).withStyle("Bold")));
    addAndMakeVisible(audioStatusLabel);

    // Status Editor
    statusEditor.setMultiLine(true);
    statusEditor.setReadOnly(true);
    statusEditor.setScrollbarsShown(true);
    statusEditor.setCaretVisible(false);
    statusEditor.setPopupMenuEnabled(false);
    addAndMakeVisible(statusEditor);

    updateStatus();
}

void MainComponent::setupStyling()
{
    // Modern color scheme
    auto darkBackground = juce::Colour(0xff2d3748);
    auto accentColor = juce::Colour(0xff4299e1);
    auto textColor = juce::Colours::white;

    // Title styling
    titleLabel.setColour(juce::Label::textColourId, textColor);

    // Button styling
    playStopButton.setColour(juce::TextButton::buttonColourId, accentColor);
    playStopButton.setColour(juce::TextButton::textColourOffId, textColor);

    // Slider styling
    volumeSlider.setColour(juce::Slider::thumbColourId, accentColor);
    volumeSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
    volumeSlider.setColour(juce::Slider::backgroundColourId, darkBackground);

    // Labels styling
    volumeLabel.setColour(juce::Label::textColourId, textColor);
    frequencyLabel.setColour(juce::Label::textColourId, textColor);

    // ComboBox styling
    frequencyCombo.setColour(juce::ComboBox::backgroundColourId, darkBackground);
    frequencyCombo.setColour(juce::ComboBox::textColourId, textColor);
    frequencyCombo.setColour(juce::ComboBox::outlineColourId, accentColor);

    // Status editor styling
    statusEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    statusEditor.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    statusEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
}

void MainComponent::paint (juce::Graphics& g)
{
    // Gradient background
    juce::ColourGradient gradient(juce::Colour(0xff1a202c), 0, 0,
                                 juce::Colour(0xff2d3748), 0, getHeight(),
                                 false);
    g.setGradientFill(gradient);
    g.fillAll();
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Title area
    auto titleArea = bounds.removeFromTop(60);
    titleLabel.setBounds(titleArea);

    bounds.removeFromTop(10); // Spacing

    // Control rows
    const int rowHeight = 50;
    const int labelWidth = 100;
    const int spacing = 10;

    // Volume row
    auto volumeRow = bounds.removeFromTop(rowHeight);
    volumeLabel.setBounds(volumeRow.removeFromLeft(labelWidth));
    volumeRow.removeFromLeft(spacing);
    volumeSlider.setBounds(volumeRow);

    bounds.removeFromTop(spacing);

    // Button row
    auto buttonRow = bounds.removeFromTop(rowHeight);
    playStopButton.setBounds(buttonRow.removeFromLeft(200));

    bounds.removeFromTop(spacing);

    // Frequency row
    auto frequencyRow = bounds.removeFromTop(rowHeight);
    frequencyLabel.setBounds(frequencyRow.removeFromLeft(labelWidth));
    frequencyRow.removeFromLeft(spacing);
    frequencyCombo.setBounds(frequencyRow.removeFromLeft(200));

    bounds.removeFromTop(spacing);

    // Audio status area
    auto audioStatusArea = bounds.removeFromTop(100);
    audioStatusLabel.setBounds(audioStatusArea);

    bounds.removeFromTop(spacing);

    // Status area (remaining space)
    statusEditor.setBounds(bounds);
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        currentVolume = slider->getValue();
        audioEngine->setVolume(static_cast<float>(currentVolume));
        updateStatus();
    }
}

void MainComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &frequencyCombo)
    {
        handleFrequencyChange();
    }
}

void MainComponent::handlePlayStopButton()
{
    isPlaying = !isPlaying;
    playStopButton.setButtonText(isPlaying ? "Stop" : "Play");

    // Change button color based on state
    auto color = isPlaying ? juce::Colours::red : juce::Colour(0xff4299e1);
    playStopButton.setColour(juce::TextButton::buttonColourId, color);

    // Update audio engine
    audioEngine->setPlaying(isPlaying);

    updateStatus();
}

void MainComponent::handleFrequencyChange()
{
    auto selectedId = frequencyCombo.getSelectedId();
    switch (selectedId)
    {
        case 1: selectedFrequency = 440; break;
        case 2: selectedFrequency = 523; break;
        case 3: selectedFrequency = 659; break;
        case 4: selectedFrequency = 784; break;
        default: selectedFrequency = 440; break;
    }

    audioEngine->setFrequency(static_cast<float>(selectedFrequency));
    updateStatus();
}

void MainComponent::timerCallback()
{
    updateAudioStatus();
}

void MainComponent::updateStatus()
{
    juce::String status;
    status << "=== JUCE Audio Generator Status ===\n\n";
    status << "State: " << (isPlaying ? "PLAYING" : "STOPPED") << "\n";
    status << "Volume: " << juce::String(currentVolume, 2) << " ("
           << juce::String(int(currentVolume * 100)) << "%)\n";
    status << "Frequency: " << juce::String(selectedFrequency) << " Hz\n";
    status << "Components: " << getNumChildComponents() << " active\n\n";

    if (isPlaying)
    {
        status << "♪ Sine wave generation active\n";
        status << "♪ Frequency: " << selectedFrequency << " Hz\n";
        status << "♪ Amplitude: " << juce::String(currentVolume, 3) << "\n";
        status << "♪ Real-time audio processing enabled\n";
    }
    else
    {
        status << "⏸ Audio generation stopped\n";
        status << "Ready to generate " << selectedFrequency << " Hz sine wave\n";
    }

    status << "\nLast updated: " << juce::Time::getCurrentTime().toString(true, true);

    statusEditor.setText(status, juce::dontSendNotification);
}

void MainComponent::updateAudioStatus()
{
    if (audioEngine)
    {
        auto audioStatus = audioEngine->getAudioDeviceStatus();
        audioStatusLabel.setText("Audio Device Status:\n" + audioStatus, juce::dontSendNotification);
    }
}

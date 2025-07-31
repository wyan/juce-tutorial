#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JUCEAudioGeneratorEditor::JUCEAudioGeneratorEditor (JUCEAudioGeneratorProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setupComponents();
    setupStyling();

    // Create parameter attachments for automatic synchronization
    auto& params = audioProcessor.getParameters();
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "volume", volumeSlider);
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "frequency", frequencySlider);
    waveTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, "waveType", waveTypeCombo);

    // Set editor size
    setSize (400, 300);

    // Start timer for GUI updates
    startTimer(50);  // 20 FPS updates
}

JUCEAudioGeneratorEditor::~JUCEAudioGeneratorEditor()
{
    stopTimer();
}

//==============================================================================
void JUCEAudioGeneratorEditor::paint (juce::Graphics& g)
{
    // Gradient background
    juce::ColourGradient gradient(juce::Colour(0xff1a202c), 0, 0,
                                 juce::Colour(0xff2d3748), 0, getHeight(),
                                 false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("JUCE Audio Generator", getLocalBounds().removeFromTop(40),
               juce::Justification::centred);
}

void JUCEAudioGeneratorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    bounds.removeFromTop(40);  // Title space

    const int rowHeight = 40;
    const int labelWidth = 100;
    const int spacing = 10;

    // Volume row
    auto volumeRow = bounds.removeFromTop(rowHeight);
    volumeLabel.setBounds(volumeRow.removeFromLeft(labelWidth));
    volumeRow.removeFromLeft(spacing);
    volumeSlider.setBounds(volumeRow);

    bounds.removeFromTop(spacing);

    // Frequency row
    auto frequencyRow = bounds.removeFromTop(rowHeight);
    frequencyLabel.setBounds(frequencyRow.removeFromLeft(labelWidth));
    frequencyRow.removeFromLeft(spacing);
    frequencySlider.setBounds(frequencyRow);

    bounds.removeFromTop(spacing);

    // Wave type row
    auto waveTypeRow = bounds.removeFromTop(rowHeight);
    waveTypeLabel.setBounds(waveTypeRow.removeFromLeft(labelWidth));
    waveTypeRow.removeFromLeft(spacing);
    waveTypeCombo.setBounds(waveTypeRow.removeFromLeft(150));

    bounds.removeFromTop(spacing * 2);

    // Status label
    statusLabel.setBounds(bounds.removeFromTop(30));
}

void JUCEAudioGeneratorEditor::timerCallback()
{
    updateStatus();
}

//==============================================================================
void JUCEAudioGeneratorEditor::setupComponents()
{
    // Volume slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);

    // Frequency slider
    frequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    frequencySlider.setSkewFactorFromMidPoint(1000.0);  // Logarithmic feel
    addAndMakeVisible(frequencySlider);

    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(frequencyLabel);

    // Wave type combo
    waveTypeCombo.addItem("Sine", 1);
    waveTypeCombo.addItem("Square", 2);
    waveTypeCombo.addItem("Sawtooth", 3);
    waveTypeCombo.addItem("Triangle", 4);
    addAndMakeVisible(waveTypeCombo);

    waveTypeLabel.setText("Wave Type", juce::dontSendNotification);
    waveTypeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(waveTypeLabel);

    // Status label
    statusLabel.setText("Plugin Status: Active", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
}

void JUCEAudioGeneratorEditor::setupStyling()
{
    auto darkBackground = juce::Colour(0xff2d3748);
    auto accentColor = juce::Colour(0xff4299e1);
    auto textColor = juce::Colours::white;

    // Slider styling
    volumeSlider.setColour(juce::Slider::thumbColourId, accentColor);
    volumeSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);

    frequencySlider.setColour(juce::Slider::thumbColourId, accentColor);
    frequencySlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);

    // Label styling
    volumeLabel.setColour(juce::Label::textColourId, textColor);
    frequencyLabel.setColour(juce::Label::textColourId, textColor);
    waveTypeLabel.setColour(juce::Label::textColourId, textColor);
    statusLabel.setColour(juce::Label::textColourId, textColor);

    // ComboBox styling
    waveTypeCombo.setColour(juce::ComboBox::backgroundColourId, darkBackground);
    waveTypeCombo.setColour(juce::ComboBox::textColourId, textColor);
    waveTypeCombo.setColour(juce::ComboBox::outlineColourId, accentColor);
}

void JUCEAudioGeneratorEditor::updateStatus()
{
    auto& engine = audioProcessor.getAudioEngine();
    auto volume = engine.getCurrentVolume();
    auto frequency = engine.getCurrentFrequency();

    juce::String status = "Plugin Active - Vol: ";
    status << juce::String(volume, 2) << " | Freq: " << juce::String(frequency, 0) << " Hz";

    statusLabel.setText(status, juce::dontSendNotification);
}

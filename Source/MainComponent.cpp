#include "MainComponent.h"

MainComponent::MainComponent()
{
    helloLabel.setText ("Hello JUCE!", juce::dontSendNotification);
    helloLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (helloLabel);
    setSize (400, 200);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    helloLabel.setBounds (getLocalBounds());
}

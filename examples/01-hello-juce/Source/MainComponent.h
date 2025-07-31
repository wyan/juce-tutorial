/*
  ==============================================================================

    MainComponent.h
    Created: Hello JUCE Example
    Author:  JUCE Tutorial Series

    This file contains the main GUI component for the Hello JUCE example.
    It demonstrates basic JUCE component concepts including custom drawing,
    event handling, and layout management.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Main Component Class
 *
 * This is the main GUI component that gets displayed in the application window.
 * It demonstrates fundamental JUCE concepts:
 * - Custom drawing with the paint() method
 * - Layout management with the resized() method
 * - Event handling with mouse interaction
 * - Component lifecycle management
 */
class MainComponent : public juce::Component
{
public:
    //==============================================================================
    /**
     * Constructor
     *
     * Initialize the component and set its initial size and properties.
     */
    MainComponent();

    /**
     * Destructor
     *
     * Clean up any resources. In this simple example, JUCE handles
     * most cleanup automatically.
     */
    ~MainComponent() override;

    //==============================================================================
    /**
     * Paint Method
     *
     * This is where all custom drawing happens. JUCE calls this method
     * whenever the component needs to be redrawn (e.g., when first shown,
     * after calling repaint(), when window is resized, etc.)
     *
     * @param g Graphics context for drawing operations
     */
    void paint(juce::Graphics& g) override;

    /**
     * Resized Method
     *
     * This is called whenever the component's size changes. Use this method
     * to position and size child components. In this example, we don't have
     * child components, but we could use this to adjust layout.
     */
    void resized() override;

    //==============================================================================
    /**
     * Mouse Down Event Handler
     *
     * Called when a mouse button is pressed down over this component.
     * We use this to change the background color when clicked.
     *
     * @param event Mouse event information (position, button, modifiers, etc.)
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * Mouse Enter Event Handler
     *
     * Called when the mouse cursor enters this component's area.
     * We use this to show that the component is interactive.
     *
     * @param event Mouse event information
     */
    void mouseEnter(const juce::MouseEvent& event) override;

    /**
     * Mouse Exit Event Handler
     *
     * Called when the mouse cursor leaves this component's area.
     *
     * @param event Mouse event information
     */
    void mouseExit(const juce::MouseEvent& event) override;

private:
    //==============================================================================
    // Private member variables

    /**
     * Display Text
     * The text that gets drawn in the center of the component
     */
    juce::String displayText;

    /**
     * Background Colour
     * The current background color, changes when clicked
     */
    juce::Colour backgroundColour;

    /**
     * Mouse Over State
     * Tracks whether the mouse is currently over the component
     */
    bool isMouseOver;

    /**
     * Click Count
     * Keeps track of how many times the component has been clicked
     */
    int clickCount;

    //==============================================================================
    /**
     * Generate Random Color
     *
     * Helper method to generate a random color for the background.
     * Uses HSV color space to ensure colors are vibrant and pleasant.
     *
     * @return A randomly generated color
     */
    juce::Colour generateRandomColour();

    /**
     * Update Display Text
     *
     * Helper method to update the display text based on current state.
     * Shows different messages based on interaction state.
     */
    void updateDisplayText();

    //==============================================================================
    // JUCE leak detector - helps catch memory leaks during development
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//==============================================================================
// Inline Implementation (for simple methods)

inline MainComponent::MainComponent()
    : displayText("Hello, JUCE World!"),
      backgroundColour(juce::Colour::fromRGB(50, 50, 80)),
      isMouseOver(false),
      clickCount(0)
{
    // Set the initial size of the component
    // This will be the default window size
    setSize(600, 400);

    // Enable mouse events for this component
    setWantsKeyboardFocus(false);  // We don't need keyboard focus for this example
}

inline MainComponent::~MainComponent()
{
    // Destructor - cleanup happens automatically for this simple example
}

inline void MainComponent::paint(juce::Graphics& g)
{
    // Fill the background with our current background color
    g.fillAll(backgroundColour);

    // If mouse is over the component, add a subtle highlight
    if (isMouseOver)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillAll();
    }

    // Draw the main text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(28.0f, juce::Font::bold));

    // Calculate text area (leave some margin)
    auto textArea = getLocalBounds().reduced(20);
    g.drawText(displayText, textArea, juce::Justification::centred, true);

    // Draw additional info text
    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white.withAlpha(0.7f));

    auto infoText = juce::String("Click count: ") + juce::String(clickCount) +
                   juce::String("\nClick anywhere to change color!");

    auto infoArea = getLocalBounds().removeFromBottom(80).reduced(20);
    g.drawText(infoText, infoArea, juce::Justification::centred, true);
}

inline void MainComponent::resized()
{
    // This method is called when the component's size changes
    // In this simple example, we don't have child components to position,
    // but this is where you would typically set the bounds of child components

    // Example of how you might position child components:
    // auto area = getLocalBounds();
    // childComponent.setBounds(area.removeFromTop(50));
    // anotherChild.setBounds(area.reduced(10));
}

inline void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    // Increment click count
    clickCount++;

    // Generate a new random background color
    backgroundColour = generateRandomColour();

    // Update the display text
    updateDisplayText();

    // Trigger a repaint to show the changes
    repaint();

    // You could also play a sound, trigger animations, etc.
    DBG("Component clicked at position: " << event.position.toString());
}

inline void MainComponent::mouseEnter(const juce::MouseEvent& event)
{
    isMouseOver = true;
    repaint();  // Repaint to show the hover effect
}

inline void MainComponent::mouseExit(const juce::MouseEvent& event)
{
    isMouseOver = false;
    repaint();  // Repaint to remove the hover effect
}

inline juce::Colour MainComponent::generateRandomColour()
{
    // Generate a random color using HSV color space
    // This ensures colors are vibrant and visually appealing
    auto& random = juce::Random::getSystemRandom();

    float hue = random.nextFloat();           // Random hue (0-1)
    float saturation = 0.6f + random.nextFloat() * 0.3f;  // Saturation (0.6-0.9)
    float brightness = 0.4f + random.nextFloat() * 0.4f;  // Brightness (0.4-0.8)

    return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
}

inline void MainComponent::updateDisplayText()
{
    // Update display text based on click count
    if (clickCount == 0)
    {
        displayText = "Hello, JUCE World!";
    }
    else if (clickCount == 1)
    {
        displayText = "Nice click! Try clicking again...";
    }
    else if (clickCount < 5)
    {
        displayText = "Keep clicking! (" + juce::String(clickCount) + " clicks)";
    }
    else if (clickCount < 10)
    {
        displayText = "You're getting the hang of this!";
    }
    else
    {
        displayText = "JUCE Master! 🎵";
    }
}

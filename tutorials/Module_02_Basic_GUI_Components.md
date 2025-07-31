# Module 2: Building Basic GUI Components

## Learning Objectives

By the end of this module, you will:
- Master JUCE's core GUI components (Button, Slider, ComboBox, TextEditor)
- Understand event handling and listener patterns
- Learn layout management and component positioning
- Implement custom component styling and Look & Feel
- Introduction to C++14 features (auto return types, generic lambdas)

## Table of Contents

1. [JUCE GUI Component Overview](#juce-gui-component-overview)
2. [Event Handling and Listener Patterns](#event-handling-and-listener-patterns)
3. [Core GUI Components](#core-gui-components)
4. [Layout Management Techniques](#layout-management-techniques)
5. [Custom Styling and Look & Feel](#custom-styling-and-look--feel)
6. [C++14 Features in GUI Context](#c14-features-in-gui-context)
7. [Practical Implementation](#practical-implementation)
8. [Practical Exercises](#practical-exercises)
9. [Code Examples and Best Practices](#code-examples-and-best-practices)

---

## JUCE GUI Component Overview

### Component Hierarchy

JUCE's GUI system is built around a hierarchical component model where every visual element inherits from [`juce::Component`](Source/MainComponent.h:5). This creates a tree structure where:

- Parent components contain and manage child components
- Events bubble up from child to parent
- Coordinate systems are relative to the parent
- Memory management is automatic (parents own children)

```cpp
juce::Component
├── juce::Button
│   ├── juce::TextButton
│   ├── juce::ToggleButton
│   └── juce::ImageButton
├── juce::Slider
├── juce::ComboBox
├── juce::TextEditor
├── juce::Label
└── Your Custom Components
```

### Component Lifecycle Revisited

Building on Module 1, let's dive deeper into the component lifecycle:

1. **Construction**: Initialize member variables, set default properties
2. **Setup**: Add child components with [`addAndMakeVisible()`](Source/MainComponent.cpp:7)
3. **Visibility**: Component becomes visible and starts receiving events
4. **Layout**: [`resized()`](Source/MainComponent.cpp:16) called when size changes
5. **Painting**: [`paint()`](Source/MainComponent.cpp:11) called for custom drawing
6. **Event Processing**: Mouse, keyboard, and other events handled
7. **Destruction**: Automatic cleanup of child components

### Key Virtual Methods for GUI Components

```cpp
class InteractiveComponent : public juce::Component
{
public:
    // Core lifecycle methods
    void paint (juce::Graphics& g) override;
    void resized() override;

    // Mouse interaction
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    // Keyboard interaction
    bool keyPressed (const juce::KeyPress& key) override;
    void focusGained (FocusChangeType cause) override;
    void focusLost (FocusChangeType cause) override;
};
```

---

## Event Handling and Listener Patterns

### The Listener Pattern in JUCE

JUCE uses the Observer pattern (called "Listener" pattern) for event handling. This decouples event sources from event handlers, making code more modular and maintainable.

### Button Listener Pattern

```cpp
class MainComponent : public juce::Component,
                     public juce::Button::Listener
{
public:
    MainComponent()
    {
        playButton.setButtonText("Play");
        playButton.addListener(this);  // Register as listener
        addAndMakeVisible(playButton);
    }

    // Button::Listener interface implementation
    void buttonClicked(juce::Button* button) override
    {
        if (button == &playButton)
        {
            handlePlayButtonClick();
        }
    }

private:
    void handlePlayButtonClick()
    {
        isPlaying = !isPlaying;
        playButton.setButtonText(isPlaying ? "Stop" : "Play");
        updateStatus();
    }

    juce::TextButton playButton;
    bool isPlaying = false;
};
```

### Modern C++ Lambda Approach

C++11 introduced lambdas, which provide a more concise way to handle events:

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        playButton.setButtonText("Play");

        // Lambda callback - captures 'this' to access member variables
        playButton.onClick = [this]()
        {
            isPlaying = !isPlaying;
            playButton.setButtonText(isPlaying ? "Stop" : "Play");
            statusEditor.setText("Status: " + juce::String(isPlaying ? "Playing" : "Stopped"));
        };

        addAndMakeVisible(playButton);
    }

private:
    juce::TextButton playButton;
    juce::TextEditor statusEditor;
    bool isPlaying = false;
};
```

### Slider Listener Pattern

```cpp
class MainComponent : public juce::Component,
                     public juce::Slider::Listener
{
public:
    MainComponent()
    {
        volumeSlider.setRange(0.0, 1.0, 0.01);
        volumeSlider.setValue(0.5);
        volumeSlider.addListener(this);
        addAndMakeVisible(volumeSlider);
    }

    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &volumeSlider)
        {
            auto volume = slider->getValue();
            volumeLabel.setText("Volume: " + juce::String(volume, 2),
                              juce::dontSendNotification);
        }
    }

private:
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
};
```

---

## Core GUI Components

### 1. Buttons

JUCE provides several button types for different use cases:

#### TextButton
```cpp
juce::TextButton playButton;

// Setup
playButton.setButtonText("Play");
playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

// Event handling with lambda
playButton.onClick = [this]() { handlePlayButton(); };
```

#### ToggleButton
```cpp
juce::ToggleButton muteButton;

// Setup
muteButton.setButtonText("Mute");
muteButton.setToggleState(false, juce::dontSendNotification);

// Event handling
muteButton.onClick = [this]()
{
    bool isMuted = muteButton.getToggleState();
    volumeSlider.setEnabled(!isMuted);
    updateAudioMuting(isMuted);
};
```

### 2. Sliders

Sliders are perfect for continuous value input:

```cpp
juce::Slider volumeSlider;
juce::Label volumeLabel;

// Setup in constructor
volumeSlider.setRange(0.0, 1.0, 0.01);           // min, max, step
volumeSlider.setValue(0.5, juce::dontSendNotification);
volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

// Lambda event handling
volumeSlider.onValueChange = [this]()
{
    auto volume = volumeSlider.getValue();
    volumeLabel.setText("Volume: " + juce::String(volume, 2),
                       juce::dontSendNotification);
};
```

#### Slider Styles
- `LinearHorizontal` / `LinearVertical`: Traditional linear sliders
- `Rotary`: Circular knob-style slider
- `RotaryHorizontalDrag` / `RotaryVerticalDrag`: Rotary with drag direction
- `IncDecButtons`: Increment/decrement buttons

### 3. ComboBox

ComboBoxes provide dropdown selection:

```cpp
juce::ComboBox frequencyCombo;

// Setup
frequencyCombo.addItem("440 Hz (A4)", 1);
frequencyCombo.addItem("523 Hz (C5)", 2);
frequencyCombo.addItem("659 Hz (E5)", 3);
frequencyCombo.addItem("784 Hz (G5)", 4);
frequencyCombo.setSelectedId(1, juce::dontSendNotification);

// Event handling
frequencyCombo.onChange = [this]()
{
    auto selectedId = frequencyCombo.getSelectedId();
    handleFrequencyChange(selectedId);
};
```

### 4. TextEditor

TextEditor provides text input and display:

```cpp
juce::TextEditor statusEditor;

// Setup for read-only status display
statusEditor.setMultiLine(true);
statusEditor.setReadOnly(true);
statusEditor.setScrollbarsShown(true);
statusEditor.setCaretVisible(false);
statusEditor.setPopupMenuEnabled(false);
statusEditor.setText("Ready...");

// Setup for user input
juce::TextEditor userInput;
userInput.setMultiLine(false);
userInput.setReturnKeyStartsNewLine(false);
userInput.onReturnKey = [this]() { handleUserInput(); };
```

### 5. Labels

Labels display text and can be made editable:

```cpp
juce::Label titleLabel;
juce::Label volumeLabel;

// Static label
titleLabel.setText("Audio Control Panel", juce::dontSendNotification);
titleLabel.setJustificationType(juce::Justification::centred);
titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));

// Dynamic label (updates with slider)
volumeLabel.setText("Volume: 0.50", juce::dontSendNotification);
volumeLabel.setJustificationType(juce::Justification::centredRight);
```

---

## Layout Management Techniques

### Understanding Bounds and Rectangles

JUCE uses [`juce::Rectangle<int>`](Source/MainComponent.cpp:18) for positioning and sizing:

```cpp
void MainComponent::resized()
{
    auto bounds = getLocalBounds();  // Get component's full area

    // Remove areas from bounds (modifies original)
    auto headerArea = bounds.removeFromTop(60);
    auto footerArea = bounds.removeFromBottom(40);
    auto leftPanel = bounds.removeFromLeft(200);
    auto rightPanel = bounds;  // Remaining area

    // Position components
    titleLabel.setBounds(headerArea.reduced(10));
    statusEditor.setBounds(footerArea.reduced(5));
}
```

### Layout Strategies

#### 1. Proportional Layout
```cpp
void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Split into thirds
    auto topThird = bounds.removeFromTop(bounds.getHeight() / 3);
    auto middleThird = bounds.removeFromTop(bounds.getHeight() / 2);  // Half of remaining
    auto bottomThird = bounds;  // Rest

    // Position components proportionally
    playButton.setBounds(topThird.reduced(20));
    volumeSlider.setBounds(middleThird.reduced(20));
    statusEditor.setBounds(bottomThird.reduced(20));
}
```

#### 2. Grid Layout
```cpp
void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    const int rows = 3;
    const int cols = 2;
    const int cellWidth = bounds.getWidth() / cols;
    const int cellHeight = bounds.getHeight() / rows;

    // Position in grid
    playButton.setBounds(0 * cellWidth, 0 * cellHeight, cellWidth, cellHeight);
    volumeSlider.setBounds(1 * cellWidth, 0 * cellHeight, cellWidth, cellHeight);
    frequencyCombo.setBounds(0 * cellWidth, 1 * cellHeight, cellWidth, cellHeight);
    statusEditor.setBounds(0 * cellWidth, 2 * cellHeight, cellWidth * 2, cellHeight);
}
```

#### 3. FlexBox Layout (Advanced)
```cpp
void MainComponent::resized()
{
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

    flexBox.items.add(juce::FlexItem(playButton).withMinHeight(50).withFlex(1));
    flexBox.items.add(juce::FlexItem(volumeSlider).withMinHeight(50).withFlex(1));
    flexBox.items.add(juce::FlexItem(frequencyCombo).withMinHeight(50).withFlex(1));
    flexBox.items.add(juce::FlexItem(statusEditor).withMinHeight(100).withFlex(2));

    flexBox.performLayout(getLocalBounds().reduced(10));
}
```

---

## Custom Styling and Look & Feel

### Component Colors

Every JUCE component has customizable color properties:

```cpp
// Button colors
playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
playButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

// Slider colors
volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
volumeSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
volumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);

// TextEditor colors
statusEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
statusEditor.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
statusEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
```

### Custom Look & Feel Class

For more advanced styling, create a custom Look & Feel:

```cpp
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel()
    {
        // Set default colors
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d3748));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff4299e1));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

        auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }
};

// Usage in MainComponent
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        setLookAndFeel(&modernLookAndFeel);
        // ... setup components
    }

    ~MainComponent() override
    {
        setLookAndFeel(nullptr);
    }

private:
    ModernLookAndFeel modernLookAndFeel;
};
```

---

## C++14 Features in GUI Context

### Auto Return Types

C++14 allows `auto` return types with type deduction:

```cpp
class MainComponent : public juce::Component
{
private:
    // C++14: auto return type deduced from return statement
    auto createStyledButton(const juce::String& text) -> std::unique_ptr<juce::TextButton>
    {
        auto button = std::make_unique<juce::TextButton>(text);
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        return button;
    }

    // Auto return type for layout calculations
    auto calculateButtonBounds(int index, int totalButtons) const
    {
        auto bounds = getLocalBounds();
        auto buttonHeight = bounds.getHeight() / totalButtons;
        return bounds.removeFromTop(buttonHeight).reduced(5);
    }
};
```

### Generic Lambdas

C++14 introduced generic lambdas with `auto` parameters:

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        // Generic lambda that works with any component type
        auto setupComponent = [this](auto& component, const juce::String& name)
        {
            component.setComponentID(name);
            addAndMakeVisible(component);

            // Log component setup
            DBG("Setup component: " + name);
        };

        // Use with different component types
        setupComponent(playButton, "PlayButton");
        setupComponent(volumeSlider, "VolumeSlider");
        setupComponent(frequencyCombo, "FrequencyCombo");

        // Generic event handler
        auto createButtonHandler = [this](const juce::String& action)
        {
            return [this, action]()
            {
                statusEditor.setText("Action: " + action, juce::dontSendNotification);
                DBG("Button action: " + action);
            };
        };

        playButton.onClick = createButtonHandler("Play/Stop");
    }
};
```

### Improved Lambda Captures

C++14 allows init captures for more flexible lambda usage:

```cpp
// Capture by move (C++14)
auto createAsyncHandler = [component = std::move(someComponent)](const juce::String& message)
{
    // Use moved component
    component->setText(message, juce::dontSendNotification);
};

// Capture with initialization
auto createTimedCallback = [startTime = juce::Time::getCurrentTime()](int delay)
{
    auto elapsed = juce::Time::getCurrentTime() - startTime;
    DBG("Elapsed time: " + juce::String(elapsed.inSeconds()) + "s");
};
```

---

## Practical Implementation

Now let's transform our basic "Hullo JUCE!" application into an interactive control panel. We'll modify the existing [`MainComponent`](Source/MainComponent.h:5) to include:

1. Volume slider with label
2. Play/Stop toggle button
3. Frequency selection combo box
4. Status text editor (read-only)
5. Custom styling and layout

### Updated MainComponent.h

```cpp
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
```

### Updated MainComponent.cpp

```cpp
#include "MainComponent.h"

MainComponent::MainComponent()
{
    setupComponents();
    setupStyling();
    setSize (800, 600);
}

void MainComponent::setupComponents()
{
    // Title Label
    titleLabel.setText("JUCE Audio Control Panel", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
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

    // Status area (remaining space)
    statusEditor.setBounds(bounds);
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        currentVolume = slider->getValue();
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
    updateStatus();
}

void MainComponent::updateStatus()
{
    juce::String status;
    status << "=== JUCE Audio Control Panel Status ===\n\n";
    status << "State: " << (isPlaying ? "PLAYING" : "STOPPED") << "\n";
    status << "Volume: " << juce::String(currentVolume, 2) << " ("
           << juce::String(int(currentVolume * 100)) << "%)\n";
    status << "Frequency: " << juce::String(selectedFrequency) << " Hz\n";
    status << "Components: " << getNumChildComponents() << " active\n\n";

    if (isPlaying)
    {
        status << "♪ Audio generation active\n";
        status << "♪ Frequency: " << selectedFrequency << " Hz sine wave\n";
        status << "♪ Amplitude: " << juce::String(currentVolume, 3) << "\n";
    }
    else
    {
        status << "⏸ Audio generation stopped\n";
        status << "Ready to play at " << selectedFrequency << " Hz\n";
    }

    status << "\nLast updated: " << juce::Time::getCurrentTime().toString(true, true);

    statusEditor.setText(status, juce::dontSendNotification);
}
```

---

## Practical Exercises

### Exercise 1: Add and Style Basic Components

**Objective**: Implement the basic GUI components and understand their properties.

**Steps**:
1. Update your `MainComponent.h` to include the new member variables
2. Implement the component setup in the constructor
3. Add basic styling to make components visually appealing
4. Build and run to see the new interface

**Expected Result**: A window with volume slider, play button, frequency combo box, and status display.

**Code Challenge**: Add a mute toggle button that disables the volume slider when active.

### Exercise 2: Implement Event Handling and Component Interactions

**Objective**: Make components interactive and responsive to user input.

**Steps**:
1. Implement the listener interfaces for slider and combo box
2. Add lambda callbacks for button interactions
3. Create the `updateStatus()` method to show current state
4. Test all interactions work correctly

**Expected Result**: Components respond to user input and status updates in real-time.

**Code Challenge**: Add keyboard shortcuts (spacebar for play/stop, up/down arrows for volume).

### Exercise 3: Create Custom Layout and Positioning

**Objective**: Master JUCE's layout system and create responsive designs.

**Steps**:
1. Implement a sophisticated `resized()` method
2. Use proportional sizing for different screen sizes
3. Add proper spacing and margins
4. Test layout at different window sizes

**Expected Result**: Components resize and reposition smoothly when window is resized.

windows.

### Exercise 4: Apply Custom Look & Feel Styling

**Objective**: Create a modern, professional appearance using custom styling.

**Steps**:
1. Implement the custom styling methods shown in the tutorial
2. Experiment with different color schemes
3. Add gradient backgrounds and rounded corners
4. Create hover effects for interactive elements

**Expected Result**: A polished, modern-looking interface with consistent styling.

**Code Challenge**: Create a dark/light theme toggle that switches the entire interface appearance.

---

## Code Examples and Best Practices

### Example 1: Advanced Event Handling with State Management

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        // Setup components with lambda callbacks
        setupButton(playButton, "Play", [this]() { togglePlayState(); });
        setupButton(recordButton, "Record", [this]() { toggleRecordState(); });
        setupButton(stopButton, "Stop", [this]() { stopAllOperations(); });

        // Disable conflicting operations initially
        updateButtonStates();
    }

private:
    void setupButton(juce::TextButton& button, const juce::String& text,
                    std::function<void()> callback)
    {
        button.setButtonText(text);
        button.onClick = callback;
        addAndMakeVisible(button);
    }

    void togglePlayState()
    {
        isPlaying = !isPlaying;
        updateButtonStates();
        updateStatus();
    }

    void updateButtonStates()
    {
        playButton.setButtonText(isPlaying ? "Pause" : "Play");
        playButton.setColour(juce::TextButton::buttonColourId,
                           isPlaying ? juce::Colours::orange : juce::Colours::green);

        recordButton.setEnabled(!isPlaying);  // Can't record while playing
        stopButton.setEnabled(isPlaying || isRecording);
    }

    juce::TextButton playButton, recordButton, stopButton;
    bool isPlaying = false, isRecording = false;
};
```

### Example 2: Responsive Layout with Breakpoints

```cpp
void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    const int width = bounds.getWidth();

    // Responsive breakpoints
    if (width < 600)
    {
        layoutMobile(bounds);
    }
    else if (width < 1000)
    {
        layoutTablet(bounds);
    }
    else
    {
        layoutDesktop(bounds);
    }
}

void MainComponent::layoutMobile(juce::Rectangle<int> bounds)
{
    // Single column layout for mobile
    const int componentHeight = 60;
    const int spacing = 10;

    titleLabel.setBounds(bounds.removeFromTop(componentHeight));
    bounds.removeFromTop(spacing);

    volumeSlider.setBounds(bounds.removeFromTop(componentHeight));
    bounds.removeFromTop(spacing);

    playStopButton.setBounds(bounds.removeFromTop(componentHeight));
    bounds.removeFromTop(spacing);

    frequencyCombo.setBounds(bounds.removeFromTop(componentHeight));
    bounds.removeFromTop(spacing);

    statusEditor.setBounds(bounds);
}

void MainComponent::layoutDesktop(juce::Rectangle<int> bounds)
{
    // Two-column layout for desktop
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds.reduced(10, 0);

    // Left column: controls
    titleLabel.setBounds(leftColumn.removeFromTop(60));
    leftColumn.removeFromTop(10);

    volumeSlider.setBounds(leftColumn.removeFromTop(50));
    leftColumn.removeFromTop(10);

    playStopButton.setBounds(leftColumn.removeFromTop(50));
    leftColumn.removeFromTop(10);

    frequencyCombo.setBounds(leftColumn.removeFromTop(50));

    // Right column: status
    statusEditor.setBounds(rightColumn);
}
```

### Example 3: Custom Component with Modern C++14 Features

```cpp
class ModernAudioControl : public juce::Component
{
public:
    ModernAudioControl()
    {
        // C++14: Generic lambda for component setup
        auto setupComponent = [this](auto& component, const auto& config)
        {
            component.setBounds(config.bounds);
            component.setColour(config.colourId, config.colour);
            addAndMakeVisible(component);
        };

        // C++14: Auto return type deduction
        auto createControlConfig = [](auto bounds, auto colourId, auto colour)
        {
            return ComponentConfig{bounds, colourId, colour};
        };

        // Setup components with modern syntax
        setupComponent(volumeSlider, createControlConfig(
            juce::Rectangle<int>(10, 10, 200, 30),
            juce::Slider::thumbColourId,
            juce::Colours::blue
        ));
    }

private:
    // C++14: Auto return type for layout calculations
    auto calculateOptimalLayout() const
    {
        struct LayoutInfo {
            int columns;
            int rowHeight;
            int spacing;
        };

        auto width = getWidth();
        if (width < 400) return LayoutInfo{1, 50, 5};
        if (width < 800) return LayoutInfo{2, 45, 8};
        return LayoutInfo{3, 40, 10};
    }

    struct ComponentConfig {
        juce::Rectangle<int> bounds;
        int colourId;
        juce::Colour colour;
    };

    juce::Slider volumeSlider;
};
```

### Example 4: Advanced Listener Pattern with Type Safety

```cpp
class SafeAudioComponent : public juce::Component,
                          private juce::Slider::Listener,
                          private juce::Button::Listener,
                          private juce::ComboBox::Listener
{
public:
    SafeAudioComponent()
    {
        // Register listeners with type safety
        registerComponent(volumeSlider, this);
        registerComponent(playButton, this);
        registerComponent(frequencyCombo, this);
    }

    ~SafeAudioComponent() override
    {
        // Automatic cleanup - listeners removed in component destructors
    }

private:
    // Template method for type-safe listener registration
    template<typename ComponentType, typename ListenerType>
    void registerComponent(ComponentType& component, ListenerType* listener)
    {
        component.addListener(listener);
        addAndMakeVisible(component);
    }

    // Listener implementations with clear component identification
    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &volumeSlider)
        {
            handleVolumeChange(slider->getValue());
        }
    }

    void buttonClicked(juce::Button* button) override
    {
        if (button == &playButton)
        {
            handlePlayButton();
        }
    }

    void comboBoxChanged(juce::ComboBox* comboBox) override
    {
        if (comboBox == &frequencyCombo)
        {
            handleFrequencyChange(comboBox->getSelectedId());
        }
    }

    void handleVolumeChange(double newVolume)
    {
        currentVolume = newVolume;
        updateAudioEngine();
        updateStatusDisplay();
    }

    void handlePlayButton()
    {
        isPlaying = !isPlaying;
        playButton.setButtonText(isPlaying ? "Stop" : "Play");
        updateAudioEngine();
    }

    void handleFrequencyChange(int selectedId)
    {
        // Map combo box selection to frequency values
        static const std::map<int, double> frequencyMap = {
            {1, 440.0}, {2, 523.25}, {3, 659.25}, {4, 783.99}
        };

        auto it = frequencyMap.find(selectedId);
        if (it != frequencyMap.end())
        {
            currentFrequency = it->second;
            updateAudioEngine();
        }
    }

    void updateAudioEngine()
    {
        // In a real application, this would update the audio processing
        DBG("Audio Update - Playing: " << isPlaying
            << ", Volume: " << currentVolume
            << ", Frequency: " << currentFrequency);
    }

    void updateStatusDisplay()
    {
        juce::String status = "Volume: " + juce::String(currentVolume, 2);
        statusLabel.setText(status, juce::dontSendNotification);
    }

    juce::Slider volumeSlider;
    juce::TextButton playButton;
    juce::ComboBox frequencyCombo;
    juce::Label statusLabel;

    bool isPlaying = false;
    double currentVolume = 0.5;
    double currentFrequency = 440.0;
};
```

---

## Summary

In this module, you've learned to create interactive JUCE applications with:

### Key Concepts Mastered

1. **GUI Component Hierarchy**: Understanding parent-child relationships and automatic memory management
2. **Event Handling**: Both traditional listener patterns and modern lambda approaches
3. **Layout Management**: Responsive design techniques using Rectangle manipulation
4. **Custom Styling**: Professional appearance with colors, fonts, and Look & Feel customization
5. **Modern C++14**: Auto return types, generic lambdas, and improved capture syntax

### Components Implemented

- **Volume Slider**: Continuous value input with real-time feedback
- **Play/Stop Button**: State management with visual feedback
- **Frequency ComboBox**: Discrete selection with mapped values
- **Status Display**: Read-only text editor showing application state
- **Custom Layout**: Responsive positioning and sizing

### Best Practices Learned

- **Type Safety**: Using templates and proper listener registration
- **Memory Management**: Automatic cleanup and RAII principles
- **Event Handling**: Clean separation of concerns with dedicated handler methods
- **Responsive Design**: Adaptive layouts for different screen sizes
- **Modern C++**: Leveraging C++14 features for cleaner, more maintainable code

### Next Steps

In Module 3, we'll add real audio functionality to make this control panel actually generate and process sound. You'll learn about:

- JUCE audio processing architecture
- Real-time audio callbacks
- Sine wave generation
- Audio device management
- Thread safety in audio applications

---

## Additional Resources

- [JUCE Component Class Reference](https://docs.juce.com/master/classComponent.html)
- [JUCE Graphics and Layout Tutorial](https://docs.juce.com/master/tutorial_graphics_class.html)
- [Modern C++ Features in JUCE](https://docs.juce.com/master/tutorial_cpp14_features.html)
- [JUCE Look and Feel Customization](https://docs.juce.com/master/tutorial_look_and_feel_customisation.html)

---

## Troubleshooting Common Issues

### Build Errors

**Problem**: Undefined reference to listener methods
**Solution**: Ensure your class inherits from the appropriate listener interface and implements all pure virtual methods.

**Problem**: Components not visible
**Solution**: Check that you've called `addAndMakeVisible()` and that `resized()` sets proper bounds.

### Runtime Issues

**Problem**: Events not firing
**Solution**: Verify listener registration with `addListener(this)` and check pointer comparisons in event handlers.

**Problem**: Layout issues on resize
**Solution**: Ensure `resized()` method handles all child components and uses proper Rectangle manipulation.

### Performance Considerations

- Use `juce::dontSendNotification` when setting values programmatically to avoid recursive updates
- Implement efficient `paint()` methods that only draw what's necessary
- Consider using `Timer` for periodic updates rather than continuous polling

---

*This tutorial is part of a comprehensive JUCE learning series. Module 2 builds upon the foundation from Module 1 and prepares you for audio processing in Module 3.*
**Code Challenge**: Implement a two-column layout that switches to single column on narrow

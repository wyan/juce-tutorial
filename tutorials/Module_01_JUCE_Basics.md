# Module 1: Understanding JUCE Basics and Current Setup

## Learning Objectives

By the end of this module, you will:
- Understand JUCE framework architecture and module system
- Analyze the current project structure and build system
- Learn JUCE component hierarchy and lifecycle
- Introduction to modern C++ features used in JUCE (auto, range-based loops, smart pointers)
- Set up development environment and build the current project

## Table of Contents

1. [JUCE Framework Overview and Philosophy](#juce-framework-overview-and-philosophy)
2. [Current Project Structure Analysis](#current-project-structure-analysis)
3. [JUCE Module System](#juce-module-system)
4. [Component Hierarchy and Lifecycle](#component-hierarchy-and-lifecycle)
5. [Modern C++ Features in JUCE](#modern-cpp-features-in-juce)
6. [Build System Walkthrough](#build-system-walkthrough)
7. [Practical Exercises](#practical-exercises)
8. [Code Examples and Explanations](#code-examples-and-explanations)

---

## JUCE Framework Overview and Philosophy

### What is JUCE?

JUCE (Jules' Utility Class Extensions) is a cross-platform C++ application framework that specializes in audio applications and plugins. Originally created by Julian Storer, JUCE has become the industry standard for developing audio software.

### Core Philosophy

JUCE follows several key design principles:

1. **Cross-Platform Consistency**: Write once, run everywhere (Windows, macOS, Linux, iOS, Android)
2. **Modern C++ Design**: Leverages modern C++ features while maintaining compatibility
3. **Component-Based Architecture**: Everything is a component that can be composed and reused
4. **Event-Driven Programming**: Reactive programming model with callbacks and listeners
5. **Memory Safety**: Smart pointer usage and RAII principles throughout

### Key Features

- **GUI Framework**: Complete windowing system with native look-and-feel
- **Audio Processing**: Real-time audio processing capabilities
- **Plugin Development**: Support for VST, AU, AAX, and other plugin formats
- **Graphics**: 2D graphics with hardware acceleration support
- **Networking**: HTTP, WebSocket, and other networking capabilities
- **File I/O**: Cross-platform file system access
- **Threading**: Thread-safe utilities and concurrent programming support

---

## Current Project Structure Analysis

Let's examine our current minimal JUCE application structure:

```
test-juce/
├── JUCE/                    # JUCE framework source code
├── Source/                  # Our application source code
│   ├── Main.cpp            # Application entry point
│   ├── MainComponent.h     # Main component header
│   └── MainComponent.cpp   # Main component implementation
├── CMakeLists.txt          # Build configuration
└── tutorials/              # Tutorial modules (this directory)
```

### Main.cpp Analysis

```cpp
#include <JuceHeader.h>
#include "MainComponent.h"

class MainApp  : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "TestJuce"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    void initialise (const juce::String&) override
    {
        mainWindow.reset (new MainWindow ("TestJuce", new MainComponent(), *this));
    }
    void shutdown() override { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow (juce::String name, juce::Component* c, JUCEApplication& app)
            : DocumentWindow (name, juce::Colours::lightgrey, DocumentWindow::allButtons), owner(app)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }
        void closeButtonPressed() override { owner.systemRequestedQuit(); }
    private:
        JUCEApplication& owner;
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (MainApp)
```

**Key Points:**
- `MainApp` inherits from `juce::JUCEApplication` - the base class for all JUCE applications
- `initialise()` is called when the app starts - creates our main window
- `shutdown()` is called when the app closes - cleans up resources
- `MainWindow` is a nested class inheriting from `juce::DocumentWindow`
- `START_JUCE_APPLICATION` macro creates the application entry point

### MainComponent Analysis

**Header (MainComponent.h):**
```cpp
#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Label helloLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
```

**Implementation (MainComponent.cpp):**
```cpp
#include "MainComponent.h"

MainComponent::MainComponent()
{
    helloLabel.setText ("Hullo JUCE!", juce::dontSendNotification);
    helloLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (helloLabel);
    setSize (800, 480);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    helloLabel.setBounds (getLocalBounds());
}
```

**Key Points:**
- Inherits from `juce::Component` - the base class for all UI elements
- Constructor sets up the label and component size
- `paint()` handles custom drawing (background color)
- `resized()` handles layout when component size changes
- `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` prevents copying and detects memory leaks

---

## JUCE Module System

JUCE is organized into modules - self-contained units of functionality that you can include in your project as needed.

### Core Modules

Our project currently uses these modules (from CMakeLists.txt):

```cmake
target_link_libraries(TestJuce PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
)
```

### Common JUCE Modules

| Module | Purpose |
|--------|---------|
| `juce_core` | Essential utilities, strings, memory management |
| `juce_events` | Message system, timers, event handling |
| `juce_graphics` | 2D graphics, fonts, images |
| `juce_gui_basics` | Basic GUI components |
| `juce_gui_extra` | Advanced GUI components |
| `juce_audio_basics` | Audio data structures, MIDI |
| `juce_audio_devices` | Audio/MIDI device access |
| `juce_audio_formats` | Audio file reading/writing |
| `juce_audio_processors` | Audio plugin framework |

### Module Dependencies

Modules have dependencies on each other:
- `juce_gui_extra` depends on `juce_gui_basics`
- `juce_gui_basics` depends on `juce_graphics`
- `juce_graphics` depends on `juce_events`
- `juce_events` depends on `juce_core`

JUCE automatically includes dependent modules, so specifying `juce_gui_extra` automatically includes all its dependencies.

---

## Component Hierarchy and Lifecycle

### Component Hierarchy

JUCE uses a tree-like component hierarchy:

```
MainWindow (DocumentWindow)
└── MainComponent (Component)
    └── helloLabel (Label)
```

### Component Lifecycle

Every JUCE component goes through these lifecycle stages:

1. **Construction**: Object is created, member variables initialized
2. **Setup**: `addAndMakeVisible()` adds child components
3. **Sizing**: `setSize()` or parent calls `setBounds()`
4. **Layout**: `resized()` is called to position child components
5. **Painting**: `paint()` is called to draw the component
6. **Event Handling**: Mouse, keyboard, and other events are processed
7. **Destruction**: Component is destroyed, children automatically cleaned up

### Key Virtual Methods

Every component can override these methods:

```cpp
class MyComponent : public juce::Component
{
public:
    // Called when component needs to be drawn
    void paint (juce::Graphics& g) override;

    // Called when component size changes
    void resized() override;

    // Mouse event handlers
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    // Keyboard event handlers
    bool keyPressed (const juce::KeyPress& key) override;
};
```

### Parent-Child Relationships

- Parents own their children (automatic cleanup)
- Children are drawn relative to parent coordinates
- Events bubble up from child to parent
- `addAndMakeVisible()` adds a child and makes it visible
- `removeChildComponent()` removes a child

---

## Modern C++ Features in JUCE

JUCE extensively uses modern C++ features. Here are the key ones you'll encounter:

### 1. Auto Keyword

```cpp
// Instead of:
juce::String longVariableName = "Hello World";

// Use:
auto longVariableName = juce::String("Hello World");

// Especially useful with complex types:
auto bounds = getLocalBounds().reduced(10);
```

### 2. Range-Based For Loops

```cpp
// Iterating over child components
for (auto* child : getChildren())
{
    child->setVisible(true);
}

// Iterating over arrays
juce::Array<int> numbers = {1, 2, 3, 4, 5};
for (auto number : numbers)
{
    DBG("Number: " << number);
}
```

### 3. Smart Pointers

JUCE uses smart pointers for automatic memory management:

```cpp
// std::unique_ptr - single ownership
std::unique_ptr<MainWindow> mainWindow;
mainWindow.reset(new MainWindow(...));  // Takes ownership
// Automatically deleted when mainWindow goes out of scope

// std::shared_ptr - shared ownership (less common in JUCE)
std::shared_ptr<SomeResource> resource = std::make_shared<SomeResource>();

// Raw pointers still used for components (parent owns children)
auto* label = new juce::Label();
addAndMakeVisible(label);  // Parent takes ownership
```

### 4. Lambda Functions

```cpp
// Timer callback using lambda
juce::Timer::callAfterDelay(1000, []()
{
    DBG("Timer fired!");
});

// Button callback using lambda
button.onClick = [this]()
{
    // Handle button click
    label.setText("Button clicked!", juce::dontSendNotification);
};
```

### 5. Override and Default Keywords

```cpp
class MainComponent : public juce::Component
{
public:
    // Explicitly mark as override (compiler checks)
    void paint (juce::Graphics& g) override;

    // Use default destructor
    ~MainComponent() override = default;

    // Delete copy constructor
    MainComponent(const MainComponent&) = delete;
};
```

---

## Build System Walkthrough

Our project uses CMake as the build system. Let's examine the CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.15)

project(TestJuce VERSION 0.1.0)

# Add JUCE module
add_subdirectory(JUCE)

# Create a GUI app
juce_add_gui_app(TestJuce
    VERSION 0.1.0
    COMPANY_NAME "finitud-labs"
    PRODUCT_NAME "test-juce"
)

# Source files
target_sources(TestJuce PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
)

juce_generate_juce_header(TestJuce)

# JUCE modules to link
target_link_libraries(TestJuce PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
)
```

### Key CMake Commands

1. **`add_subdirectory(JUCE)`**: Includes the JUCE framework
2. **`juce_add_gui_app()`**: Creates a GUI application target
3. **`target_sources()`**: Specifies source files to compile
4. **`juce_generate_juce_header()`**: Creates JuceHeader.h with all module includes
5. **`target_link_libraries()`**: Links JUCE modules to our application

### Build Process

1. CMake generates platform-specific build files (Makefiles, Xcode projects, Visual Studio solutions)
2. The build system compiles JUCE modules
3. Our source files are compiled
4. Everything is linked together into the final executable

---

## Practical Exercises

### Exercise 1: Build and Run the Current Project

**Objective**: Get familiar with the build process and see the current application in action.

**Steps**:
1. Open a terminal in the project root directory
2. Create a build directory: `mkdir build && cd build`
3. Generate build files: `cmake ..`
4. Build the project: `cmake --build .`
5. Run the application (the exact command depends on your platform)

**Expected Result**: A window should appear with "Hullo JUCE!" displayed in the center on a dark grey background.

**Troubleshooting**:
- Make sure you have CMake 3.15+ installed
- Ensure you have a C++ compiler installed (Xcode on macOS, Visual Studio on Windows, GCC/Clang on Linux)

### Exercise 2: Modify the Label Text and Styling

**Objective**: Learn how to modify component properties and understand the component update cycle.

**Steps**:
1. Open `Source/MainComponent.cpp`
2. In the constructor, change the label text from "Hullo JUCE!" to "Welcome to JUCE Development!"
3. Add color to the label: `helloLabel.setColour(juce::Label::textColourId, juce::Colours::white);`
4. Change the font size: `helloLabel.setFont(juce::Font(24.0f, juce::Font::bold));`
5. Rebuild and run the application

**Expected Result**: The label should display the new text in white, bold, 24pt font.

**Code to add**:
```cpp
MainComponent::MainComponent()
{
    helloLabel.setText ("Welcome to JUCE Development!", juce::dontSendNotification);
    helloLabel.setJustificationType (juce::Justification::centred);
    helloLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    helloLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    addAndMakeVisible (helloLabel);
    setSize (800, 480);
}
```

### Exercise 3: Add Debug Output to Understand Component Lifecycle

**Objective**: Understand when different component methods are called.

**Steps**:
1. Add debug output to the constructor, `paint()`, and `resized()` methods
2. Use `DBG()` macro for debug output
3. Rebuild and run, then resize the window to see when methods are called

**Code to add**:
```cpp
MainComponent::MainComponent()
{
    DBG("MainComponent constructor called");
    helloLabel.setText ("Hullo JUCE!", juce::dontSendNotification);
    helloLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (helloLabel);
    setSize (800, 480);
}

void MainComponent::paint (juce::Graphics& g)
{
    DBG("MainComponent paint() called");
    g.fillAll (juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    DBG("MainComponent resized() called - new size: " << getWidth() << "x" << getHeight());
    helloLabel.setBounds (getLocalBounds());
}
```

**Expected Result**: You should see debug output in your IDE's console or terminal showing when each method is called.

### Exercise 4: Experiment with Window Properties

**Objective**: Learn about window management and component sizing.

**Steps**:
1. In `MainComponent.cpp`, change the initial size: `setSize (1200, 600);`
2. In `Main.cpp`, modify the window creation to set a minimum size:
   ```cpp
   setResizeLimits(400, 300, 1600, 1200);
   ```
3. Try different background colors in the `paint()` method
4. Experiment with different label justifications

**Expected Result**: A larger window with size constraints and visual changes.

---

## Code Examples and Explanations

### Example 1: Understanding Component Ownership

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        // Create a label - we own this object
        titleLabel = std::make_unique<juce::Label>();
        titleLabel->setText("My App", juce::dontSendNotification);
        addAndMakeVisible(*titleLabel);

        // Create a button - parent takes ownership
        auto* button = new juce::Button("Click Me");
        addAndMakeVisible(button);
        // Don't need to store pointer - parent manages lifetime
    }

private:
    std::unique_ptr<juce::Label> titleLabel;  // We own this
    // Button is owned by parent Component class
};
```

### Example 2: Event Handling Pattern

```cpp
class MainComponent : public juce::Component,
                     public juce::Button::Listener
{
public:
    MainComponent()
    {
        clickButton.setButtonText("Click Me!");
        clickButton.addListener(this);  // Register for button events
        addAndMakeVisible(clickButton);
    }

    // Button::Listener interface
    void buttonClicked(juce::Button* button) override
    {
        if (button == &clickButton)
        {
            statusLabel.setText("Button was clicked!",
                              juce::dontSendNotification);
        }
    }

private:
    juce::TextButton clickButton;
    juce::Label statusLabel;
};
```

### Example 3: Layout Management

```cpp
void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Reserve space for header
    auto headerArea = bounds.removeFromTop(60);
    titleLabel.setBounds(headerArea.reduced(10));

    // Split remaining area
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightArea = bounds;

    // Position components
    leftButton.setBounds(leftArea.reduced(10));
    rightButton.setBounds(rightArea.reduced(10));
}
```

### Example 4: Modern C++ in JUCE Context

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        // Range-based for loop with initializer list
        for (auto& buttonText : {"Play", "Stop", "Record"})
        {
            auto button = std::make_unique<juce::TextButton>(buttonText);

            // Lambda for button callback
            button->onClick = [this, buttonText]()
            {
                handleButtonClick(buttonText);
            };

            addAndMakeVisible(*button);
            buttons.push_back(std::move(button));
        }
    }

private:
    void handleButtonClick(const juce::String& buttonName)
    {
        // Auto type deduction
        auto message = "Clicked: " + buttonName;
        DBG(message);
    }

    std::vector<std::unique_ptr<juce::TextButton>> buttons;
};
```

---

## Summary

In this module, you've learned:

1. **JUCE Philosophy**: Cross-platform, component-based, modern C++ framework
2. **Project Structure**: How a basic JUCE application is organized
3. **Module System**: How JUCE functionality is divided into modules
4. **Component Lifecycle**: Construction, layout, painting, and event handling
5. **Modern C++**: Auto, smart pointers, lambdas, and range-based loops
6. **Build System**: CMake configuration for JUCE projects

### Key Takeaways

- Every JUCE application inherits from `JUCEApplication`
- Components form a hierarchy with automatic memory management
- The `paint()` method handles drawing, `resized()` handles layout
- JUCE embraces modern C++ while maintaining cross-platform compatibility
- The module system allows you to include only the functionality you need

### Next Steps

In Module 2, we'll dive deeper into GUI components and event handling, building upon the foundation established here. You'll learn to create interactive interfaces with buttons, sliders, and custom components.

---

## Additional Resources

- [JUCE Documentation](https://docs.juce.com/)
- [JUCE Tutorials](https://docs.juce.com/master/tutorial_getting_started.html)
- [JUCE Forum](https://forum.juce.com/)
- [JUCE GitHub Repository](https://github.com/juce-framework/JUCE)

---

*This tutorial is part of a comprehensive JUCE learning series. Each module builds upon previous knowledge while introducing new concepts and practical skills.*

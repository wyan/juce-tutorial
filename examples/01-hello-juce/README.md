# Hello JUCE - Basic Application Example

This is the simplest possible JUCE application that demonstrates the fundamental structure and concepts of JUCE development. Perfect for beginners to understand how JUCE applications are organized.

## 🎯 Learning Objectives

- Understand basic JUCE application structure
- Learn about the Component hierarchy
- Implement basic window management
- Handle simple graphics and events
- Set up a minimal build configuration

## 📁 Files Overview

```
01-hello-juce/
├── README.md           # This documentation
├── CMakeLists.txt      # Build configuration
├── Source/
│   ├── Main.cpp        # Application entry point
│   └── MainComponent.h # Main GUI component
└── build/              # Build directory (created when building)
```

## 🔧 Building and Running

### Quick Start
```bash
cd examples/01-hello-juce
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run the application
# Windows: ./Release/HelloJUCE.exe
# macOS: open ./Release/HelloJUCE.app
# Linux: ./HelloJUCE
```

### Platform-Specific Build
```bash
# Windows (Visual Studio)
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# macOS (Xcode)
cmake .. -G Xcode
cmake --build . --config Release

# Linux (Make)
cmake ..
make -j$(nproc)
```

## 📚 Code Explanation

### Application Entry Point ([`Main.cpp`](Source/Main.cpp))

The main application class inherits from `juce::JUCEApplication` and manages the application lifecycle:

```cpp
class HelloJUCEApplication : public juce::JUCEApplication
{
public:
    // Application metadata
    const juce::String getApplicationName() override { return "Hello JUCE"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    // Lifecycle methods
    void initialise(const juce::String& commandLine) override;
    void shutdown() override;

private:
    std::unique_ptr<MainWindow> mainWindow;
};
```

**Key Concepts:**
- **Application Lifecycle**: `initialise()` and `shutdown()` methods
- **Window Management**: Creating and managing the main window
- **JUCE Macros**: `START_JUCE_APPLICATION()` macro

### Main Component ([`MainComponent.h`](Source/MainComponent.h))

The main GUI component demonstrates basic JUCE component concepts:

```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent();

    // Component lifecycle
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Event handling
    void mouseDown(const juce::MouseEvent& event) override;

private:
    juce::String displayText;
    juce::Colour backgroundColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
```

**Key Concepts:**
- **Custom Drawing**: `paint()` method for graphics
- **Layout Management**: `resized()` method for component sizing
- **Event Handling**: Mouse and keyboard interaction
- **Memory Management**: JUCE leak detector

## 🎨 What You'll See

When you run the application, you'll see:

1. **Window**: A resizable window titled "Hello JUCE"
2. **Background**: A gradient background that changes color
3. **Text**: "Hello, JUCE World!" displayed in the center
4. **Interaction**: Click anywhere to change the background color
5. **Responsiveness**: Text stays centered when window is resized

## 🔍 Key Learning Points

### 1. JUCE Application Structure
```cpp
// Every JUCE app needs these components:
class MyApp : public juce::JUCEApplication { /* ... */ };
class MainWindow : public juce::DocumentWindow { /* ... */ };
class MainComponent : public juce::Component { /* ... */ };
START_JUCE_APPLICATION(MyApp)
```

### 2. Component Hierarchy
- Applications contain Windows
- Windows contain Components
- Components can contain child Components
- Parent components manage child lifetimes

### 3. Graphics and Painting
```cpp
void paint(juce::Graphics& g) override
{
    // Fill background
    g.fillAll(backgroundColour);

    // Draw text
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Hello, JUCE World!", getLocalBounds(),
               juce::Justification::centred);
}
```

### 4. Event Handling
```cpp
void mouseDown(const juce::MouseEvent& event) override
{
    // Change background color on click
    backgroundColour = juce::Colour::fromHSV(
        juce::Random::getSystemRandom().nextFloat(), 0.8f, 0.9f, 1.0f);
    repaint(); // Trigger a repaint
}
```

### 5. Layout Management
```cpp
void resized() override
{
    // This is called when the component size changes
    // Use this to position child components
    // In this simple example, we don't have child components
}
```

## 🚀 Experiments to Try

### Beginner Modifications
1. **Change the text**: Modify the display text in `paint()`
2. **Different colors**: Try different background colors
3. **Font changes**: Experiment with different fonts and sizes
4. **Window size**: Change the initial window size

### Intermediate Modifications
1. **Add a button**: Include a `juce::TextButton` as a child component
2. **Multiple colors**: Cycle through predefined colors on click
3. **Animation**: Use a `juce::Timer` to animate the background
4. **Keyboard input**: Handle keyboard events to change text

### Advanced Modifications
1. **Custom shapes**: Draw custom graphics instead of just text
2. **Multiple components**: Add several child components with different behaviors
3. **Settings**: Save and restore window position and colors
4. **Menu system**: Add a menu bar with options

## 🔗 Related Examples

After mastering this example, try:
- **[02-audio-player](../02-audio-player/)** - Add audio functionality
- **[04-gui-components](../04-gui-components/)** - More complex GUI elements
- **[03-basic-synthesizer](../03-basic-synthesizer/)** - Audio generation

## 📖 Further Reading

### JUCE Documentation
- [Component Class](https://docs.juce.com/master/classComponent.html)
- [Graphics Class](https://docs.juce.com/master/classGraphics.html)
- [JUCEApplication Class](https://docs.juce.com/master/classJUCEApplication.html)

### Tutorial Series
- [Module 1: JUCE Basics](../../tutorials/Module_01_JUCE_Basics.md)
- [Module 2: Basic GUI Components](../../tutorials/Module_02_Basic_GUI_Components.md)

---

*This example is part of the JUCE Audio Development Tutorial Series. It provides a foundation for understanding JUCE application structure and basic GUI programming.*

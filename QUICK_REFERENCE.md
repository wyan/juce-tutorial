# JUCE Audio Development Quick Reference

A comprehensive quick lookup guide for JUCE classes, concepts, and modern C++ patterns used throughout the tutorial series. Perfect for developers who need fast access to syntax, patterns, and best practices.

## 📚 Table of Contents

1. [JUCE Core Classes](#juce-core-classes)
2. [GUI Components](#gui-components)
3. [Audio Processing](#audio-processing)
4. [Modern C++ Patterns](#modern-cpp-patterns)
5. [Common Patterns](#common-patterns)
6. [Performance Optimization](#performance-optimization)
7. [Troubleshooting Guide](#troubleshooting-guide)
8. [Code Snippets](#code-snippets)

---

## 🎯 JUCE Core Classes

### Component System

#### [`juce::Component`](https://docs.juce.com/master/classComponent.html)
Base class for all GUI elements

```cpp
class MyComponent : public juce::Component
{
public:
    MyComponent() { setSize(400, 300); }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
    }

    void resized() override {
        // Layout child components
        childComponent.setBounds(getLocalBounds().reduced(10));
    }

    void mouseDown(const juce::MouseEvent& e) override {
        // Handle mouse events
    }
};
```

**Key Methods:**
- `paint(Graphics&)` - Custom drawing
- `resized()` - Layout management
- `addAndMakeVisible(Component&)` - Add child component
- `setBounds(Rectangle<int>)` - Set position and size
- `getLocalBounds()` - Get component's area

#### [`juce::Graphics`](https://docs.juce.com/master/classGraphics.html)
Drawing context for custom graphics

```cpp
void paint(juce::Graphics& g) override
{
    // Fill background
    g.fillAll(juce::Colours::black);

    // Set color and draw shapes
    g.setColour(juce::Colours::white);
    g.drawRect(10, 10, 100, 50);
    g.fillEllipse(20, 20, 80, 30);

    // Draw text
    g.setFont(16.0f);
    g.drawText("Hello JUCE", 10, 70, 100, 20, juce::Justification::centred);

    // Draw paths
    juce::Path path;
    path.startNewSubPath(0, 0);
    path.lineTo(100, 100);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}
```

#### [`juce::Rectangle<int>`](https://docs.juce.com/master/classRectangle.html)
Geometric rectangle for layout management

```cpp
void resized() override
{
    auto bounds = getLocalBounds();

    // Remove areas from bounds
    auto header = bounds.removeFromTop(50);
    auto footer = bounds.removeFromBottom(30);
    auto leftPanel = bounds.removeFromLeft(200);
    auto rightPanel = bounds; // Remaining area

    // Position components
    titleLabel.setBounds(header.reduced(10));
    statusLabel.setBounds(footer);
    leftComponent.setBounds(leftPanel.reduced(5));
    rightComponent.setBounds(rightPanel.reduced(5));
}
```

---

## 🎛️ GUI Components

### Interactive Controls

#### [`juce::Button`](https://docs.juce.com/master/classButton.html) & [`juce::TextButton`](https://docs.juce.com/master/classTextButton.html)

```cpp
// Setup
juce::TextButton playButton;
playButton.setButtonText("Play");
playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);

// Event handling (modern approach)
playButton.onClick = [this]() {
    isPlaying = !isPlaying;
    playButton.setButtonText(isPlaying ? "Stop" : "Play");
};

// Event handling (traditional approach)
class MyComponent : public juce::Component, public juce::Button::Listener
{
    void buttonClicked(juce::Button* button) override {
        if (button == &playButton) {
            // Handle click
        }
    }
};
```

#### [`juce::Slider`](https://docs.juce.com/master/classSlider.html)

```cpp
// Setup
juce::Slider volumeSlider;
volumeSlider.setRange(0.0, 1.0, 0.01);           // min, max, step
volumeSlider.setValue(0.5);
volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

// Event handling
volumeSlider.onValueChange = [this]() {
    auto value = volumeSlider.getValue();
    // Process value change
};

// Listener approach
class MyComponent : public juce::Slider::Listener
{
    void sliderValueChanged(juce::Slider* slider) override {
        if (slider == &volumeSlider) {
            auto value = slider->getValue();
        }
    }
};
```

#### [`juce::ComboBox`](https://docs.juce.com/master/classComboBox.html)

```cpp
// Setup
juce::ComboBox formatCombo;
formatCombo.addItem("WAV", 1);
formatCombo.addItem("AIFF", 2);
formatCombo.addItem("FLAC", 3);
formatCombo.setSelectedId(1);

// Event handling
formatCombo.onChange = [this]() {
    auto selectedId = formatCombo.getSelectedId();
    auto selectedText = formatCombo.getText();
};
```

#### [`juce::Label`](https://docs.juce.com/master/classLabel.html)

```cpp
// Setup
juce::Label titleLabel;
titleLabel.setText("Audio Processor", juce::dontSendNotification);
titleLabel.setJustificationType(juce::Justification::centred);
titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

// Editable label
titleLabel.setEditable(true);
titleLabel.onTextChange = [this]() {
    auto newText = titleLabel.getText();
};
```

### Layout Management

#### FlexBox Layout
```cpp
void resized() override
{
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

    flexBox.items.add(juce::FlexItem(component1).withMinHeight(50).withFlex(1));
    flexBox.items.add(juce::FlexItem(component2).withMinHeight(50).withFlex(2));

    flexBox.performLayout(getLocalBounds());
}
```

#### Grid Layout
```cpp
void resized() override
{
    juce::Grid grid;
    grid.templateRows = {juce::Grid::TrackInfo(1_fr), juce::Grid::TrackInfo(1_fr)};
    grid.templateColumns = {juce::Grid::TrackInfo(1_fr), juce::Grid::TrackInfo(1_fr)};

    grid.items.add(juce::GridItem(component1));
    grid.items.add(juce::GridItem(component2));
    grid.items.add(juce::GridItem(component3));
    grid.items.add(juce::GridItem(component4));

    grid.performLayout(getLocalBounds());
}
```

---

## 🔊 Audio Processing

### Audio Application Structure

#### [`juce::JUCEApplication`](https://docs.juce.com/master/classJUCEApplication.html)

```cpp
class MyAudioApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "MyAudioApp"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow("MyAudioApp", new MainComponent(), *this));
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MyAudioApp)
```

### Audio I/O

#### [`juce::AudioDeviceManager`](https://docs.juce.com/master/classAudioDeviceManager.html)

```cpp
class AudioComponent : public juce::AudioIODeviceCallback
{
public:
    AudioComponent() {
        deviceManager.initialise(2, 2, nullptr, true);
        deviceManager.addAudioCallback(this);
    }

    ~AudioComponent() {
        deviceManager.removeAudioCallback(this);
    }

    void audioDeviceIOCallback(const float** inputChannelData,
                              int numInputChannels,
                              float** outputChannelData,
                              int numOutputChannels,
                              int numSamples) override {
        // Process audio here - REAL-TIME SAFE CODE ONLY!
        for (int channel = 0; channel < numOutputChannels; ++channel) {
            for (int sample = 0; sample < numSamples; ++sample) {
                outputChannelData[channel][sample] = inputChannelData[channel][sample] * gain;
            }
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        sampleRate = device->getCurrentSampleRate();
        bufferSize = device->getCurrentBufferSizeSamples();
    }

    void audioDeviceStopped() override {}

private:
    juce::AudioDeviceManager deviceManager;
    double sampleRate = 44100.0;
    int bufferSize = 512;
    float gain = 1.0f;
};
```

### Plugin Development

#### [`juce::AudioProcessor`](https://docs.juce.com/master/classAudioProcessor.html)

```cpp
class MyAudioProcessor : public juce::AudioProcessor
{
public:
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        // Initialize processing
        filter.prepare(sampleRate, samplesPerBlock);
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override {
        juce::ScopedNoDenormals noDenormals;

        // Process MIDI
        for (const auto metadata : midiMessages) {
            handleMidiMessage(metadata.getMessage());
        }

        // Process audio
        filter.processBlock(buffer);
    }

    void releaseResources() override {
        filter.reset();
    }

    // Plugin info
    const juce::String getName() const override { return "MyPlugin"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Programs
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return "Default"; }

    // State
    void getStateInformation(juce::MemoryBlock& destData) override {
        // Save state
    }
    void setStateInformation(const void* data, int sizeInBytes) override {
        // Load state
    }

    // Editor
    juce::AudioProcessorEditor* createEditor() override {
        return new MyAudioProcessorEditor(*this);
    }
    bool hasEditor() const override { return true; }

private:
    MyFilter filter;
};
```

#### [`juce::AudioProcessorValueTreeState`](https://docs.juce.com/master/classAudioProcessorValueTreeState.html)

```cpp
class MyAudioProcessor : public juce::AudioProcessor
{
public:
    MyAudioProcessor() : parameters(*this, nullptr, "Parameters", createParameterLayout()) {
        gainParam = parameters.getRawParameterValue("gain");
        frequencyParam = parameters.getRawParameterValue("frequency");
    }

private:
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* frequencyParam = nullptr;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "gain", "Gain",
            juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "frequency", "Frequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));

        return {params.begin(), params.end()};
    }
};
```

### DSP Processing

#### Basic Filter Implementation

```cpp
class SimpleFilter
{
public:
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        reset();
    }

    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
    }

    void setFrequency(float frequency) {
        auto omega = 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
        auto sin_omega = std::sin(omega);
        auto cos_omega = std::cos(omega);
        auto alpha = sin_omega / (2.0f * Q);

        // Low-pass coefficients
        b0 = (1.0f - cos_omega) / 2.0f;
        b1 = 1.0f - cos_omega;
        b2 = b0;
        a1 = -2.0f * cos_omega;
        a2 = 1.0f - alpha;

        // Normalize
        auto norm = 1.0f / (1.0f + alpha);
        b0 *= norm; b1 *= norm; b2 *= norm;
        a1 *= norm; a2 *= norm;
    }

    float processSample(float input) {
        auto output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        x2 = x1; x1 = input;
        y2 = y1; y1 = output;

        return output;
    }

    void processBlock(juce::AudioBuffer<float>& buffer) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                channelData[sample] = processSample(channelData[sample]);
            }
        }
    }

private:
    double sampleRate = 44100.0;
    float Q = 0.707f;
    float b0 = 1, b1 = 0, b2 = 0;
    float a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};
```

---

## 🚀 Modern C++ Patterns

### C++11 Features

#### Smart Pointers
```cpp
// Unique ownership
std::unique_ptr<juce::Component> component = std::make_unique<MyComponent>();

// Shared ownership (less common in JUCE)
std::shared_ptr<AudioProcessor> processor = std::make_shared<MyProcessor>();

// JUCE components use raw pointers (parent owns children)
auto* button = new juce::TextButton("Click Me");
addAndMakeVisible(button); // Parent takes ownership
```

#### Lambda Expressions
```cpp
// Simple lambda
button.onClick = []() { DBG("Button clicked!"); };

// Capture by reference
button.onClick = [&]() { isPlaying = !isPlaying; };

// Capture by value
button.onClick = [=]() { processValue(currentValue); };

// Capture specific variables
button.onClick = [this, &processor](){ processor.setEnabled(!processor.isEnabled()); };

// Generic lambda (C++14)
auto processAudio = [](auto& buffer) {
    for (auto& sample : buffer) {
        sample *= 0.5f; // Apply gain
    }
};
```

#### Range-Based For Loops
```cpp
// Iterate over container
std::vector<float> samples = {0.1f, 0.2f, 0.3f};
for (auto sample : samples) {
    DBG("Sample: " << sample);
}

// Modify elements
for (auto& sample : samples) {
    sample *= 2.0f;
}

// Iterate over child components
for (auto* child : getChildren()) {
    child->setVisible(true);
}
```

#### Auto Keyword
```cpp
// Type deduction
auto bounds = getLocalBounds();
auto colour = juce::Colours::blue;
auto processor = std::make_unique<MyProcessor>();

// With functions
auto createButton = [](const juce::String& text) {
    return std::make_unique<juce::TextButton>(text);
};
```

### C++14 Features

#### Generic Lambdas
```cpp
// Generic lambda
auto applyGain = [](auto& buffer, auto gain) {
    for (auto& sample : buffer) {
        sample *= gain;
    }
};

// Use with different types
std::vector<float> floatBuffer;
std::vector<double> doubleBuffer;
applyGain(floatBuffer, 0.5f);
applyGain(doubleBuffer, 0.5);
```

#### Auto Return Types
```cpp
// C++14: auto return type deduction
auto createProcessor(ProcessorType type) {
    switch (type) {
        case ProcessorType::Filter:
            return std::make_unique<FilterProcessor>();
        case ProcessorType::Delay:
            return std::make_unique<DelayProcessor>();
        default:
            return std::unique_ptr<AudioProcessor>{};
    }
}
```

### C++17 Features

#### Structured Bindings
```cpp
// Unpack pairs/tuples
auto [min, max] = std::minmax_element(buffer.begin(), buffer.end());
auto [width, height] = getSize();

// Unpack custom types
struct AudioStats { float rms, peak; };
auto [rms, peak] = calculateAudioStats(buffer);
```

#### std::optional
```cpp
std::optional<float> findPeakFrequency(const std::vector<float>& spectrum) {
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    if (maxIt != spectrum.end() && *maxIt > threshold) {
        return std::distance(spectrum.begin(), maxIt) * binWidth;
    }
    return std::nullopt;
}

// Usage
if (auto freq = findPeakFrequency(spectrum)) {
    DBG("Peak frequency: " << *freq << " Hz");
}
```

### C++20 Features

#### Concepts
```cpp
// Define audio sample concept
template<typename T>
concept AudioSample = std::floating_point<T> || std::integral<T>;

// Use concept in function
template<AudioSample T>
void processBuffer(std::vector<T>& buffer) {
    // Type-safe audio processing
}

// Concept-constrained class
template<AudioSample T>
class TypeSafeProcessor {
public:
    void process(T sample) requires std::floating_point<T> {
        // Float-specific processing
    }
};
```

#### Coroutines (Basic Example)
```cpp
#include <coroutine>

// Simple task type
template<typename T>
struct Task {
    struct promise_type {
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(T value) { result = value; }
        void unhandled_exception() {}
        T result;
    };

    std::coroutine_handle<promise_type> handle;
    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }

    T get() { return handle.promise().result; }
};

// Async audio file loading
Task<std::vector<float>> loadAudioFileAsync(const std::string& filename) {
    // Simulate async loading
    co_await std::suspend_always{};

    std::vector<float> audioData(44100); // 1 second
    // Load audio data...

    co_return audioData;
}
```

---

## 🔧 Common Patterns

### RAII (Resource Acquisition Is Initialization)

```cpp
class AudioBufferManager {
public:
    AudioBufferManager(size_t size) : buffer(size) {
        DBG("Buffer allocated: " << size << " samples");
    }

    ~AudioBufferManager() {
        DBG("Buffer deallocated");
        // Automatic cleanup
    }

    // Move-only semantics
    AudioBufferManager(const AudioBufferManager&) = delete;
    AudioBufferManager& operator=(const AudioBufferManager&) = delete;

    AudioBufferManager(AudioBufferManager&&) = default;
    AudioBufferManager& operator=(AudioBufferManager&&) = default;

private:
    std::vector<float> buffer;
};
```

### Observer Pattern (Listeners)

```cpp
class ParameterListener {
public:
    virtual ~ParameterListener() = default;
    virtual void parameterChanged(const juce::String& paramID, float newValue) = 0;
};

class ParameterManager {
public:
    void addListener(ParameterListener* listener) {
        listeners.add(listener);
    }

    void removeListener(ParameterListener* listener) {
        listeners.removeAllInstancesOf(listener);
    }

    void setParameter(const juce::String& paramID, float value) {
        parameters[paramID] = value;
        listeners.call([&](ParameterListener& l) {
            l.parameterChanged(paramID, value);
        });
    }

private:
    juce::ListenerList<ParameterListener> listeners;
    std::map<juce::String, float> parameters;
};
```

### Singleton Pattern (Use Sparingly)

```cpp
class AudioEngine {
public:
    static AudioEngine& getInstance() {
        static AudioEngine instance;
        return instance;
    }

    // Delete copy constructor and assignment
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void startProcessing() { /* implementation */ }
    void stopProcessing() { /* implementation */ }

private:
    AudioEngine() = default;
    ~AudioEngine() = default;
};

// Usage
AudioEngine::getInstance().startProcessing();
```

---

## ⚡ Performance Optimization

### Real-Time Safe Programming

#### Do's and Don'ts in Audio Callback

```cpp
void audioDeviceIOCallback(const float** inputChannelData,
                          int numInputChannels,
                          float** outputChannelData,
                          int numOutputChannels,
                          int numSamples) override
{
    // ✅ DO: Use stack variables
    float gain = 0.5f;

    // ✅ DO: Use pre-allocated buffers
    tempBuffer.setSize(numOutputChannels, numSamples, false, false, true);

    // ✅ DO: Use atomic variables for parameters
    float currentGain = gainParameter.load();

    // ✅ DO: Simple arithmetic operations
    for (int channel = 0; channel < numOutputChannels; ++channel) {
        for (int sample = 0; sample < numSamples; ++sample) {
            outputChannelData[channel][sample] =
                inputChannelData[channel][sample] * currentGain;
        }
    }

    // ❌ DON'T: Allocate memory
    // std::vector<float> buffer(numSamples); // BAD!

    // ❌ DON'T: Use locks
    // std::lock_guard<std::mutex> lock(mutex); // BAD!

    // ❌ DON'T: Call system functions
    // DBG("Processing audio"); // BAD!

    // ❌ DON'T: Access GUI components
    // slider.setValue(currentGain); // BAD!
}
```

#### Lock-Free Communication

```cpp
// Use atomic variables for simple data
std::atomic<float> gainParameter{1.0f};

// Use lock-free queues for complex data
template<typename T, size_t Size>
class LockFreeQueue {
public:
    bool push(const T& item) {
        auto currentTail = tail.load();
        auto nextTail = (currentTail + 1) % Size;

        if (nextTail != head.load()) {
            buffer[currentTail] = item;
            tail.store(nextTail);
            return true;
        }
        return false; // Queue full
    }

    bool pop(T& item) {
        auto currentHead = head.load();
        if (currentHead == tail.load()) {
            return false; // Queue empty
        }

        item = buffer[currentHead];
        head.store((currentHead + 1) % Size);
        return true;
    }

private:
    std::array<T, Size> buffer;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
};
```

### SIMD Optimization

```cpp
// Use JUCE's SIMD utilities
void processBufferSIMD(juce::AudioBuffer<float>& buffer, float gain) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        auto numSamples = buffer.getNumSamples();

        // Process in chunks of 4 samples
        juce::FloatVectorOperations::multiply(channelData, gain, numSamples);
    }
}

// Manual SIMD (advanced)
#include <immintrin.h>

void processBufferAVX(float* buffer, int numSamples, float gain) {
    const __m256 gainVec = _mm256_set1_ps(gain);

    int simdSamples = numSamples & ~7; // Round down to multiple of 8

    for (int i = 0; i < simdSamples; i += 8) {
        __m256 samples = _mm256_load_ps(&buffer[i]);
        samples = _mm256_mul_ps(samples, gainVec);
        _mm256_store_ps(&buffer[i], samples);
    }

    // Process remaining samples
    for (int i = simdSamples; i < numSamples; ++i) {
        buffer[i] *= gain;
    }
}
```

---

## 🐛 Troubleshooting Guide

### Common Build Issues

#### CMake Configuration Problems
```bash
# Clear CMake cache
rm -rf build/
mkdir build && cd build

# Regenerate with verbose output
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

# Check JUCE submodule
git submodule update --init --recursive
```

#### Missing Dependencies
```bash
# macOS
xcode-select --install

# Ubuntu/Debian
sudo apt-get install build-essential libasound2-dev libjack-jackd2-dev \
    ladspa-sdk libcurl4-openssl-dev libfreetype6-dev libx11-dev \
    libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev

# Windows (using vcpkg)
vcpkg install curl freetype
```

### Runtime Issues

#### Audio Dropouts
```cpp
// Check buffer sizes
void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
    auto bufferSize = device->getCurrentBufferSizeSamples();
    auto sampleRate = device->getCurrentSampleRate();

    DBG("Buffer size: " << bufferSize << " samples");
    DBG("Sample rate: " << sampleRate << " Hz");
    DBG("Latency: " << (bufferSize / sampleRate * 1000.0) << " ms");

    // Ensure processing can complete within buffer time
    jassert(bufferSize >= 64 && bufferSize <= 2048);
}
```

#### Memory Leaks
```cpp
// Use JUCE's leak detector
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyComponent)

// Check for proper cleanup
~MyComponent() override {
    // Remove listeners
    button.removeListener(this);

    // Stop timers
    stopTimer();

    // Clear references
    processor = nullptr;
}
```

#### Plugin Loading Issues
```cpp
// Validate plugin in constructor
MyAudioProcessor() {
    // Check JUCE version compatibility
    jassert(JUCE_MAJOR_VERSION >= 6);

    // Validate parameter ranges
    jassert(gainParam != nullptr);
    jassert(gainParam->load() >= -60.0f && gainParam->load() <= 12.0f);
}
```

### Performance Issues

#### CPU Usage Too High
```cpp
// Profile your code
class ScopedProfiler {
public:
    ScopedProfiler(const juce::String& name) : name_(name) {
        start_ = juce::Time::getHighResolutionTicks();
    }

    ~ScopedProfiler() {
        auto end = juce::Time::getHighResolutionTicks();
        auto elapsed = juce::Time::highResolutionTicksToSeconds(end - start_) * 1000.0;
        DBG(name_ << " took " << elapsed << " ms");
    }

private:
    juce::String name_;
    juce::int64 start_;
};

#define PROFILE_SCOPE(name) ScopedProfiler _prof(name)

void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
    PROFILE_SCOPE("processBlock");

    {
        PROFILE_SCOPE("EQ Processing");
        eq.processBlock(buffer);
    }

    {
        PROFILE_SCOPE("Compressor Processing");
        compressor.processBlock(buffer);
    }
}
```

#### GUI Performance
```cpp
// Optimize paint() calls
void paint(juce::Graphics& g) override {
    // Cache expensive calculations
    if (needsRecalculation) {
        cachedPath.clear();
        // Rebuild path...
        needsRecalculation = false;
    }

    g.strokePath(cachedPath, juce::PathStrokeType(2.0f));
}

// Use Timer for periodic updates instead of constant repainting
class MyComponent : public juce::Component, public juce::Timer {
public:
    MyComponent() {
        startTimerHz(30); // 30 FPS updates
    }

    void timerCallback() override {
        if (needsUpdate) {
            repaint();
            needsUpdate = false;
        }
    }
};
```

---

## 💡 Code Snippets

### Utility Functions

#### Decibel Conversions
```cpp
// Convert linear gain to decibels
inline float gainToDecibels(float gain) {
    return gain > 0.0f ? 20.0f * std::log10(gain) : -100.0f;
}

// Convert decibels to linear gain
inline float decibelsToGain(float decibels) {
    return decibels > -100.0f ? std::pow(10.0f, decibels * 0.05f) : 0.0f;
}
```

#### Frequency Conversions
```cpp
// Convert frequency to MIDI note number
inline float frequencyToMidi(float frequency) {
    return 69.0f + 12.0f * std::log2(frequency / 440.0f);
}

// Convert MIDI note number to frequency
inline float midiToFrequency(float midiNote) {
    return 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
}
```

#### Audio Buffer Utilities
```cpp
// Calculate RMS level
float calculateRMS(const juce::AudioBuffer<float>& buffer, int channel) {
    auto* data = buffer.getReadPointer(channel);
    auto numSamples = buffer.getNumSamples();

    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        sum += data[i] * data[i];
    }

    return std::sqrt(sum / numSamples);
}

// Find peak level
float findPeak(const juce::AudioBuffer<float>& buffer, int channel) {
    auto* data = buffer.getReadPointer(channel);
    auto numSamples = buffer.getNumSamples();

    return *std::max_element(data, data + numSamples,
        [](float a, float b) { return std::abs(a) < std::abs(b); });
}

// Apply fade in/out
void applyFade(juce::AudioBuffer<float>& buffer, int channel,
               int startSample, int numSamples, bool fadeIn) {
    auto* data = buffer.getWritePointer(channel);

    for (int i = 0; i < numSamples; ++i) {
        float gain = fadeIn ?
            static_cast<float>(i) / numSamples :
            static_cast<float>(numSamples - i) / numSamples;
        data[startSample + i] *= gain;
    }
}
```

### Custom Components

#### Rotary Slider
```cpp
class RotarySlider : public juce::Slider
{
public:
    RotarySlider() {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                           juce::MathConstants<float>::pi * 2.8f, true);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 10.0f;

        // Draw track
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(centre.x - radius, centre.y - radius,
                     radius * 2.0f, radius * 2.0f, 2.0f);

        // Draw thumb
        auto angle = getRotaryParameters().startAngleRadians +
                    (getValue() - getMinimum()) / (getMaximum() - getMinimum()) *
                    (getRotaryParameters().endAngleRadians - getRotaryParameters().startAngleRadians);

        auto thumbX = centre.x + radius * 0.8f * std::cos(angle - juce::MathConstants<float>::halfPi);
        auto thumbY = centre.y + radius * 0.8f * std::sin(angle - juce::MathConstants<float>::halfPi);

        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbX - 4, thumbY - 4, 8, 8);
    }
};
```

#### Level Meter
```cpp
class LevelMeter : public juce::Component, public juce::Timer
{
public:
    LevelMeter() {
        startTimerHz(30);
    }

    void setLevel(float newLevel) {
        targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colours::black);
        g.fillRect(bounds);

        // Level bar
        auto levelHeight = bounds.getHeight() * currentLevel;
        auto levelBounds = bounds.withTop(bounds.getBottom() - levelHeight);

        // Color based on level
        if (currentLevel > 0.8f)
            g.setColour(juce::Colours::red);
        else if (currentLevel > 0.6f)
            g.setColour(juce::Colours::yellow);
        else
            g.setColour(juce::Colours::green);

        g.fillRect(levelBounds);

        // Border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1);
    }

    void timerCallback() override {
        // Smooth level changes
        currentLevel += (targetLevel - currentLevel) * 0.2f;

        if (std::abs(currentLevel - targetLevel) > 0.001f) {
            repaint();
        }
    }

private:
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;
};
```

### File I/O

#### Audio File Loading
```cpp
class AudioFileLoader
{
public:
    static std::unique_ptr<juce::AudioBuffer<float>> loadAudioFile(const juce::File& file) {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        auto reader = std::unique_ptr<juce::AudioFormatReader>(
            formatManager.createReaderFor(file));

        if (reader == nullptr)
            return nullptr;

        auto buffer = std::make_unique<juce::AudioBuffer<float>>(
            static_cast<int>(reader->numChannels),
            static_cast<int>(reader->lengthInSamples));

        reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

        return buffer;
    }

    static bool saveAudioFile(const juce::AudioBuffer<float>& buffer,
                             const juce::File& file, double sampleRate) {
        juce::WavAudioFormat format;
        auto outputStream = file.createOutputStream();

        if (outputStream == nullptr)
            return false;

        auto writer = std::unique_ptr<juce::AudioFormatWriter>(
            format.createWriterFor(outputStream.release(), sampleRate,
                                 buffer.getNumChannels(), 24, {}, 0));

        if (writer == nullptr)
            return false;

        return writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
};
```

#### Settings Management
```cpp
class SettingsManager
{
public:
    SettingsManager(const juce::String& applicationName)
        : settings(juce::PropertiesFile::Options()
                  .withApplicationName(applicationName)
                  .withFilenameSuffix(".settings")
                  .withFolderName(applicationName)) {}

    template<typename T>
    void setValue(const juce::String& key, const T& value) {
        if constexpr (std::is_same_v<T, float>) {
            settings.setValue(key, value);
        } else if constexpr (std::is_same_v<T, int>) {
            settings.setValue(key, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            settings.setValue(key, value);
        } else if constexpr (std::is_same_v<T, juce::String>) {
            settings.setValue(key, value);
        }
        settings.saveIfNeeded();
    }

    template<typename T>
    T getValue(const juce::String& key, const T& defaultValue) const {
        if constexpr (std::is_same_v<T, float>) {
            return static_cast<float>(settings.getDoubleValue(key, defaultValue));
        } else if constexpr (std::is_same_v<T, int>) {
            return settings.getIntValue(key, defaultValue);
        } else if constexpr (std::is_same_v<T, bool>) {
            return settings.getBoolValue(key, defaultValue);
        } else if constexpr (std::is_same_v<T, juce::String>) {
            return settings.getValue(key, defaultValue);
        }
    }

private:
    juce::ApplicationProperties settings;
};
```

---

## 🔗 Quick Links

### Official Documentation
- [JUCE Documentation](https://docs.juce.com/)
- [JUCE Tutorials](https://docs.juce.com/master/tutorial_getting_started.html)
- [JUCE API Reference](https://docs.juce.com/master/classes.html)
- [JUCE Forum](https://forum.juce.com/)

### Modern C++ Resources
- [cppreference.com](https://en.cppreference.com/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Compiler Explorer](https://godbolt.org/)

### Audio Programming Resources
- [The Audio Programming Book](http://www.aspbooks.com/)
- [DSP Guide](http://www.dspguide.com/)
- [Audio Developer Conference](https://audio.dev/)

---

## 📝 Notes

- **Real-time Safety**: Always consider real-time constraints in audio callbacks
- **Memory Management**: Use RAII and smart pointers appropriately
- **Performance**: Profile your code and optimize hot paths
- **Cross-platform**: Test on all target platforms
- **Modern C++**: Leverage modern features for cleaner, safer code

---

*This quick reference is part of the comprehensive JUCE Audio Development Tutorial Series. For detailed explanations and step-by-step tutorials, see the main [README](README.md) and [Tutorial Index](tutorials/README.md).*

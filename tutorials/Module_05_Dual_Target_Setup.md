# Module 5: Dual-Target Setup (Standalone + Plugin)

## Learning Objectives

By the end of this module, you will:
- Configure multi-target builds (standalone app + plugin formats)
- Implement AUv3 support for iPad and macOS
- Learn cross-platform deployment strategies
- Understand code sharing between targets
- Master build system optimization
- Introduction to C++20 features (concepts, modules basics)

## Table of Contents

1. [Multi-Target Architecture Overview](#multi-target-architecture-overview)
2. [Build System Configuration](#build-system-configuration)
3. [AUv3 Implementation](#auv3-implementation)
4. [Code Sharing Strategies](#code-sharing-strategies)
5. [Cross-Platform Deployment](#cross-platform-deployment)
6. [Platform-Specific Optimizations](#platform-specific-optimizations)
7. [C++20 Features Introduction](#c20-features-introduction)
8. [Practical Implementation](#practical-implementation)
9. [Practical Exercises](#practical-exercises)
10. [Deployment and Distribution](#deployment-and-distribution)

---

## Multi-Target Architecture Overview

### Understanding Dual-Target Development

Dual-target development allows you to maintain a single codebase that can be built as both a standalone application and various plugin formats. This approach maximizes code reuse while providing flexibility for different deployment scenarios.

#### Target Types Comparison

| Aspect | Standalone App | Plugin (VST3/AU) | AUv3 (iOS/macOS) |
|--------|----------------|------------------|------------------|
| **Host Environment** | Independent | DAW-hosted | App/DAW-hosted |
| **Audio I/O** | Direct device access | Host-provided | Host-provided |
| **GUI Integration** | Native window | Embedded view | View controller |
| **Lifecycle** | User-controlled | Host-controlled | Host-controlled |
| **Distribution** | App stores/direct | Plugin directories | App Store only |
| **Platform Support** | All platforms | Desktop only | iOS/macOS only |

### Architecture Design Patterns

```cpp
// Multi-target architecture with shared core
namespace AudioGenerator {

// Shared core functionality
class AudioCore
{
public:
    virtual ~AudioCore() = default;

    // Pure audio processing - target agnostic
    virtual void processBlock(juce::AudioBuffer<float>& buffer) = 0;
    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
    virtual void releaseResources() = 0;

    // Parameter management
    virtual void setParameter(const juce::String& paramID, float value) = 0;
    virtual float getParameter(const juce::String& paramID) const = 0;
};

// Target-specific implementations
class StandaloneCore : public AudioCore
{
public:
    StandaloneCore();

    // Standalone-specific features
    void setAudioDeviceManager(juce::AudioDeviceManager* manager);
    void handleDeviceChange();

private:
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
};

class PluginCore : public AudioCore
{
public:
    PluginCore(juce::AudioProcessor& processor);

    // Plugin-specific features
    void syncWithHost();
    void handleAutomation();

private:
    juce::AudioProcessor& hostProcessor;
};

class AUv3Core : public AudioCore
{
public:
    AUv3Core();

    // AUv3-specific features
    void handleViewControllerLifecycle();
    void optimizeForMobile();

private:
    bool isMobileOptimized = false;
};

} // namespace AudioGenerator
```

### Target Detection and Conditional Compilation

```cpp
// Platform and target detection
#pragma once

// Compiler and platform detection
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define PLATFORM_IOS 1
        #define PLATFORM_MOBILE 1
    #elif TARGET_OS_MAC
        #define PLATFORM_MACOS 1
        #define PLATFORM_DESKTOP 1
    #endif
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_DESKTOP 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
    #define PLATFORM_DESKTOP 1
#endif

// Target type detection
#if JucePlugin_Build_Standalone
    #define TARGET_STANDALONE 1
#elif JucePlugin_Build_VST3 || JucePlugin_Build_AU || JucePlugin_Build_AAX
    #define TARGET_PLUGIN 1
#elif JucePlugin_Build_AUv3
    #define TARGET_AUV3 1
    #define TARGET_MOBILE_PLUGIN 1
#endif

// Feature availability based on target
namespace TargetFeatures {
    constexpr bool hasDirectAudioAccess = TARGET_STANDALONE;
    constexpr bool hasHostIntegration = TARGET_PLUGIN || TARGET_AUV3;
    constexpr bool supportsTouchInterface = PLATFORM_IOS || PLATFORM_MOBILE;
    constexpr bool supportsFileSystem = !TARGET_AUV3; // AUv3 has limited file access
    constexpr bool supportsMultiWindow = TARGET_STANDALONE && PLATFORM_DESKTOP;
}

// Conditional compilation helpers
#define IF_STANDALONE(code) do { if constexpr (TARGET_STANDALONE) { code } } while(0)
#define IF_PLUGIN(code) do { if constexpr (TARGET_PLUGIN) { code } } while(0)
#define IF_AUV3(code) do { if constexpr (TARGET_AUV3) { code } } while(0)
#define IF_MOBILE(code) do { if constexpr (PLATFORM_MOBILE) { code } } while(0)
```

---

## Build System Configuration

### Enhanced CMakeLists.txt for Multi-Target Builds

```cmake
cmake_minimum_required(VERSION 3.22)

project(JUCEAudioGenerator VERSION 1.0.0)

# C++20 requirement for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration options
option(BUILD_STANDALONE "Build standalone application" ON)
option(BUILD_VST3 "Build VST3 plugin" ON)
option(BUILD_AU "Build Audio Unit plugin" ON)
option(BUILD_AUV3 "Build AUv3 plugin for iOS/macOS" OFF)
option(BUILD_AAX "Build AAX plugin" OFF)
option(ENABLE_CODESIGNING "Enable code signing" OFF)

# Platform detection
if(APPLE)
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(PLATFORM_IOS TRUE)
        set(BUILD_AUV3 ON)  # Enable AUv3 for iOS builds
    else()
        set(PLATFORM_MACOS TRUE)
    endif()
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

# JUCE setup
add_subdirectory(JUCE)

# Shared configuration
set(PLUGIN_NAME "JUCEAudioGenerator")
set(PLUGIN_MANUFACTURER "finitud-labs")
set(PLUGIN_DESCRIPTION "Professional dual-target audio generator")
set(PLUGIN_CATEGORY "Generator")

# Shared source files
set(SHARED_SOURCES
    Source/AudioEngine.cpp
    Source/SharedAudioComponent.cpp
    Source/ParameterManager.cpp
    Source/PresetManager.cpp
)

# Platform-specific sources
set(PLATFORM_SOURCES)
if(PLATFORM_IOS)
    list(APPEND PLATFORM_SOURCES
        Source/iOS/TouchInterface.cpp
        Source/iOS/AUv3Wrapper.cpp
    )
elseif(PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        Source/macOS/NativeIntegration.cpp
    )
elseif(PLATFORM_WINDOWS)
    list(APPEND PLATFORM_SOURCES
        Source/Windows/WindowsOptimizations.cpp
    )
endif()

# Function to configure common target properties
function(configure_target target_name)
    target_sources(${target_name} PRIVATE ${SHARED_SOURCES} ${PLATFORM_SOURCES})

    target_link_libraries(${target_name} PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
    )

    target_compile_definitions(${target_name} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
    )

    # Platform-specific definitions
    if(PLATFORM_IOS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_IOS=1
            JUCE_MOBILE=1
        )
    endif()

    # C++20 features
    target_compile_features(${target_name} PRIVATE cxx_std_20)

    # Optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target_name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -flto>
        )
    endif()
endfunction()

# Plugin target (VST3, AU, AAX)
if(BUILD_VST3 OR BUILD_AU OR BUILD_AAX)
    set(PLUGIN_FORMATS)
    if(BUILD_VST3)
        list(APPEND PLUGIN_FORMATS VST3)
    endif()
    if(BUILD_AU AND APPLE)
        list(APPEND PLUGIN_FORMATS AU)
    endif()
    if(BUILD_AAX)
        list(APPEND PLUGIN_FORMATS AAX)
    endif()

    juce_add_plugin(${PLUGIN_NAME}
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JAG5"
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME "${PLUGIN_NAME}"
        DESCRIPTION "${PLUGIN_DESCRIPTION}"
        PLUGIN_CATEGORY_ID "kPluginCategGenerator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES "Generator"
        AU_MAIN_TYPE "kAudioUnitType_Generator"
    )

    target_sources(${PLUGIN_NAME} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
    )

    configure_target(${PLUGIN_NAME})

    # Plugin-specific modules
    target_link_libraries(${PLUGIN_NAME} PRIVATE
        juce::juce_audio_plugin_client
    )
endif()

# AUv3 target for iOS/macOS
if(BUILD_AUV3 AND APPLE)
    juce_add_plugin(${PLUGIN_NAME}_AUv3
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JA3V"
        FORMATS AUv3
        PRODUCT_NAME "${PLUGIN_NAME} AUv3"
        DESCRIPTION "${PLUGIN_DESCRIPTION} for iOS and macOS"
        PLUGIN_CATEGORY_ID "kAudioUnitType_Generator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        AUV3_CATEGORY "Generator"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator.auv3"
    )

    target_sources(${PLUGIN_NAME}_AUv3 PRIVATE
        Source/PluginProcessor.cpp
        Source/AUv3/AUv3Editor.cpp
    )

    configure_target(${PLUGIN_NAME}_AUv3)

    # AUv3-specific configuration
    if(PLATFORM_IOS)
        set_target_properties(${PLUGIN_NAME}_AUv3 PROPERTIES
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
        )
    endif()
endif()

# Standalone application
if(BUILD_STANDALONE)
    juce_add_gui_app(${PLUGIN_NAME}_Standalone
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PRODUCT_NAME "${PLUGIN_NAME}"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator"
    )

    target_sources(${PLUGIN_NAME}_Standalone PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Standalone/StandaloneWrapper.cpp
    )

    configure_target(${PLUGIN_NAME}_Standalone)
endif()

# Code signing configuration
if(ENABLE_CODESIGNING AND APPLE)
    include(cmake/CodeSigning.cmake)
    setup_code_signing()
endif()

# Custom build targets
add_custom_target(build_all_targets
    DEPENDS
        $<$<BOOL:${BUILD_STANDALONE}>:${PLUGIN_NAME}_Standalone>
        $<$<BOOL:${BUILD_VST3}>:${PLUGIN_NAME}>
        $<$<BOOL:${BUILD_AUV3}>:${PLUGIN_NAME}_AUv3>
)

# Installation and packaging
include(cmake/Installation.cmake)
include(cmake/Packaging.cmake)
```

### Platform-Specific Build Scripts

#### macOS/iOS Build Script

```bash
#!/bin/bash
# build_macos_ios.sh - Comprehensive build script for Apple platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# Configuration
BUILD_TYPE="Release"
ENABLE_CODESIGNING=false
BUILD_IOS=false
BUILD_MACOS=true
XCODE_VERSION=$(xcodebuild -version | head -n1 | awk '{print $2}')

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --codesign)
            ENABLE_CODESIGNING=true
            shift
            ;;
        --ios)
            BUILD_IOS=true
            BUILD_MACOS=false
            shift
            ;;
        --all)
            BUILD_IOS=true
            BUILD_MACOS=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=== JUCE Audio Generator - Apple Platform Build ==="
echo "Build Type: $BUILD_TYPE"
echo "Xcode Version: $XCODE_VERSION"
echo "Code Signing: $ENABLE_CODESIGNING"
echo ""

# Function to build for specific platform
build_platform() {
    local platform=$1
    local build_suffix=$2
    local cmake_args=$3

    echo "Building for $platform..."

    local platform_build_dir="$BUILD_DIR/$build_suffix"
    mkdir -p "$platform_build_dir"
    cd "$platform_build_dir"

    # Configure
    cmake "$PROJECT_ROOT" \
        -G "Xcode" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DENABLE_CODESIGNING="$ENABLE_CODESIGNING" \
        $cmake_args

    # Build
    cmake --build . --config "$BUILD_TYPE" --parallel $(sysctl -n hw.ncpu)

    echo "$platform build completed successfully!"
    echo ""
}

# Build macOS targets
if [ "$BUILD_MACOS" = true ]; then
    build_platform "macOS" "macos" "-DBUILD_AUV3=ON"
fi

# Build iOS targets
if [ "$BUILD_IOS" = true ]; then
    build_platform "iOS" "ios" "\
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphoneos \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DBUILD_STANDALONE=OFF \
        -DBUILD_VST3=OFF \
        -DBUILD_AU=OFF \
        -DBUILD_AUV3=ON"
fi

echo "=== Build Summary ==="
if [ "$BUILD_MACOS" = true ]; then
    echo "macOS builds:"
    echo "  - Standalone: $BUILD_DIR/macos/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/"
    echo "  - VST3: $BUILD_DIR/macos/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/"
    echo "  - AU: $BUILD_DIR/macos/JUCEAudioGenerator_artefacts/$BUILD_TYPE/AU/"
    echo "  - AUv3: $BUILD_DIR/macos/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/"
fi

if [ "$BUILD_IOS" = true ]; then
    echo "iOS builds:"
    echo "  - AUv3: $BUILD_DIR/ios/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/"
fi

echo ""
echo "Build completed successfully!"
```

#### Windows Build Script

```batch
@echo off
REM build_windows.bat - Windows build script

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%
set BUILD_DIR=%PROJECT_ROOT%build

REM Configuration
set BUILD_TYPE=Release
set ENABLE_CODESIGNING=OFF
set GENERATOR="Visual Studio 17 2022"

REM Parse command line arguments
:parse_args
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="--vs2019" (
    set GENERATOR="Visual Studio 16 2019"
    shift
    goto parse_args
)
if "%1"=="" goto start_build

:start_build
echo === JUCE Audio Generator - Windows Build ===
echo Build Type: %BUILD_TYPE%
echo Generator: %GENERATOR%
echo.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure
echo Configuring CMake...
cmake "%PROJECT_ROOT%" ^
    -G %GENERATOR% ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_VST3=ON ^
    -DBUILD_AU=OFF ^
    -DBUILD_AUV3=OFF ^
    -DBUILD_STANDALONE=ON

if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo ERROR: Build failed!
    exit /b 1
)

echo.
echo === Build Summary ===
echo Standalone: %BUILD_DIR%\JUCEAudioGenerator_Standalone_artefacts\%BUILD_TYPE%\
echo VST3: %BUILD_DIR%\JUCEAudioGenerator_artefacts\%BUILD_TYPE%\VST3\
echo.
echo Build completed successfully!

pause
```

---

## AUv3 Implementation

### AUv3-Specific Architecture

AUv3 (Audio Unit version 3) is Apple's modern plugin format designed for iOS and macOS, with special considerations for mobile devices and App Store distribution.

```cpp
// AUv3Editor.h - iOS/macOS optimized editor
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#if TARGET_AUV3

class AUv3Editor : public juce::AudioProcessorEditor,
                   public juce::Timer
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

    // iOS-specific lifecycle
    void viewWillAppear();
    void viewDidDisappear();
    void handleMemoryWarning();

private:
    JUCEAudioGeneratorProcessor& audioProcessor;

    // Touch-optimized components
    class TouchSlider;
    class TouchButton;

    std::unique_ptr<TouchSlider> volumeSlider;
    std::unique_ptr<TouchSlider> frequencySlider;
    std::unique_ptr<TouchButton> waveTypeButton;

    // Mobile-specific features
    bool isTouchDevice = false;
    float touchScaleFactor = 1.0f;
    juce::Point<float> lastTouchPosition;

    // Performance optimization
    bool isVisible = false;
    int updateCounter = 0;

    // Layout management
    void layoutForOrientation();
    void updateTouchScaling();

    // iOS integration
    void setupiOSSpecificFeatures();
    void handleOrientationChange();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AUv3Editor)
};

// Touch-optimized slider component
class AUv3Editor::TouchSlider : public juce::Slider
{
public:
    TouchSlider() : juce::Slider()
    {
        // Larger touch targets for mobile
        setSize(60, 200);
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);

        // Touch-friendly styling
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a90e2));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff2d3748));
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a202c));
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        // Haptic feedback on iOS
        #if JUCE_IOS
        if (auto* haptics = juce::HapticFeedback::getInstance())
            haptics->impact(juce::HapticFeedback::light);
        #endif

        juce::Slider::mouseDown(event);
    }

    void paint(juce::Graphics& g) override
    {
        // Custom touch-friendly rendering
        auto bounds = getLocalBounds().toFloat();

        // Track
        auto trackBounds = bounds.reduced(20, 10);
        g.setColour(findColour(juce::Slider::trackColourId));
        g.fillRoundedRectangle(trackBounds, 4.0f);

        // Thumb - larger for touch
        auto thumbY = juce::jmap(getValue(), getMinimum(), getMaximum(),
                                trackBounds.getBottom() - 15, trackBounds.getY() + 15);
        auto thumbBounds = juce::Rectangle<float>(trackBounds.getX() - 5, thumbY - 15,
                                                 trackBounds.getWidth() + 10, 30);

        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillRoundedRectangle(thumbBounds, 15.0f);

        // Value text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        auto textBounds = bounds.removeFromBottom(30);
        g.drawText(getTextFromValue(getValue()), textBounds, juce::Justification::centred);
    }
};

#endif // TARGET_AUV3
```

### AUv3 Implementation Details

```cpp
// AUv3Editor.cpp
#include "AUv3Editor.h"

#if TARGET_AUV3

AUv3Editor::AUv3Editor(JUCEAudioGeneratorProcessor& processor)
    : AudioProcessorEditor(&processor)
    , audioProcessor(processor)
{
    setupiOSSpecificFeatures();

    // Create touch-optimized components
    volumeSlider = std::make_unique<TouchSlider>();
    volumeSlider->setRange(0.0, 1.0, 0.01);
    volumeSlider->setValue(0.5);
    addAndMakeVisible(*volumeSlider);

    frequencySlider = std::make_unique<TouchSlider>();
    frequencySlider->setRange(20.0, 20000.0, 1.0);
    frequencySlider->setValue(440.0);
    frequencySlider->setSkewFactorFromMidPoint(1000.0);
    addAndMakeVisible(*frequencySlider);

    // Parameter attachments
    auto& params = audioProcessor.getParameters();
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "volume", *volumeSlider);
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "frequency", *frequencySlider);

    // Detect touch device
    #if JUCE_IOS
    isTouchDevice = true;
    touchScaleFactor = 1.2f; // Larger touch targets
    #else
    isTouchDevice = juce::Desktop::getInstance().getMainMouseSource().isTouch();
    touchScaleFactor = isTouchDevice ? 1.2f : 1.0f;
    #endif

    // Set appropriate size for AUv3
    setSize(320, 480); // iPhone-friendly default

    // Start timer for updates
    startTimer(33); // ~30 FPS
}

void AUv3Editor::setupiOSSpecificFeatures()
{
    #if JUCE_IOS
    // Register for orientation changes
    [[NSNotificationCenter defaultCenter]
        addObserverForName:UIDeviceOrientationDidChangeNotification
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification* notification) {
            handleOrientationChange();
        }];

    // Register for memory warnings
    [[NSNotificationCenter defaultCenter]
        addObserverForName:UIApplicationDidReceiveMemoryWarningNotification
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification* notification) {
            handleMemoryWarning();
        }];
    #endif
}

void AUv3Editor::paint(juce::Graphics& g)
{
    // Gradient background optimized for mobile
    auto bounds = getLocalBounds();

    juce::ColourGradient gradient(
        juce::Colour(0xff1a202c), bounds.getTopLeft().toFloat(),
        juce::Colour(0xff2d3748), bounds.getBottomRight().toFloat(),
        false
    );

    g.setGradientFill(gradient);
    g.fillAll();

    // Title with larger font for mobile
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(isTouchDevice ? 24.0f : 20.0f, juce::Font::bold));

    auto titleBounds = bounds.removeFromTop(60);
    g.drawText("Audio Generator", titleBounds, juce::Justification::centred);
}

void AUv3Editor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(60); // Title space

    // Layout for mobile-first design
    if (getWidth() < getHeight()) // Portrait
    {
        // Vertical layout
        auto sliderHeight = (bounds.getHeight() - 40) / 2;

        volumeSlider->setBounds(bounds.removeFromTop(sliderHeight).reduced(20));
        bounds.removeFromTop(20); // Spacing
        frequencySlider->setBounds(bounds.removeFromTop(sliderHeight).reduced(20));
    }
    else // Landscape
    {
        // Horizontal layout
        auto sliderWidth = (bounds.getWidth() - 60) / 2;

        volumeSlider->setBounds(bounds.removeFromLeft(sliderWidth).reduced(10));
        bounds.removeFromLeft(20); // Spacing
        frequencySlider->setBounds(bounds.removeFromLeft(sliderWidth).reduced(10));
    }
}

void AUv3Editor::handleOrientationChange()
{
    // Defer layout update to next run loop
    juce::MessageManager::callAsync([this]() {
        layoutForOrientation();
    });
}

void AUv3Editor::handleMemoryWarning()
{
    // Reduce update frequency during memory pressure
    stopTimer();
    startTimer(100); // Reduce to 10 FPS

    // Clear any cached graphics
    repaint();
}

void AUv3Editor::viewWillAppear()
{
    isVisible = true;
    startTimer(33); // Resume normal update rate
}

void AUv3Editor::viewDidDisappear()
{
    isVisible = false;
    stopTimer(); // Stop updates when not visible
}

void AUv3Editor::timerCallback()
{
    if (!isVisible) return;

    // Throttled updates for performance
    updateCounter++;
    if (updateCounter % 3 == 0) // Update every 3rd frame
    {
        // Update any real-time displays
        repaint();
    }
}

#endif // TARGET_AUV3
```

### AUv3 Info.plist Configuration

```xml
<!-- AUv3-Info.plist template -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>

CFBundleExecutable</key>
    <string>${EXECUTABLE_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.finitud-labs.juceaudiogenerator.auv3</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>JUCE Audio Generator AUv3</string>
    <key>CFBundlePackageType</key>
    <string>XPC!</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>NSExtension</key>
    <dict>
        <key>NSExtensionAttributes</key>
        <dict>
            <key>AudioComponents</key>
            <array>
                <dict>
                    <key>description</key>
                    <string>JUCE Audio Generator</string>
                    <key>manufacturer</key>
                    <string>Flab</string>
                    <key>name</key>
                    <string>finitud-labs: JUCE Audio Generator</string>
                    <key>subtype</key>
                    <string>JAG5</string>
                    <key>type</key>
                    <string>augn</string>
                    <key>version</key>
                    <integer>1</integer>
                    <key>factoryFunction</key>
                    <string>JUCEAudioGeneratorAUv3Factory</string>
                    <key>sandboxSafe</key>
                    <true/>
                    <key>tags</key>
                    <array>
                        <string>Generator</string>
                    </array>
                </dict>
            </array>
        </dict>
        <key>NSExtensionPointIdentifier</key>
        <string>com.apple.AudioUnit-UI</string>
        <key>NSExtensionPrincipalClass</key>
        <string>JUCEAudioGeneratorAUv3</string>
    </dict>
    <key>UIRequiredDeviceCapabilities</key>
    <array>
        <string>arm64</string>
    </array>
    <key>UISupportedInterfaceOrientations</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
        <string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>
    </array>
    <key>UISupportedInterfaceOrientations~ipad</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
        <string>UIInterfaceOrientationPortraitUpsideDown</string>
        <string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>
    </array>
</dict>
</plist>
```

---

## Code Sharing Strategies

### Shared Component Architecture

The key to successful dual-target development is creating a robust shared component architecture that maximizes code reuse while allowing for target-specific customizations.

```cpp
// SharedAudioComponent.h - Core shared functionality
#pragma once

#include <JuceHeader.h>
#include "TargetDetection.h"

namespace SharedComponents {

// Abstract base for shared audio processing
class AudioProcessorCore
{
public:
    virtual ~AudioProcessorCore() = default;

    // Core audio processing interface
    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
    virtual void processBlock(juce::AudioBuffer<float>& buffer) = 0;
    virtual void releaseResources() = 0;

    // Parameter management
    virtual void setParameter(const juce::String& paramID, float value) = 0;
    virtual float getParameter(const juce::String& paramID) const = 0;

    // State management
    virtual void saveState(juce::MemoryBlock& destData) = 0;
    virtual void loadState(const void* data, int sizeInBytes) = 0;

protected:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool isInitialized = false;
};

// Shared parameter management system
class ParameterManager
{
public:
    struct ParameterInfo
    {
        juce::String id;
        juce::String name;
        float minValue;
        float maxValue;
        float defaultValue;
        float currentValue;
        juce::String units;
        std::function<juce::String(float)> valueToText;
        std::function<float(const juce::String&)> textToValue;
    };

    ParameterManager();

    void addParameter(const ParameterInfo& info);
    void setParameter(const juce::String& id, float value);
    float getParameter(const juce::String& id) const;

    const std::vector<ParameterInfo>& getAllParameters() const { return parameters; }

    // Target-specific parameter creation
    template<typename ParameterType>
    std::unique_ptr<ParameterType> createJUCEParameter(const juce::String& id) const;

private:
    std::vector<ParameterInfo> parameters;
    std::unordered_map<juce::String, size_t> parameterMap;
    mutable std::mutex parameterMutex;
};

// Shared GUI component base
class SharedGUIComponent : public juce::Component
{
public:
    SharedGUIComponent();
    virtual ~SharedGUIComponent() = default;

    // Platform-aware rendering
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Touch interface support
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Accessibility support
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

protected:
    // Platform detection
    bool isTouchDevice() const;
    bool isMobileDevice() const;
    float getScaleFactor() const;

    // Adaptive layout helpers
    juce::Rectangle<int> getAdaptiveBounds(const juce::Rectangle<int>& baseBounds) const;
    int getAdaptiveSize(int baseSize) const;

    // Theme management
    virtual void updateTheme();
    juce::Colour getThemeColour(const juce::String& colourName) const;

private:
    bool touchDeviceDetected = false;
    float scaleFactor = 1.0f;

    void detectPlatformFeatures();
};

} // namespace SharedComponents
```

### Implementation of Shared Components

```cpp
// SharedAudioComponent.cpp
#include "SharedAudioComponent.h"

namespace SharedComponents {

// ParameterManager Implementation
ParameterManager::ParameterManager()
{
    // Add default parameters that all targets use
    addParameter({
        "volume", "Volume", 0.0f, 1.0f, 0.5f, 0.5f, "",
        [](float value) { return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
        [](const juce::String& text) { return juce::Decibels::decibelsToGain(text.getFloatValue()); }
    });

    addParameter({
        "frequency", "Frequency", 20.0f, 20000.0f, 440.0f, 440.0f, "Hz",
        [](float value) { return juce::String(value, 0) + " Hz"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    });

    addParameter({
        "waveType", "Wave Type", 0.0f, 3.0f, 0.0f, 0.0f, "",
        [](float value) {
            const char* types[] = {"Sine", "Square", "Sawtooth", "Triangle"};
            return juce::String(types[juce::jlimit(0, 3, (int)value)]);
        },
        [](const juce::String& text) {
            if (text == "Sine") return 0.0f;
            if (text == "Square") return 1.0f;
            if (text == "Sawtooth") return 2.0f;
            if (text == "Triangle") return 3.0f;
            return 0.0f;
        }
    });
}

void ParameterManager::addParameter(const ParameterInfo& info)
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    parameterMap[info.id] = parameters.size();
    parameters.push_back(info);
}

void ParameterManager::setParameter(const juce::String& id, float value)
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        auto& param = parameters[it->second];
        param.currentValue = juce::jlimit(param.minValue, param.maxValue, value);
    }
}

float ParameterManager::getParameter(const juce::String& id) const
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        return parameters[it->second].currentValue;
    }
    return 0.0f;
}

// Template specializations for different parameter types
template<>
std::unique_ptr<juce::AudioParameterFloat>
ParameterManager::createJUCEParameter<juce::AudioParameterFloat>(const juce::String& id) const
{
    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        const auto& info = parameters[it->second];
        return std::make_unique<juce::AudioParameterFloat>(
            info.id, info.name,
            juce::NormalisableRange<float>(info.minValue, info.maxValue),
            info.defaultValue,
            info.units,
            juce::AudioProcessorParameter::genericParameter,
            info.valueToText,
            info.textToValue
        );
    }
    return nullptr;
}

// SharedGUIComponent Implementation
SharedGUIComponent::SharedGUIComponent()
{
    detectPlatformFeatures();
    updateTheme();
}

void SharedGUIComponent::detectPlatformFeatures()
{
    #if JUCE_IOS || JUCE_ANDROID
    touchDeviceDetected = true;
    scaleFactor = 1.2f;
    #else
    touchDeviceDetected = juce::Desktop::getInstance().getMainMouseSource().isTouch();
    scaleFactor = touchDeviceDetected ? 1.2f : 1.0f;
    #endif
}

bool SharedGUIComponent::isTouchDevice() const
{
    return touchDeviceDetected;
}

bool SharedGUIComponent::isMobileDevice() const
{
    #if JUCE_IOS || JUCE_ANDROID
    return true;
    #else
    return false;
    #endif
}

float SharedGUIComponent::getScaleFactor() const
{
    return scaleFactor;
}

juce::Rectangle<int> SharedGUIComponent::getAdaptiveBounds(const juce::Rectangle<int>& baseBounds) const
{
    if (isTouchDevice())
    {
        return baseBounds.expanded(10); // Larger touch targets
    }
    return baseBounds;
}

int SharedGUIComponent::getAdaptiveSize(int baseSize) const
{
    return static_cast<int>(baseSize * scaleFactor);
}

void SharedGUIComponent::paint(juce::Graphics& g)
{
    // Adaptive rendering based on platform
    auto bounds = getLocalBounds();

    // Background
    auto bgColour = getThemeColour("background");
    if (isMobileDevice())
    {
        // Gradient background for mobile
        juce::ColourGradient gradient(
            bgColour.brighter(0.1f), bounds.getTopLeft().toFloat(),
            bgColour.darker(0.1f), bounds.getBottomRight().toFloat(),
            false
        );
        g.setGradientFill(gradient);
    }
    else
    {
        g.setColour(bgColour);
    }
    g.fillAll();

    // Border for desktop
    if (!isMobileDevice())
    {
        g.setColour(getThemeColour("border"));
        g.drawRect(bounds, 1);
    }
}

void SharedGUIComponent::resized()
{
    // Adaptive layout will be implemented by derived classes
}

void SharedGUIComponent::mouseDown(const juce::MouseEvent& event)
{
    #if JUCE_IOS
    // Haptic feedback for iOS
    if (auto* haptics = juce::HapticFeedback::getInstance())
        haptics->impact(juce::HapticFeedback::light);
    #endif

    Component::mouseDown(event);
}

juce::Colour SharedGUIComponent::getThemeColour(const juce::String& colourName) const
{
    // Dark theme optimized for all platforms
    static const std::unordered_map<juce::String, juce::Colour> colours = {
        {"background", juce::Colour(0xff2d3748)},
        {"surface", juce::Colour(0xff4a5568)},
        {"primary", juce::Colour(0xff4299e1)},
        {"secondary", juce::Colour(0xff9f7aea)},
        {"text", juce::Colours::white},
        {"textSecondary", juce::Colour(0xffa0aec0)},
        {"border", juce::Colour(0xff718096)},
        {"success", juce::Colour(0xff48bb78)},
        {"warning", juce::Colour(0xffed8936)},
        {"error", juce::Colour(0xfff56565)}
    };

    auto it = colours.find(colourName);
    return it != colours.end() ? it->second : juce::Colours::grey;
}

std::unique_ptr<juce::AccessibilityHandler> SharedGUIComponent::createAccessibilityHandler()
{
    return std::make_unique<juce::AccessibilityHandler>(
        *this, juce::AccessibilityRole::group);
}

} // namespace SharedComponents
```

### Target-Specific Wrappers

```cpp
// StandaloneWrapper.h - Standalone-specific implementation
#pragma once

#include "SharedAudioComponent.h"
#include <JuceHeader.h>

class StandaloneWrapper : public SharedComponents::AudioProcessorCore,
                         public juce::AudioIODeviceCallback
{
public:
    StandaloneWrapper();
    ~StandaloneWrapper() override;

    // AudioProcessorCore interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    void releaseResources() override;

    void setParameter(const juce::String& paramID, float value) override;
    float getParameter(const juce::String& paramID) const override;

    void saveState(juce::MemoryBlock& destData) override;
    void loadState(const void* data, int sizeInBytes) override;

    // AudioIODeviceCallback interface
    void audioDeviceIOCallback(const float** inputChannelData,
                              int numInputChannels,
                              float** outputChannelData,
                              int numOutputChannels,
                              int numSamples) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // Standalone-specific features
    void setAudioDeviceManager(juce::AudioDeviceManager* manager);
    juce::AudioDeviceManager* getAudioDeviceManager() const { return deviceManager; }

    bool isPlaying() const { return playing; }
    void setPlaying(bool shouldPlay) { playing = shouldPlay; }

private:
    juce::AudioDeviceManager* deviceManager = nullptr;
    SharedComponents::ParameterManager parameterManager;

    // Audio processing
    std::unique_ptr<juce::AudioEngine> audioEngine;
    bool playing = false;

    // Thread safety
    juce::CriticalSection processingLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneWrapper)
};

// PluginWrapper.h - Plugin-specific implementation
#pragma once

#include "SharedAudioComponent.h"
#include <JuceHeader.h>

class PluginWrapper : public SharedComponents::AudioProcessorCore
{
public:
    PluginWrapper(juce::AudioProcessor& processor);
    ~PluginWrapper() override;

    // AudioProcessorCore interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    void releaseResources() override;

    void setParameter(const juce::String& paramID, float value) override;
    float getParameter(const juce::String& paramID) const override;

    void saveState(juce::MemoryBlock& destData) override;
    void loadState(const void* data, int sizeInBytes) override;

    // Plugin-specific features
    juce::AudioProcessor& getProcessor() { return processor; }
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    juce::AudioProcessor& processor;
    juce::AudioProcessorValueTreeState parameters;
    SharedComponents::ParameterManager parameterManager;

    // Parameter synchronization
    void syncParametersWithHost();
    void syncParametersFromHost();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWrapper)
};
```

---

## Cross-Platform Deployment

### Deployment Strategy Overview

Cross-platform deployment requires careful consideration of each platform's requirements, distribution methods, and technical constraints.

#### Platform-Specific Requirements

| Platform | Distribution | Code Signing | Formats | Special Requirements |
|----------|-------------|--------------|---------|---------------------|
| **macOS** | Direct/App Store | Required | VST3, AU, AUv3, Standalone | Notarization for Gatekeeper |
| **Windows** | Direct/Microsoft Store | Optional | VST3, Standalone | Digital signature recommended |
| **iOS** | App Store only | Required | AUv3 only | App Store review process |
| **Linux** | Package managers/Direct | Optional | VST3, Standalone | Multiple distribution formats |

### Code Signing and Notarization

```cmake
# cmake/CodeSigning.cmake - Code signing configuration
function(setup_code_signing)
    if(APPLE)
        # macOS/iOS code signing
        set(CODESIGN_IDENTITY "Developer ID Application: Your Name (TEAM_ID)" CACHE STRING "Code signing identity")
        set(PROVISIONING_PROFILE_SPECIFIER "" CACHE STRING "Provisioning profile for iOS")

        # Find code signing identity
        execute_process(
            COMMAND security find-identity -v -p codesigning
            OUTPUT_VARIABLE AVAILABLE_IDENTITIES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(AVAILABLE_IDENTITIES MATCHES "${CODESIGN_IDENTITY}")
            message(STATUS "Found code signing identity: ${CODESIGN_IDENTITY}")

            # Apply to all targets
            set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODESIGN_IDENTITY}")
            set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}")

            if(PLATFORM_IOS AND PROVISIONING_PROFILE_SPECIFIER)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "${PROVISIONING_PROFILE_SPECIFIER}")
            endif()

            # Enable hardened runtime for notarization
            set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
            set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--timestamp")

        else()
            message(WARNING "Code signing identity not found: ${CODESIGN_IDENTITY}")
        endif()

    elseif(WIN32)
        # Windows code signing
        set(SIGNTOOL_PATH "" CACHE PATH "Path to signtool.exe")
        set(CERTIFICATE_PATH "" CACHE PATH "Path to code signing certificate")
        set(CERTIFICATE_PASSWORD "" CACHE STRING "Certificate password")

        if(SIGNTOOL_PATH AND CERTIFICATE_PATH)
            message(STATUS "Windows code signing configured")
            # Custom post-build step for signing will be added per target
        endif()
    endif()
endfunction()

# Function to sign a specific target
function(sign_target target_name)
    if(APPLE AND ENABLE_CODESIGNING)
        # macOS/iOS signing handled by Xcode
        set_target_properties(${target_name} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODESIGN_IDENTITY}"
        )

        # Add notarization step for macOS
        if(PLATFORM_MACOS)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Notarizing ${target_name}..."
                COMMAND xcrun notarytool submit
                    "$<TARGET_BUNDLE_DIR:${target_name}>"
                    --keychain-profile "notarization-profile"
                    --wait
                COMMENT "Notarizing ${target_name}"
            )
        endif()

    elseif(WIN32 AND SIGNTOOL_PATH AND CERTIFICATE_PATH)
        # Windows signing
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND "${SIGNTOOL_PATH}" sign
                /f "${CERTIFICATE_PATH}"
                /p "${CERTIFICATE_PASSWORD}"
                /t http://timestamp.digicert.com
                /fd SHA256
                "$<TARGET_FILE:${target_name}>"
            COMMENT "Signing ${target_name}"
        )
    endif()
endfunction()
```

### Automated Deployment Scripts

```bash
#!/bin/bash
# deploy.sh - Comprehensive deployment script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
DEPLOY_DIR="$PROJECT_ROOT/deploy"

# Configuration
VERSION="1.0.0"
BUILD_TYPE="Release"
PLATFORMS=("macos" "windows" "ios")
ENABLE_CODESIGNING=false
ENABLE_NOTARIZATION=false
CREATE_INSTALLERS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --platforms)
            IFS=',' read -ra PLATFORMS <<< "$2"
            shift 2
            ;;
        --codesign)
            ENABLE_CODESIGNING=true
            shift
            ;;
        --notarize)
            ENABLE_NOTARIZATION=true
            shift
            ;;
        --installers)
            CREATE_INSTALLERS=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=== JUCE Audio Generator Deployment ==="
echo "Version: $VERSION"
echo "Platforms: ${PLATFORMS[*]}"
echo "Code Signing: $ENABLE_CODESIGNING"
echo "Notarization: $ENABLE_NOTARIZATION"
echo "Create Installers: $CREATE_INSTALLERS"
echo ""

# Create deployment directory
mkdir -p "$DEPLOY_DIR"

# Function to deploy for specific platform
deploy_platform() {
    local platform=$1
    echo "Deploying for $platform..."

    local platform_build_dir="$BUILD_DIR/$platform"
    local platform_deploy_dir="$DEPLOY_DIR/$platform"

    mkdir -p "$platform_deploy_dir"

    case $platform in
        "macos")
            deploy_macos "$platform_build_dir" "$platform_deploy_dir"
            ;;
        "windows")
            deploy_windows "$platform_build_dir" "$platform_deploy_dir"
            ;;
        "ios")
            deploy_ios "$platform_build_dir" "$platform_deploy_dir"
            ;;
        *)
            echo "Unknown platform: $platform"
            return 1
            ;;
    esac
}

deploy_macos() {
    local build_dir=$1
    local deploy_dir=$2

    echo "Deploying macOS targets..."

    # Copy standalone app
    if [ -d "$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.app" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.app" "$deploy_dir/"
        echo "  ✓ Standalone app copied"
    fi

    # Copy VST3
    if [ -d "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3" "$deploy_dir/"
        echo "  ✓ VST3 plugin copied"
    fi

    # Copy AU
    if [ -d "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/AU/JUCEAudioGenerator.component" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/AU/JUCEAudioGenerator.component" "$deploy_dir/"
        echo "  ✓ AU plugin copied"
    fi

    # Copy AUv3
    if [ -d "$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/JUCEAudioGenerator AUv3.appex" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/JUCEAudioGenerator AUv3.appex" "$deploy_dir/"
        echo "  ✓ AUv3 plugin copied"
    fi

    # Create installer if requested
    if [ "$CREATE_INSTALLERS" = true ]; then
        create_macos_installer "$deploy_dir"
    fi
}

deploy_windows() {
    local build_dir=$1
    local deploy_dir=$2

    echo "Deploying Windows targets..."

    # Copy standalone app
    if [ -f "$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.exe" ]; then
        cp "$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.exe" "$deploy_dir/"
        echo "  ✓ Standalone app copied"
    fi

    # Copy VST3
    if [ -d "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3" "$deploy_dir/"
        echo "  ✓ VST3 plugin copied"
    fi

    # Create installer if requested
    if [ "$CREATE_INSTALLERS" = true ]; then
        create_windows_installer "$deploy_dir"
    fi
}

deploy_ios() {
    local build_dir=$1
    local deploy_dir=$2

    echo "Deploying iOS targets..."

    # Copy AUv3 app
    if [ -d "$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE-iphoneos/JUCEAudioGenerator AUv3.app" ]; then
        cp -R "$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE-iphoneos/JUCEAudioGenerator AUv3.app" "$deploy_dir/"
        echo "  ✓ AUv3 iOS app copied"
    fi

    # Create IPA if requested
    if [ "$CREATE_INSTALLERS" = true ]; then
        create_ios_ipa "$deploy_dir"
    fi
}

create_macos_installer() {
    local deploy_dir=$1
    echo "Creating macOS installer..."

    # Create installer package using pkgbuild
    pkgbuild --root "$deploy_dir" \
             --identifier "com.finitud-labs.juceaudiogenerator" \
             --version "$VERSION" \
             --install-location "/Applications/JUCE Audio Generator" \
             "$deploy_dir/JUCEAudioGenerator-$VERSION-macOS.pkg"

    echo "  ✓ macOS installer created"
}

create_windows_installer() {
    local deploy_dir=$1
    echo "Creating Windows installer..."

    # Use NSIS or Inno Setup to create installer
    # This would require the installer script to be pre-configured
    echo "  ⚠ Windows installer creation not implemented"
}

create_ios_ipa() {
    local deploy_dir=$1
    echo "Creating iOS IPA..."

    # Create IPA from app bundle
    local app_path="$deploy_dir/JUCEAudioGenerator AUv3.app"
    if [ -d "$app_path" ]; then
        mkdir -p "$deploy_dir/Payload"
        cp -R "$app_path" "$deploy_dir/Payload/"
        cd "$deploy_dir"
        zip -r "JUCEAudioGenerator-$VERSION-iOS.ipa" Payload/
        rm -rf Payload/
        echo "  ✓ iOS IPA created"
    fi
}

# Deploy for each platform
for platform in "${PLATFORMS[@]}"; do
    deploy_platform "$platform"
done

# Create distribution archive
echo ""
echo "Creating distribution archive..."
cd "$DEPLOY_DIR"
tar -czf "JUCEAudioGenerator-$VERSION-All-Platforms.tar.gz" */

echo ""
echo "=== Deployment Summary ==="
echo "Deployment directory: $DEPLOY_DIR"
echo "Distribution archive: JUCEAudioGenerator-$VERSION-All-Platforms.tar.gz"
echo

echo "Deployment completed successfully!"
```

---

## Platform-Specific Optimizations

### iOS/iPad Touch Interface Optimizations

```cpp
// TouchInterface.h - iOS-specific touch optimizations
#pragma once

#include <JuceHeader.h>

#if JUCE_IOS

class TouchInterface
{
public:
    TouchInterface();
    ~TouchInterface();

    // Touch gesture recognition
    struct TouchGesture
    {
        enum Type { Tap, DoubleTap, LongPress, Pinch, Pan, Swipe };
        Type type;
        juce::Point<float> position;
        float velocity;
        float scale;
        int touchCount;
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void touchGestureDetected(const TouchGesture& gesture) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    // Touch event processing
    void processTouchEvent(const juce::MouseEvent& event);
    void processTouchMove(const juce::MouseEvent& event);
    void processTouchEnd(const juce::MouseEvent& event);

    // Haptic feedback
    void triggerHapticFeedback(juce::HapticFeedback::Type type);

    // Accessibility
    void announceForAccessibility(const juce::String& announcement);

private:
    juce::ListenerList<Listener> listeners;

    // Gesture detection state
    struct TouchState
    {
        juce::Point<float> startPosition;
        juce::Point<float> currentPosition;
        juce::Time startTime;
        bool isActive = false;
        int touchId = -1;
    };

    std::vector<TouchState> activeTouches;
    juce::Time lastTapTime;
    juce::Point<float> lastTapPosition;

    // Gesture thresholds
    static constexpr float LONG_PRESS_DURATION = 0.5f; // seconds
    static constexpr float DOUBLE_TAP_INTERVAL = 0.3f; // seconds
    static constexpr float SWIPE_THRESHOLD = 50.0f; // pixels
    static constexpr float TAP_THRESHOLD = 10.0f; // pixels

    void detectGestures();
    bool isDoubleTap(const juce::Point<float>& position, juce::Time time) const;
    TouchGesture::Type classifySwipe(const juce::Point<float>& start,
                                    const juce::Point<float>& end) const;
};

// Touch-optimized component base class
class TouchOptimizedComponent : public juce::Component,
                               public TouchInterface::Listener
{
public:
    TouchOptimizedComponent();
    ~TouchOptimizedComponent() override;

    // Component interface
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // TouchInterface::Listener
    void touchGestureDetected(const TouchInterface::TouchGesture& gesture) override;

    // Touch-specific virtual methods
    virtual void handleTap(const juce::Point<float>& position) {}
    virtual void handleDoubleTap(const juce::Point<float>& position) {}
    virtual void handleLongPress(const juce::Point<float>& position) {}
    virtual void handlePinch(float scale, const juce::Point<float>& center) {}
    virtual void handlePan(const juce::Point<float>& delta) {}
    virtual void handleSwipe(const juce::Point<float>& direction) {}

protected:
    TouchInterface touchInterface;

    // Touch state
    bool isTouchActive = false;
    juce::Point<float> touchStartPosition;

    // Visual feedback
    void showTouchFeedback(const juce::Point<float>& position);
    void hideTouchFeedback();

private:
    std::unique_ptr<juce::Component> touchFeedbackComponent;
};

#endif // JUCE_IOS
```

### Performance Optimizations for Mobile

```cpp
// MobileOptimizations.h - Mobile-specific performance optimizations
#pragma once

#include <JuceHeader.h>

namespace MobileOptimizations {

// Memory management for mobile devices
class MobileMemoryManager
{
public:
    static MobileMemoryManager& getInstance();

    // Memory pressure handling
    void handleMemoryWarning();
    void optimizeForLowMemory();
    void restoreNormalMemoryUsage();

    // Resource management
    void preloadCriticalResources();
    void unloadNonCriticalResources();

    // Cache management
    void clearImageCache();
    void clearAudioBufferCache();
    void clearUnusedResources();

    // Memory monitoring
    size_t getCurrentMemoryUsage() const;
    size_t getAvailableMemory() const;
    bool isMemoryPressureHigh() const;

private:
    MobileMemoryManager() = default;

    bool lowMemoryMode = false;
    size_t memoryThreshold = 50 * 1024 * 1024; // 50MB threshold

    void registerMemoryWarningCallback();
};

// Battery optimization
class BatteryOptimizer
{
public:
    enum PowerState { Normal, PowerSaver, Critical };

    static BatteryOptimizer& getInstance();

    PowerState getCurrentPowerState() const;
    void optimizeForPowerState(PowerState state);

    // CPU optimization
    void reduceCPUUsage();
    void restoreNormalCPUUsage();

    // Display optimization
    void reduceDisplayUpdates();
    void restoreNormalDisplayUpdates();

    // Audio optimization
    void optimizeAudioProcessing();
    void restoreNormalAudioProcessing();

private:
    BatteryOptimizer() = default;

    PowerState currentState = Normal;
    int normalUpdateRate = 60; // FPS
    int powerSaverUpdateRate = 30; // FPS
    int criticalUpdateRate = 15; // FPS
};

// Network optimization for cloud features
class NetworkOptimizer
{
public:
    enum NetworkType { WiFi, Cellular, None };

    static NetworkOptimizer& getInstance();

    NetworkType getCurrentNetworkType() const;
    bool isNetworkAvailable() const;
    bool isHighSpeedNetwork() const;

    // Adaptive quality based on network
    void optimizeForNetwork(NetworkType type);

    // Data usage optimization
    void enableDataSaver();
    void disableDataSaver();
    bool isDataSaverEnabled() const;

private:
    NetworkOptimizer() = default;

    NetworkType currentNetwork = None;
    bool dataSaverEnabled = false;
};

// iOS-specific optimizations
#if JUCE_IOS

class iOSOptimizations
{
public:
    static void optimizeForDevice();
    static void handleAppStateChange(bool isActive);
    static void optimizeForOrientation(bool isPortrait);

    // Device-specific optimizations
    static void optimizeForPhone();
    static void optimizeForPad();

    // iOS version optimizations
    static void enableModernFeatures();
    static void fallbackForOlderVersions();

    // Background processing
    static void prepareForBackground();
    static void restoreFromBackground();

private:
    static bool isPhone();
    static bool isPad();
    static int getIOSVersion();
};

#endif // JUCE_IOS

} // namespace MobileOptimizations
```

---

## C++20 Features Introduction

### Concepts for Audio Processing

C++20 concepts provide a powerful way to constrain templates and improve code clarity, especially useful in audio processing contexts.

```cpp
// AudioConcepts.h - C++20 concepts for audio processing
#pragma once

#include <concepts>
#include <type_traits>

namespace AudioConcepts {

// Basic audio sample type concept
template<typename T>
concept AudioSample = std::floating_point<T> &&
                     (std::same_as<T, float> || std::same_as<T, double>);

// Audio buffer concept
template<typename T>
concept AudioBuffer = requires(T buffer) {
    { buffer.getNumChannels() } -> std::convertible_to<int>;
    { buffer.getNumSamples() } -> std::convertible_to<int>;
    { buffer.getReadPointer(0) } -> std::convertible_to<const float*>;
    { buffer.getWritePointer(0) } -> std::convertible_to<float*>;
};

// Audio processor concept
template<typename T>
concept AudioProcessor = requires(T processor, juce::AudioBuffer<float>& buffer) {
    { processor.prepareToPlay(44100.0, 512) } -> std::same_as<void>;
    { processor.processBlock(buffer) } -> std::same_as<void>;
    { processor.releaseResources() } -> std::same_as<void>;
};

// Parameter concept
template<typename T>
concept Parameter = requires(T param) {
    { param.getValue() } -> AudioSample;
    { param.setValue(0.5f) } -> std::same_as<void>;
    { param.getRange() } -> std::convertible_to<std::pair<float, float>>;
};

// Real-time safe concept
template<typename T>
concept RealtimeSafe = requires {
    // Must not allocate memory
    requires !requires(T t) { new T; };
    // Must not use blocking operations
    requires !requires(T t) { t.lock(); };
    // Must be noexcept for critical operations
    requires noexcept(T{});
};

// Example usage with concepts
template<AudioSample SampleType, AudioBuffer BufferType>
class ConceptAwareProcessor
{
public:
    void processBlock(BufferType& buffer)
    {
        static_assert(AudioBuffer<BufferType>, "Buffer must satisfy AudioBuffer concept");

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] = processSample(channelData[sample]);
            }
        }
    }

private:
    SampleType processSample(SampleType input) requires AudioSample<SampleType>
    {
        return input * static_cast<SampleType>(0.5);
    }
};

} // namespace AudioConcepts
```

### Modules Basics for Code Organization

C++20 modules provide better encapsulation and faster compilation times.

```cpp
// AudioEngine.cppm - Module interface
export module AudioEngine;

import <memory>;
import <vector>;
import <atomic>;

// Export the main audio engine interface
export namespace AudioEngine {

class Engine
{
public:
    Engine();
    ~Engine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(float* const* outputChannelData, int numChannels, int numSamples);
    void releaseResources();

    // Parameter management
    void setParameter(int parameterIndex, float value);
    float getParameter(int parameterIndex) const;

private:
    struct Impl; // Hide implementation details
    std::unique_ptr<Impl> pImpl;
};

// Export utility functions
export float decibelToGain(float decibels);
export float gainToDecibel(float gain);
export double noteToFrequency(int midiNote);

} // namespace AudioEngine
```

```cpp
// AudioEngine.cpp - Module implementation
module AudioEngine;

import <cmath>;
import <algorithm>;

namespace AudioEngine {

struct Engine::Impl
{
    double sampleRate = 44100.0;
    int blockSize = 512;
    std::vector<std::atomic<float>> parameters;

    // Audio processing state
    float phase = 0.0f;
    float frequency = 440.0f;
    float volume = 0.5f;

    Impl() : parameters(3) // volume, frequency, waveType
    {
        parameters[0].store(0.5f); // volume
        parameters[1].store(440.0f); // frequency
        parameters[2].store(0.0f); // waveType
    }
};

Engine::Engine() : pImpl(std::make_unique<Impl>()) {}
Engine::~Engine() = default;

void Engine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    pImpl->sampleRate = sampleRate;
    pImpl->blockSize = samplesPerBlock;
    pImpl->phase = 0.0f;
}

void Engine::processBlock(float* const* outputChannelData, int numChannels, int numSamples)
{
    // Update parameters from atomic storage
    pImpl->volume = pImpl->parameters[0].load();
    pImpl->frequency = pImpl->parameters[1].load();

    const float phaseIncrement = pImpl->frequency * 2.0f * M_PI / pImpl->sampleRate;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float sampleValue = std::sin(pImpl->phase) * pImpl->volume;
        pImpl->phase += phaseIncrement;

        // Wrap phase
        if (pImpl->phase >= 2.0f * M_PI)
            pImpl->phase -= 2.0f * M_PI;

        // Copy to all output channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            outputChannelData[channel][sample] = sampleValue;
        }
    }
}

void Engine::setParameter(int parameterIndex, float value)
{
    if (parameterIndex >= 0 && parameterIndex < pImpl->parameters.size())
    {
        pImpl->parameters[parameterIndex].store(value);
    }
}

float Engine::getParameter(int parameterIndex) const
{
    if (parameterIndex >= 0 && parameterIndex < pImpl->parameters.size())
    {
        return pImpl->parameters[parameterIndex].load();
    }
    return 0.0f;
}

// Utility function implementations
float decibelToGain(float decibels)
{
    return std::pow(10.0f, decibels / 20.0f);
}

float gainToDecibel(float gain)
{
    return 20.0f * std::log10(std::max(gain, 1e-6f));
}

double noteToFrequency(int midiNote)
{
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

} // namespace AudioEngine
```

### Coroutines for Async Operations

```cpp
// AsyncOperations.h - C++20 coroutines for async audio operations
#pragma once

#include <coroutine>
#include <future>
#include <chrono>

namespace AsyncAudio {

// Simple task coroutine
template<typename T>
struct Task
{
    struct promise_type
    {
        T value;
        std::exception_ptr exception;

        Task get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_value(T val) { value = std::move(val); }
        void unhandled_exception() { exception = std::current_exception(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }

    T get()
    {
        if (handle.promise().exception)
            std::rethrow_exception(handle.promise().exception);
        return handle.promise().value;
    }
};

// Async audio file loading
Task<std::unique_ptr<juce::AudioBuffer<float>>> loadAudioFileAsync(const juce::String& filePath)
{
    // Simulate async file loading
    co_await std::suspend_always{};

    juce::File audioFile(filePath);
    if (!audioFile.existsAsFile())
    {
        co_return nullptr;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    auto reader = std::unique_ptr<juce::AudioFormatReader>(
        formatManager.createReaderFor(audioFile));

    if (reader == nullptr)
    {
        co_return nullptr;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(
        reader->numChannels,
        static_cast<int>(reader->lengthInSamples));

    reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

    co_return buffer;
}

// Async parameter automation
Task<void> animateParameterAsync(std::atomic<float>& parameter,
                                float targetValue,
                                std::chrono::milliseconds duration)
{
    const float startValue = parameter.load();
    const float valueRange = targetValue - startValue;
    const auto startTime = std::chrono::steady_clock::now();

    while (true)
    {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime);

        if (elapsed >= duration)
        {
            parameter.store(targetValue);
            break;
        }

        float progress = static_cast<float>(elapsed.count()) / duration.count();
        float currentValue = startValue + (valueRange * progress);
        parameter.store(currentValue);

        co_await std::suspend_always{};
    }

    co_return;
}

} // namespace AsyncAudio
```

---

## Practical Implementation

Now let's implement the enhanced build system and create the necessary files for our dual-target setup.

### Enhanced CMakeLists.txt Implementation

Let's update our existing CMakeLists.txt to support the dual-target architecture:

```cmake
# Enhanced CMakeLists.txt with full dual-target support
cmake_minimum_required(VERSION 3.22)

project(JUCEAudioGenerator VERSION 1.0.0)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_STANDALONE "Build standalone application" ON)
option(BUILD_VST3 "Build VST3 plugin" ON)
option(BUILD_AU "Build Audio Unit plugin" ON)
option(BUILD_AUV3 "Build AUv3 plugin for iOS/macOS" OFF)
option(BUILD_AAX "Build AAX plugin" OFF)
option(ENABLE_CODESIGNING "Enable code signing" OFF)
option(ENABLE_TESTING "Enable unit tests" OFF)

# Platform detection
if(APPLE)
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(PLATFORM_IOS TRUE)
        set(BUILD_AUV3 ON)
        set(BUILD_STANDALONE OFF)
        set(BUILD_VST3 OFF)
        set(BUILD_AU OFF)
    else()
        set(PLATFORM_MACOS TRUE)
    endif()
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    set(BUILD_AU OFF) # AU not available on Windows
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    set(BUILD_AU OFF) # AU not available on Linux
endif()

# JUCE setup
find_package(PkgConfig REQUIRED)
add_subdirectory(JUCE)

# Project configuration
set(PLUGIN_NAME "JUCEAudioGenerator")
set(PLUGIN_MANUFACTURER "finitud-labs")
set(PLUGIN_DESCRIPTION "Professional dual-target audio generator with AUv3 support")
set(PLUGIN_CATEGORY "Generator")

# Shared source files
set(SHARED_SOURCES
    Source/AudioEngine.cpp
    Source/SharedAudioComponent.cpp
)

# Platform-specific sources
set(PLATFORM_SOURCES)
if(PLATFORM_IOS)
    list(APPEND PLATFORM_SOURCES
        Source/iOS/TouchInterface.cpp
        Source/iOS/MobileOptimizations.cpp
    )
elseif(PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        Source/macOS/NativeIntegration.cpp
    )
elseif(PLATFORM_WINDOWS)
    list(APPEND PLATFORM_SOURCES
        Source/Windows/WindowsOptimizations.cpp
    )
elseif(PLATFORM_LINUX)
    list(APPEND PLATFORM_SOURCES
        Source/Linux/LinuxOptimizations.cpp
    )
endif()

# Common target configuration function
function(configure_common_target target_name)
    target_sources(${target_name} PRIVATE ${SHARED_SOURCES} ${PLATFORM_SOURCES})

    target_link_libraries(${target_name} PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
    )

    target_compile_definitions(${target_name} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=1
    )

    # Platform-specific definitions
    if(PLATFORM_IOS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_IOS=1
            JUCE_MOBILE=1
            TARGET_AUV3=1
        )
    elseif(PLATFORM_MACOS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_MAC=1
            PLATFORM_DESKTOP=1
        )
    elseif(PLATFORM_WINDOWS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_WINDOWS=1
            PLATFORM_DESKTOP=1
        )
    elseif(PLATFORM_LINUX)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_LINUX=1
            PLATFORM_DESKTOP=1
        )
    endif()

    # C++20 features
    target_compile_features(${target_name} PRIVATE cxx_std_20)

    # Optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target_name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL /arch:AVX2>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -flto -march=native>
        )

        target_link_options(${target_name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/LTCG>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-flto>
        )
    endif()
endfunction()

# Plugin targets (VST3, AU, AAX)
if(BUILD_VST3 OR BUILD_AU OR BUILD_AAX)
    set(PLUGIN_FORMATS)
    if(BUILD_VST3)
        list(APPEND PLUGIN_FORMATS VST3)
    endif()
    if(BUILD_AU AND APPLE)
        list(APPEND PLUGIN_FORMATS AU)
    endif()
    if(BUILD_AAX)
        list(APPEND PLUGIN_FORMATS AAX)
    endif()

    juce_add_plugin(${PLUGIN_NAME}
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JAG5"
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME "${PLUGIN_NAME}"
        DESCRIPTION "${PLUGIN_DESCRIPTION}"
        PLUGIN_CATEGORY_ID "kPluginCategGenerator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES "Generator"
        AU_MAIN_TYPE "kAudioUnitType_Generator"
        AU_SANDBOX_SAFE TRUE
    )

    target_sources(${PLUGIN_NAME} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
    )

    configure_common_target(${PLUGIN_NAME})

    target_link_libraries(${PLUGIN_NAME} PRIVATE
        juce::juce_audio_plugin_client
    )

    target_compile_definitions(${PLUGIN_NAME} PRIVATE
        TARGET_PLUGIN=1
    )
endif()

# AUv3 target for iOS/macOS
if(BUILD_AUV3 AND APPLE)
    juce_add_plugin(${PLUGIN_NAME}_AUv3
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JA3V"
        FORMATS AUv3
        PRODUCT_NAME "${PLUGIN_NAME} AUv3"
        DESCRIPTION "${PLUGIN_DESCRIPTION} for iOS and macOS"
        PLUGIN_CATEGORY_ID "kAudioUnitType_Generator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        AUV3_CATEGORY "Generator"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator.auv3"
        AU_SANDBOX_SAFE TRUE
    )

    target_sources(${PLUGIN_NAME}_AUv3 PRIVATE
        Source/PluginProcessor.cpp
        Source/AUv3/AUv3Editor.cpp
    )

    configure_common_target(${PLUGIN_NAME}_AUv3)

    target_link_libraries(${PLUGIN_NAME}_AUv3 PRIVATE
        juce::juce_audio_plugin_client
    )

    target_compile_definitions(${PLUGIN_NAME}_AUv3 PRIVATE
        TARGET_AUV3=1
        TARGET_MOBILE_PLUGIN=1
    )

    # iOS-specific configuration
    if(PLATFORM_IOS)
        set_target_properties(${PLUGIN_NAME}_AUv3 PROPERTIES
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
            XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
        )
    endif()
endif()

# Standalone application
if(BUILD_STANDALONE)
    juce_add_gui_app(${PLUGIN_NAME}_Standalone
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PRODUCT_NAME "${PLUGIN_NAME}"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator"
    )

    target_sources(${PLUGIN_NAME}_Standalone PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Standalone/StandaloneWrapper.cpp
    )

    configure_common_target(${PLUGIN_NAME}_Standalone)

    target_compile_definitions(${PLUGIN_NAME}_Standalone PRIVATE
        TARGET_STANDALONE=1
    )
endif()

# Code signing
if(ENABLE_CODESIGNING AND APPLE)
    include(cmake/CodeSigning.cmake)
    setup_code_signing()

    if(TARGET ${PLUGIN_NAME})
        sign_target(${PLUGIN_NAME})
    endif()

    if(TARGET ${PLUGIN_NAME}_AUv3)
        sign_target(${PLUGIN_NAME}_AUv3)
    endif()

    if(TARGET ${PLUGIN_NAME}_Standalone)
        sign_target(${PLUGIN_NAME}_Standalone)
    endif()
endif()

# Testing
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(Tests)
endif()

# Custom targets
add_custom_target(build_all_targets
    DEPENDS
        $<$<BOOL:${BUILD_STANDALONE}>:${PLUGIN_NAME}_Standalone>
        $<$<BOOL:${BUILD_VST3}>:${PLUGIN_NAME}>
        $<$<BOOL:${BUILD_AUV3}>:${PLUGIN_NAME}_AUv3>
)

# Installation and packaging
include(GNUInstallDirs)
include(cmake/Installation.cmake)
include(cmake/Packaging.cmake)

# Print build configuration
message(STATUS "=== JUCE Audio Generator Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Standalone: ${BUILD_STANDALONE}")
message(STATUS "VST3: ${BUILD_VST3}")
message(STATUS "AU: ${BUILD_AU}")
message(STATUS "AUv3: ${BUILD_AUV3}")
message(STATUS "AAX: ${BUILD_AAX}")
message(STATUS "Code Signing: ${ENABLE_CODESIGNING}")
message(STATUS "Testing: ${ENABLE_TESTING}")
message(STATUS "===============================================")
```

---

## Practical Exercises

### Exercise 1: Build and Deploy Multi-Target Setup

**Objective**: Successfully configure and build all target formats from a single codebase.

**Steps**:
1. Update your [`CMakeLists.txt`](CMakeLists.txt:1) with the enhanced multi-target configuration
2. Create the shared component architecture files
3. Build for multiple platforms using the provided build scripts
4. Test each target format in appropriate hosts/environments
5. Verify parameter synchronization across all targets

**Expected Result**: All target formats build successfully and function identically.

**Code Challenge**: Add a new parameter that automatically adapts its behavior based on the target type.

### Exercise 2: Implement AUv3 for iPad

**Objective**: Create a touch-optimized AUv3 plugin for iPad deployment.

**Steps**:
1. Configure iOS build environment and provisioning
2. Implement touch-optimized GUI components
3. Add haptic feedback and accessibility features
4. Test on iPad simulator and device
5. Submit to App Store Connect for testing

**Expected Result**: Fully functional AUv3 plugin optimized for iPad use.

**Code Challenge**: Implement gesture-based parameter control (pinch-to-zoom, swipe gestures).

### Exercise 3: Cross-Platform Deployment Pipeline

**Objective**: Create an automated deployment pipeline that builds and packages for all platforms.

**Steps**:
1. Set up continuous integration (GitHub Actions/GitLab CI)
2. Configure automated builds for macOS, Windows, and iOS
3. Implement code signing and notarization
4. Create distribution packages for each platform
5. Set up automated testing across all targets

**Expected Result**: Fully automated deployment pipeline with signed, notarized packages.

**Code Challenge**: Add automated plugin validation using pluginval or similar tools.

### Exercise 4: Performance Optimization and Profiling

**Objective**: Optimize the dual-target setup for maximum performance across all platforms.

**Steps**:
1. Profile CPU usage on desktop and mobile platforms
2. Implement platform-specific optimizations
3. Add memory usage monitoring and optimization
4. Test real-time performance under various conditions
5. Optimize for battery life on mobile devices

**Expected Result**: Optimized performance with minimal CPU usage and memory footprint.

**Code Challenge**: Implement adaptive quality settings based on device capabilities.

---

## Deployment and Distribution

### App Store Deployment (iOS AUv3)

```xml
<!-- App Store metadata configuration -->
<!-- AppStoreMetadata.xml -->
<?xml version="1.0" encoding="UTF-8"?>
<metadata>
    <app_info>
        <name>JUCE Audio Generator AUv3</name>
        <bundle_id>com.finitud-labs.juceaudiogenerator.auv3</bundle_id>
        <version>1.0.0</version>
        <build_number>1</build_number>
        <category>Music</category>
        <subcategory>Audio Unit Extensions</subcategory>
    </app_info>

    <description>
        <short>Professional audio generator plugin for iOS</short>
        <long>
            JUCE Audio Generator AUv3 is a professional-grade audio generator plugin
            designed specifically for iOS devices. Features include:

            • Touch-optimized interface for iPad and iPhone
            • Multiple waveform types (Sine, Square, Sawtooth, Triangle)
            • Real-time parameter control with haptic feedback
            • AUv3 compatibility with all major iOS DAWs
            • Optimized for low latency and battery efficiency
            • Accessibility support for VoiceOver users

            Perfect for music producers, sound designers, and audio enthusiasts
            who want professional audio generation capabilities on their iOS devices.
        </long>
    </description>

    <keywords>
        <keyword>audio</keyword>
        <keyword>generator</keyword>
        <keyword>synthesizer</keyword>
        <keyword>music</keyword>
        <keyword>AUv3</keyword>
        <keyword>plugin</keyword>
        <keyword>professional</keyword>
    </keywords>

    <screenshots>
        <screenshot device="ipad" size="2048x2732">screenshots/ipad_portrait.png</screenshot>
        <screenshot device="ipad" size="2732x2048">screenshots/ipad_landscape.png</screenshot>
        <screenshot device="iphone" size="1284x2778">screenshots/iphone_portrait.png</screenshot>
    </screenshots>

    <privacy>
        <data_collection>false</data_collection>
        <third_party_analytics>false</third_party_analytics>
        <user_tracking>false</user_tracking>
    </privacy>
</metadata>
```

### Plugin Directory Installation

```bash
#!/bin/bash
# install_plugins.sh - Plugin installation script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
PLUGIN_NAME="JUCEAudioGenerator"

echo "=== JUCE Audio Generator Plugin Installation ==="

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
    AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
    AUV3_DIR="$HOME/Library/Audio/Plug-Ins/Components"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
    VST3_DIR="$PROGRAMFILES/Common Files/VST3"
    AU_DIR=""
    AUV3_DIR=""
else
    PLATFORM="linux"
    VST3_DIR="$HOME/.vst3"
    AU_DIR=""
    AUV3_DIR=""
fi

echo "Platform: $PLATFORM"
echo ""

# Function to install plugin
install_plugin() {
    local plugin_path=$1
    local dest_dir=$2
    local plugin_type=$3

    if [ -d "$plugin_path" ] || [ -f "$plugin_path" ]; then
        echo "Installing $plugin_type plugin..."

        # Create destination directory if it doesn't exist
        mkdir -p "$dest_dir"

        # Copy plugin
        if [[ "$OSTYPE" == "darwin"* ]]; then
            cp -R "$plugin_path" "$dest_dir/"
        else
            cp -r "$plugin_path" "$dest_dir/"
        fi

        echo "  ✓ $plugin_type installed to $dest_dir"
    else
        echo "  ⚠ $plugin_type plugin not found at $plugin_path"
    fi
}

# Install VST3
if [ -n "$VST3_DIR" ]; then
    VST3_PATH="$BUILD_DIR/$PLATFORM/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
    install_plugin "$VST3_PATH" "$VST3_DIR" "VST3"
fi

# Install AU (macOS only)
if [ -n "$AU_DIR" ] && [[ "$PLATFORM" == "macos" ]]; then
    AU_PATH="$BUILD_DIR/$PLATFORM/${PLUGIN_NAME}_artefacts/Release/AU/${PLUGIN_NAME}.component"
    install_plugin "$AU_PATH" "$AU_DIR" "AU"
fi

# Install AUv3 (macOS only)
if [ -n "$AUV3_DIR" ] && [[ "$PLATFORM" == "macos" ]]; then
    AUV3_PATH="$BUILD_DIR/$PLATFORM/${PLUGIN_NAME}_AUv3_artefacts/Release/AUv3/${PLUGIN_NAME} AUv3.appex"
    install_plugin "$AUV3_PATH" "$AUV3_DIR" "AUv3"
fi

echo ""
echo "Installation completed!"
echo ""
echo "Next steps:"
echo "1. Restart your DAW"
echo "2. Rescan plugins if necessary"
echo "3. Look for '$PLUGIN_NAME' in your plugin list"
echo ""

# Plugin validation
echo "=== Plugin Validation ==="
if command -v pluginval &> /dev/null; then
    echo "Running pluginval validation..."

    if [ -d "$VST3_DIR/${PLUGIN_NAME}.vst3" ]; then
        pluginval --validate "$VST3_DIR/${PLUGIN_NAME}.vst3"
    fi

    if [[ "$PLATFORM" == "macos" ]] && [ -d "$AU_DIR/${PLUGIN_NAME}.component" ]; then
        pluginval --validate "$AU_DIR/${PLUGIN_NAME}.component"
    fi
else
    echo "pluginval not found. Install it for automated plugin validation:"
    echo "https://github.com/Tracktion/pluginval"
fi

echo ""
echo "Installation and validation completed!"
```

### Distribution Package Creation

```bash
#!/bin/bash
# create_distribution.sh - Create distribution packages

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/distribution"
VERSION="1.0.0"
PLUGIN_NAME="JUCEAudioGenerator"

echo "=== Creating Distribution Packages ==="
echo "Version: $VERSION"
echo ""

# Create distribution directory
mkdir -p "$DIST_DIR"

# Function to create platform-specific package
create_package() {
    local platform=$1
    local package_dir="$DIST_DIR/$platform"

    echo "Creating $platform package..."
    mkdir -p "$package_dir"

    case $platform in
        "macos")
            create_macos_package "$package_dir"
            ;;
        "windows")
            create_windows_package "$package_dir"
            ;;
        "ios")
            create_ios_package "$package_dir"
            ;;
    esac
}

create_macos_package() {
    local package_dir=$1
    local build_path="$BUILD_DIR/macos"

    # Copy binaries
    mkdir -p "$package_dir/Plugins/VST3"
    mkdir -p "$package_dir/Plugins/AU"
    mkdir -p "$package_dir/Plugins/AUv3"
    mkdir -p "$package_dir/Applications"

    # VST3
    if [ -d "$build_path/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" "$package_dir/Plugins/VST3/"
    fi

    # AU
    if [ -d "$build_path/${PLUGIN_NAME}_artefacts/Release/AU/${PLUGIN_NAME}.component" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_artefacts/Release/AU/${PLUGIN_NAME}.component" "$package_dir/Plugins/AU/"
    fi

    # AUv3
    if [ -d "$build_path/${PLUGIN_NAME}_AUv3_artefacts/Release/AUv3/${PLUGIN_NAME} AUv3.appex" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_AUv3_artefacts/Release/AUv3/${PLUGIN_NAME} AUv3.appex" "$package_dir/Plugins/AUv3/"
    fi

    # Standalone
    if [ -d "$build_path/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}.app" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}.app" "$package_dir/Applications/"
    fi

    # Documentation and extras
    cp "$PROJECT_ROOT/README.md" "$package_dir/"
    cp "$PROJECT_ROOT/LICENSE" "$package_dir/" 2>/dev/null || true
    cp "$PROJECT_ROOT/install_plugins.sh" "$package_dir/"

    # Create installer package
    create_macos_installer "$package_dir"

    # Create DMG
    create_macos_dmg "$package_dir"
}

create_macos_installer() {
    local package_dir=$1

    echo "  Creating macOS installer package..."

    # Create installer using pkgbuild
    pkgbuild --root "$package_dir" \
             --identifier "com.finitud-labs.juceaudiogenerator" \
             --version "$VERSION" \
             --install-location "/tmp/JUCEAudioGenerator" \
             --scripts "$PROJECT_ROOT/installer/macos/scripts" \
             "$package_dir/${PLUGIN_NAME}-${VERSION}-macOS.pkg"
}

create_macos_dmg() {
    local package_dir=$1

    echo "  Creating macOS DMG..."

    # Create DMG using hdiutil
    hdiutil create -volname "${PLUGIN_NAME} ${VERSION}" \
                   -srcfolder "$package_dir" \
                   -ov -format UDZO \
                   "$DIST_DIR/${PLUGIN_NAME}-${VERSION}-macOS.dmg"
}

create_windows_package() {
    local package_dir=$1
    local build_path="$BUILD_DIR/windows"

    # Copy binaries
    mkdir -p "$package_dir/Plugins/VST3"
    mkdir -p "$package_dir/Applications"

    # VST3
    if [ -d "$build_path/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" "$package_dir/Plugins/VST3/"
    fi

    # Standalone
    if [ -f "$build_path/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}.exe" ]; then
        cp "$build_path/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}.exe" "$package_dir/Applications/"
    fi

    # Documentation
    cp "$PROJECT_ROOT/README.md" "$package_dir/"
    cp "$PROJECT_ROOT/LICENSE" "$package_dir/" 2>/dev/null || true

    # Create ZIP archive
    cd "$package_dir"
    zip -r "../${PLUGIN_NAME}-${VERSION}-Windows.zip" .
    cd - > /dev/null
}

create_ios_package() {
    local package_dir=$1
    local build_path="$BUILD_DIR/ios"

    # Copy AUv3 app
    if [ -d "$build_path/${PLUGIN_NAME}_AUv3_artefacts/Release-iphoneos/${PLUGIN_NAME} AUv3.app" ]; then
        cp -R "$build_path/${PLUGIN_NAME}_AUv3_artefacts/Release-iphoneos/${PLUGIN_NAME} AUv3.app" "$package_dir/"

        # Create IPA
        mkdir -p "$package_dir/Payload"
        cp -R "$package_dir/${PLUGIN_NAME} AUv3.app" "$package_dir/Payload/"
        cd "$package_dir"
        zip -r "${PLUGIN_NAME}-${VERSION}-iOS.ipa" Payload/
        rm -rf Payload/
        cd - > /dev/null
    fi
}

# Create packages for available platforms
if [ -d "$BUILD_DIR/macos" ]; then
    create_package "macos"
fi

if [ -d "$BUILD_DIR/windows" ]; then
    create_package "windows"
fi

if [ -d "$BUILD_DIR/ios" ]; then
    create_package "ios"
fi

echo ""
echo "=== Distribution Summary ==="
echo "Distribution packages created in: $DIST_DIR"
ls -la "$DIST_DIR"
echo ""
echo "Distribution creation completed!"
```

---

## Summary

In this module, you've learned how to create a comprehensive dual-target setup that supports both standalone applications and plugins across multiple platforms:

### Key Concepts Mastered

1. **Multi-Target Architecture**: Designed a flexible architecture that maximizes code reuse while allowing target-specific customizations
2. **Build System Optimization**: Created an advanced CMake configuration supporting multiple platforms and plugin formats
3. **AUv3 Implementation**: Implemented touch-optimized AUv3 plugins for iOS and macOS with mobile-specific features
4. **Code Sharing Strategies**: Developed shared component architecture with platform-specific wrappers
5. **Cross-Platform Deployment**: Created automated deployment pipelines with code signing and distribution
6. **Platform Optimizations**: Implemented mobile-specific optimizations for performance and battery life
7. **C++20 Features**: Introduced modern C++ concepts, modules, and coroutines for better code organization

### Dual-Target Features Implemented

- **Unified Codebase**: Single source code supporting multiple targets
- **Platform Detection**: Automatic platform and target detection with conditional compilation
- **Touch Interface**: iOS-optimized touch controls with haptic feedback
- **Performance Optimization**: Platform-specific optimizations for desktop and mobile
- **Automated Deployment**: Complete CI/CD pipeline with code signing and distribution
- **Professional Distribution**: App Store ready packages with proper metadata

### Build System Enhancements

- **Multi-Platform Support**: macOS, Windows, Linux, and iOS builds from single configuration
- **Plugin Format Support**: VST3, AU, AUv3, and Standalone targets
- **Code Signing Integration**: Automated code signing and notarization for Apple platforms
- **Optimization Flags**: Platform-specific compiler optimizations
- **Dependency Management**: Efficient JUCE module linking and configuration

### Deployment Capabilities

- **Automated Builds**: Platform-specific build scripts with error handling
- **Code Signing**: Integrated code signing for macOS and iOS
- **Distribution Packages**: Professional installer creation for all platforms
- **App Store Deployment**: Complete iOS App Store submission pipeline
- **Plugin Validation**: Automated plugin testing and validation

### Next Steps

In Module 6, we'll build upon this dual-target foundation by adding:

- Advanced audio effects and processing algorithms
- Real-time spectrum analysis and visualization
- MIDI input processing and virtual instrument capabilities
- Advanced preset management with cloud synchronization
- Professional GUI components with custom graphics
- Performance profiling and optimization tools

---

## Additional Resources

- [JUCE Multi-Target Development Guide](https://docs.juce.com/master/tutorial_plugin_examples.html)
- [Apple AUv3 Development Documentation](https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins)
- [CMake Best Practices for Audio Projects](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [iOS App Store Review Guidelines](https://developer.apple.com/app-store/review/guidelines/)
- [Cross-Platform C++ Development](https://isocpp.org/wiki/faq/compiler-dependencies)

---

## Troubleshooting Common Issues

### Build System Problems

**Problem**: CMake configuration fails on different platforms
**Solution**: Check platform detection logic, verify JUCE installation, ensure proper CMake version

**Problem**: Code signing fails during build
**Solution**: Verify developer certificates, check provisioning profiles, ensure proper entitlements

### AUv3 Development Issues

**Problem**: AUv3 doesn't appear in iOS host apps
**Solution**: Check bundle identifier, verify Info.plist configuration, ensure proper code signing

**Problem**: Touch interface not responsive on iPad
**Solution**: Verify touch event handling, check component bounds, test touch target sizes

### Deployment Problems

**Problem**: Plugin validation fails
**Solution**: Run pluginval, check plugin metadata, verify audio processing implementation

**Problem**: App Store rejection
**Solution**: Review App Store guidelines, check privacy compliance, verify accessibility support

### Performance Issues

**Problem**: High CPU usage on mobile devices
**Solution**: Profile audio processing, implement mobile optimizations, reduce update frequency

**Problem**: Memory warnings on iOS
**Solution**: Implement memory pressure handling, optimize resource usage, clear unused caches

---

*This tutorial is part of a comprehensive JUCE learning series. Module 5 builds upon the plugin architecture from Module 4 and introduces professional deployment strategies that will be expanded with advanced features in subsequent modules.*

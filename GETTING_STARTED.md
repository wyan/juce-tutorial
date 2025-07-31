# Getting Started with JUCE Audio Development

Welcome to the JUCE Audio Development Tutorial Series! This guide will walk you through setting up your development environment, building your first JUCE project, and understanding the project structure.

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Development Environment Setup](#development-environment-setup)
3. [Getting the Tutorial Project](#getting-the-tutorial-project)
4. [Building the Project](#building-the-project)
5. [IDE Configuration](#ide-configuration)
6. [First Run and Testing](#first-run-and-testing)
7. [Project Structure Overview](#project-structure-overview)
8. [Next Steps](#next-steps)
9. [Troubleshooting](#troubleshooting)

---

## 🔧 Prerequisites

### System Requirements

#### Minimum Requirements
- **CPU**: 64-bit processor (Intel/AMD x64 or Apple Silicon)
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free space for development tools and project files
- **OS**:
  - Windows 10/11 (64-bit)
  - macOS 10.15+ (Catalina or later)
  - Linux (Ubuntu 18.04+, Debian 10+, or equivalent)

#### Recommended Specifications
- **CPU**: Multi-core processor (4+ cores)
- **RAM**: 16GB or more
- **Storage**: SSD with 10GB+ free space
- **Audio Interface**: Dedicated audio interface for professional development

### Required Knowledge
- **C++ Fundamentals**: Variables, functions, classes, inheritance
- **Basic Programming Concepts**: Loops, conditionals, data structures
- **Command Line Basics**: Navigating directories, running commands

### Helpful Background (Not Required)
- Digital Signal Processing concepts
- Audio programming experience
- Modern C++ features (C++11 and later)
- Cross-platform development

---

## 🛠️ Development Environment Setup

### Windows Setup

#### 1. Install Visual Studio
```bash
# Download Visual Studio Community (free) from:
# https://visualstudio.microsoft.com/downloads/

# Required workloads:
# - Desktop development with C++
# - Game development with C++ (optional, for additional tools)
```

**Required Components:**
- MSVC v143 compiler toolset
- Windows 10/11 SDK (latest version)
- CMake tools for Visual Studio
- Git for Windows

#### 2. Install Additional Tools
```bash
# Install Git (if not included with Visual Studio)
# Download from: https://git-scm.com/download/win

# Install CMake (standalone, optional)
# Download from: https://cmake.org/download/

# Verify installations
git --version
cmake --version
```

#### 3. Audio Drivers (Optional but Recommended)
```bash
# Install ASIO drivers for low-latency audio
# Download ASIO4ALL: https://www.asio4all.org/
# Or use manufacturer-specific drivers for your audio interface
```

### macOS Setup

#### 1. Install Xcode
```bash
# Install Xcode from Mac App Store
# Or download from Apple Developer portal

# Install command line tools
xcode-select --install

# Verify installation
xcode-select -p
clang --version
```

#### 2. Install Package Manager (Recommended)
```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install development tools
brew install cmake git
brew install --cask visual-studio-code  # Optional IDE
```

#### 3. Audio Configuration
```bash
# macOS includes Core Audio - no additional drivers needed
# For professional work, consider:
# - Audio interface with macOS drivers
# - Audio MIDI Setup configuration
```

### Linux Setup (Ubuntu/Debian)

#### 1. Install Build Tools
```bash
# Update package list
sudo apt update

# Install essential build tools
sudo apt install build-essential cmake git

# Install JUCE dependencies
sudo apt install libasound2-dev libjack-jackd2-dev \
    ladspa-sdk libcurl4-openssl-dev libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
    libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev

# Optional: Install additional audio tools
sudo apt install jackd2 qjackctl
```

#### 2. Configure Audio System
```bash
# Add user to audio group
sudo usermod -a -G audio $USER

# Configure JACK (optional, for professional audio)
# Start qjackctl for GUI configuration
# Or configure ALSA for simpler setup
```

#### 3. IDE Options
```bash
# Install Visual Studio Code
sudo snap install --classic code

# Or install Qt Creator
sudo apt install qtcreator

# Or use any preferred C++ IDE
```

---

## 📥 Getting the Tutorial Project

### Clone the Repository
```bash
# Clone the main repository
git clone https://github.com/your-username/juce-audio-tutorial.git
cd juce-audio-tutorial

# Initialize and update JUCE submodule
git submodule update --init --recursive

# Verify JUCE submodule
ls -la JUCE/  # Should contain JUCE framework files
```

### Alternative: Download ZIP
```bash
# If you prefer not to use Git:
# 1. Download ZIP from GitHub repository
# 2. Extract to desired location
# 3. Download JUCE separately and place in JUCE/ directory
```

### Verify Project Structure
```bash
# Your directory should look like this:
juce-audio-tutorial/
├── CMakeLists.txt
├── README.md
├── GETTING_STARTED.md
├── QUICK_REFERENCE.md
├── JUCE/                    # JUCE framework submodule
├── Source/                  # Tutorial source code
├── tutorials/               # Tutorial documentation
├── examples/                # Standalone examples
└── build/                   # Build directory (created later)
```

---

## 🔨 Building the Project

### Using CMake (Recommended)

#### 1. Create Build Directory
```bash
# From project root directory
mkdir build
cd build
```

#### 2. Configure Project
```bash
# Windows (Visual Studio)
cmake .. -G "Visual Studio 17 2022" -A x64

# macOS (Xcode)
cmake .. -G Xcode

# Linux (Unix Makefiles)
cmake .. -G "Unix Makefiles"

# Alternative: Use default generator
cmake ..
```

#### 3. Build Project
```bash
# Build all targets
cmake --build . --config Release

# Or build specific target
cmake --build . --target JUCETutorial --config Release

# For faster builds on multi-core systems
cmake --build . --config Release --parallel 4
```

### Platform-Specific Build Instructions

#### Windows (Visual Studio)
```bash
# After CMake configuration, you can:
# 1. Open JUCETutorial.sln in Visual Studio
# 2. Set JUCETutorial as startup project
# 3. Build using Ctrl+Shift+B
# 4. Run using F5 or Ctrl+F5

# Or continue with command line:
cmake --build . --config Release
```

#### macOS (Xcode)
```bash
# After CMake configuration:
# 1. Open JUCETutorial.xcodeproj in Xcode
# 2. Select JUCETutorial scheme
# 3. Build using Cmd+B
# 4. Run using Cmd+R

# Or continue with command line:
cmake --build . --config Release
```

#### Linux (Make)
```bash
# After CMake configuration:
make -j$(nproc)  # Build using all CPU cores

# Or specify number of cores:
make -j4
```

### Build Verification
```bash
# Check if executable was created
# Windows:
ls Release/JUCETutorial.exe

# macOS:
ls Release/JUCETutorial.app

# Linux:
ls JUCETutorial
```

---

## 🎯 IDE Configuration

### Visual Studio Code (Cross-Platform)

#### 1. Install Extensions
```json
// Recommended extensions:
{
    "recommendations": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "ms-vscode.cpptools-extension-pack",
        "twxs.cmake",
        "ms-vscode.hexeditor"
    ]
}
```

#### 2. Configure CMake Tools
```json
// .vscode/settings.json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",  // Adjust for your platform
    "cmake.configureOnOpen": true,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

#### 3. Configure C++ IntelliSense
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Linux",  // or "Win32", "Mac"
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/JUCE/modules"
            ],
            "defines": [
                "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1"
            ],
            "compilerPath": "/usr/bin/gcc",  // Adjust for your system
            "cStandard": "c17",
            "cppStandard": "c++20",
            "intelliSenseMode": "linux-gcc-x64"  // Adjust for your platform
        }
    ],
    "version": 4
}
```

### Visual Studio (Windows)

#### 1. Open Project
```bash
# After CMake configuration:
start JUCETutorial.sln  # Opens in Visual Studio
```

#### 2. Configure Project Properties
- Right-click JUCETutorial project → Properties
- Configuration: All Configurations
- Platform: x64
- C/C++ → General → Additional Include Directories: Add JUCE modules path
- Debugging → Working Directory: Set to $(OutDir)

### Xcode (macOS)

#### 1. Open Project
```bash
# After CMake configuration:
open JUCETutorial.xcodeproj
```

#### 2. Configure Scheme
- Product → Scheme → Edit Scheme
- Run → Options → Working Directory: Use custom working directory
- Set to project root directory

---

## 🚀 First Run and Testing

### Running the Application

#### 1. Launch the Application
```bash
# Windows
cd build/Release
./JUCETutorial.exe

# macOS
cd build/Release
open JUCETutorial.app

# Linux
cd build
./JUCETutorial
```

#### 2. Expected Behavior
When you first run the application, you should see:
- A window titled "JUCE Audio Tutorial"
- Basic GUI components (buttons, sliders, labels)
- Audio device selection dropdown
- Volume controls and level meters

#### 3. Test

Basic Functionality
- **Audio Device Selection**: Try changing audio devices in the dropdown
- **Volume Control**: Adjust volume sliders and observe level meters
- **Button Interactions**: Click buttons to test GUI responsiveness
- **Window Resizing**: Resize the window to test layout management

#### 4. Audio Testing
```bash
# Test audio input/output (if you have a microphone)
# 1. Select your audio device from the dropdown
# 2. Speak into microphone
# 3. Observe input level meters
# 4. Adjust volume and test output
```

### Verifying the Build

#### Check Build Artifacts
```bash
# Verify all components were built successfully
ls build/

# Expected files:
# - JUCETutorial executable/app
# - CMakeCache.txt
# - Makefile (Linux) or .sln/.vcxproj (Windows) or .xcodeproj (macOS)
```

#### Run Unit Tests (If Available)
```bash
# If unit tests are built:
cd build
ctest --verbose

# Or run test executable directly:
./Tests/ModernAudioTests  # Linux
./Tests/Release/ModernAudioTests.exe  # Windows
```

---

## 📁 Project Structure Overview

### Root Directory
```
juce-audio-tutorial/
├── CMakeLists.txt           # Main CMake configuration
├── README.md                # Project overview and quick start
├── GETTING_STARTED.md       # This file - detailed setup guide
├── QUICK_REFERENCE.md       # Quick lookup for JUCE/C++ patterns
├── JUCE/                    # JUCE framework (submodule)
└── build/                   # Build artifacts (generated)
```

### Source Code Organization
```
Source/
├── Main.cpp                 # Application entry point
├── MainComponent.h/.cpp     # Main GUI component
├── AdvancedDSP/            # Advanced audio processing
│   ├── BiquadFilter.h      # Biquad filter implementation
│   ├── MultiBandEQ.h       # Multi-band equalizer
│   └── ...
├── ModernCPP/              # Modern C++ examples
│   ├── AudioConcepts.h     # C++20 concepts for audio
│   ├── FunctionalAudio.h   # Functional programming patterns
│   └── ...
├── Visualization/          # Audio visualization components
│   ├── SpectrumAnalyzer.h  # FFT spectrum analyzer
│   └── ...
├── Tests/                  # Unit tests
│   └── ModernAudioTests.cpp
└── Exercises/              # Practice exercises
    └── Exercise1_ConceptRefactoring.h
```

### Tutorial Documentation
```
tutorials/
├── README.md               # Tutorial index and navigation
├── Module_01_JUCE_Basics.md
├── Module_02_Basic_GUI_Components.md
├── Module_06_Advanced_Audio_Features.md
├── Module_07_Modern_CPP_Integration.md
└── ...                     # Additional modules
```

### Key Files Explained

#### [`CMakeLists.txt`](CMakeLists.txt)
- Main build configuration
- JUCE integration setup
- Compiler flags and options
- Target definitions

#### [`Source/Main.cpp`](Source/Main.cpp)
- Application entry point
- JUCE application class
- Window management

#### [`Source/MainComponent.h`](Source/MainComponent.h)
- Main GUI component
- Audio device management
- Component layout and interaction

---

## 🎯 Next Steps

### 1. Explore the Tutorials
Start with the tutorial series in order:

```bash
# Begin with Module 1
open tutorials/Module_01_JUCE_Basics.md

# Or view in your preferred markdown reader
```

**Recommended Learning Path:**
1. **[Module 1: JUCE Basics](tutorials/Module_01_JUCE_Basics.md)** - Framework fundamentals
2. **[Module 2: Basic GUI Components](tutorials/Module_02_Basic_GUI_Components.md)** - Interactive interfaces
3. **[Module 6: Advanced Audio Features](tutorials/Module_06_Advanced_Audio_Features.md)** - Professional DSP
4. **[Module 7: Modern C++ Integration](tutorials/Module_07_Modern_CPP_Integration.md)** - C++20 features

### 2. Set Up Your Development Workflow

#### Configure Your IDE
- Set up debugging configurations
- Configure code formatting (clang-format recommended)
- Install relevant extensions/plugins
- Set up version control integration

#### Establish Build Practices
```bash
# Create convenient build scripts
# Windows (build.bat):
@echo off
cd build
cmake --build . --config Release --parallel 4
pause

# macOS/Linux (build.sh):
#!/bin/bash
cd build
cmake --build . --config Release --parallel $(nproc)
```

### 3. Customize the Project

#### Modify for Your Needs
- Change application name and branding
- Add your own audio processing algorithms
- Implement custom GUI components
- Integrate with external libraries

#### Experiment with Examples
```bash
# Explore the examples directory
ls examples/

# Try building and running individual examples
cd examples/basic-synthesizer
mkdir build && cd build
cmake ..
cmake --build .
```

### 4. Join the Community

#### Resources for Continued Learning
- **JUCE Forum**: https://forum.juce.com/
- **Audio Developer Conference**: https://audio.dev/
- **JUCE Tutorials**: https://docs.juce.com/master/tutorial_getting_started.html
- **Modern C++ Resources**: https://isocpp.org/

#### Contributing
- Report issues on GitHub
- Submit pull requests for improvements
- Share your own audio projects
- Help other learners in forums

---

## 🔧 Troubleshooting

### Common Build Issues

#### CMake Configuration Fails
```bash
# Problem: CMake can't find JUCE
# Solution: Ensure JUCE submodule is initialized
git submodule update --init --recursive

# Problem: Wrong CMake version
# Solution: Update CMake to 3.15 or later
cmake --version  # Check current version
```

#### Compiler Errors
```bash
# Problem: C++20 features not recognized
# Solution: Ensure compiler supports C++20
# GCC 10+, Clang 10+, MSVC 2019 16.11+

# Problem: Missing system libraries (Linux)
# Solution: Install development packages
sudo apt install libasound2-dev libfreetype6-dev libx11-dev
```

#### Linker Errors
```bash
# Problem: Undefined references to JUCE symbols
# Solution: Check CMakeLists.txt JUCE module configuration

# Problem: Missing audio system libraries
# Windows: Ensure Windows SDK is installed
# macOS: Ensure Xcode command line tools are installed
# Linux: Install ALSA/JACK development packages
```

### Runtime Issues

#### Application Won't Start
```bash
# Check dependencies
# Windows: Use Dependency Walker or similar
# macOS: Use otool -L JUCETutorial.app/Contents/MacOS/JUCETutorial
# Linux: Use ldd JUCETutorial

# Verify working directory
# Ensure application can find resources/assets
```

#### No Audio Devices Found
```bash
# Windows: Check audio drivers and Windows Audio service
# macOS: Check Audio MIDI Setup, reset Core Audio if needed
# Linux: Check ALSA/PulseAudio configuration

# Test with system audio applications first
```

#### Performance Issues
```bash
# Check audio buffer sizes
# Increase buffer size for stability: 512-1024 samples
# Decrease for lower latency: 64-256 samples

# Monitor CPU usage
# Use built-in profiling tools
# Check for real-time violations in audio callback
```

### Getting Help

#### Before Asking for Help
1. **Check the documentation**: README, tutorials, and QUICK_REFERENCE
2. **Search existing issues**: GitHub issues, JUCE forum
3. **Verify your setup**: Follow this guide step-by-step
4. **Create minimal reproduction**: Isolate the problem

#### Where to Get Help
- **GitHub Issues**: For bugs and feature requests
- **JUCE Forum**: For general JUCE questions
- **Stack Overflow**: For specific programming questions
- **Discord/Slack**: Real-time community help

#### Providing Useful Information
When asking for help, include:
- Operating system and version
- Compiler and version
- JUCE version
- Complete error messages
- Minimal code example
- Steps to reproduce

---

## 📚 Additional Resources

### Documentation
- **[Main README](README.md)** - Project overview and quick navigation
- **[Tutorial Index](tutorials/README.md)** - Complete tutorial series
- **[Quick Reference](QUICK_REFERENCE.md)** - Fast lookup for common patterns

### External Resources
- **[JUCE Documentation](https://docs.juce.com/)** - Official JUCE docs
- **[Audio Programming Book](http://www.aspbooks.com/)** - Comprehensive audio programming guide
- **[DSP Guide](http://www.dspguide.com/)** - Digital signal processing fundamentals
- **[C++ Reference](https://en.cppreference.com/)** - Modern C++ documentation

### Tools and Utilities
- **[Compiler Explorer](https://godbolt.org/)** - Online C++ compiler and assembly viewer
- **[Audio Plugin Host](https://github.com/kushview/Element)** - Test your plugins
- **[Reaper](https://www.reaper.fm/)** - Professional DAW for plugin testing
- **[Audacity](https://www.audacityteam.org/)** - Free audio editor

---

## 🎉 Congratulations!

You've successfully set up your JUCE development environment and built your first JUCE application! You're now ready to dive into the world of audio programming.

**What you've accomplished:**
- ✅ Set up a complete C++ development environment
- ✅ Built a JUCE audio application from source
- ✅ Verified audio functionality
- ✅ Understood the project structure
- ✅ Prepared for advanced audio development

**Next steps:**
1. Start with [Module 1: JUCE Basics](tutorials/Module_01_JUCE_Basics.md)
2. Experiment with the code examples
3. Build your first custom audio effect
4. Join the JUCE community

Happy coding, and welcome to the exciting world of audio software development! 🎵

---

*This guide is part of the comprehensive JUCE Audio Development Tutorial Series. For more information, see the main [README](README.md) and [Tutorial Index](tutorials/README.md).*

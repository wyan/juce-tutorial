#!/bin/bash

# JUCE Audio Generator Plugin Build Script
# This script builds the plugin in both Debug and Release configurations

echo "=== JUCE Audio Generator Plugin Build Script ==="
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

echo ""
echo "Building plugin..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "=== Build completed successfully! ==="
echo ""
echo "Plugin files created:"
echo "- VST3: build/JUCEAudioGenerator_artefacts/Release/VST3/JUCEAudioGenerator.vst3"
echo "- AU: build/JUCEAudioGenerator_artefacts/Release/AU/JUCEAudioGenerator.component"
echo "- Standalone: build/JUCEAudioGenerator_artefacts/Release/Standalone/JUCEAudioGenerator.app"
echo ""
echo "To test the plugin:"
echo "1. Copy the VST3/AU to your plugin directory"
echo "2. Launch your DAW and scan for new plugins"
echo "3. Load 'JUCE Audio Generator' in a track"
echo ""
echo "Plugin directories:"
echo "- macOS VST3: ~/Library/Audio/Plug-Ins/VST3/"
echo "- macOS AU: ~/Library/Audio/Plug-Ins/Components/"
echo "- Windows VST3: C:\\Program Files\\Common Files\\VST3\\"

# JUCE Audio Generator Plugin

This project demonstrates how to transform a standalone JUCE audio application into a professional audio plugin that can be loaded in Digital Audio Workstations (DAWs).

## Project Structure

```
├── Source/
│   ├── AudioEngine.h/cpp          # Core audio processing engine
│   ├── PluginProcessor.h/cpp      # Plugin AudioProcessor implementation
│   ├── PluginEditor.h/cpp         # Plugin GUI editor
│   ├── MainComponent.h/cpp        # Standalone app GUI (legacy)
│   └── Main.cpp                   # Standalone app entry point
├── tutorials/
│   └── Module_04_Plugin_Architecture.md  # Comprehensive tutorial
├── CMakeLists.txt                 # Build configuration
├── build_plugin.sh               # Build script
└── README_Plugin.md              # This file
```

## Features

### Plugin Capabilities
- **Multi-format Support**: VST3, AU, and Standalone builds
- **Real-time Audio Generation**: Sine wave generator with configurable parameters
- **Host Integration**: Full parameter automation support
- **Professional GUI**: Embedded editor with modern styling
- **State Management**: Save/load plugin state and presets

### Parameters
- **Volume**: 0.0 to 1.0 (linear scale)
- **Frequency**: 20 Hz to 20 kHz (logarithmic scale)
- **Wave Type**: Sine, Square, Sawtooth, Triangle (future implementation)

## Building the Plugin

### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (included as submodule)
- Platform-specific requirements:
  - **macOS**: Xcode command line tools
  - **Windows**: Visual Studio 2019 or higher
  - **Linux**: GCC or Clang

### Build Instructions

#### Using the Build Script (macOS/Linux)
```bash
chmod +x build_plugin.sh
./build_plugin.sh
```

#### Manual Build
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Build Outputs
After successful build, you'll find:
- **VST3**: `build/JUCEAudioGenerator_artefacts/Release/VST3/JUCEAudioGenerator.vst3`
- **AU**: `build/JUCEAudioGenerator_artefacts/Release/AU/JUCEAudioGenerator.component`
- **Standalone**: `build/JUCEAudioGenerator_artefacts/Release/Standalone/JUCEAudioGenerator`

## Installing the Plugin

### macOS
```bash
# VST3
cp -r build/JUCEAudioGenerator_artefacts/Release/VST3/JUCEAudioGenerator.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Audio Unit
cp -r build/JUCEAudioGenerator_artefacts/Release/AU/JUCEAudioGenerator.component ~/Library/Audio/Plug-Ins/Components/
```

### Windows
```cmd
# VST3
copy "build\JUCEAudioGenerator_artefacts\Release\VST3\JUCEAudioGenerator.vst3" "C:\Program Files\Common Files\VST3\"
```

### Linux
```bash
# VST3
cp -r build/JUCEAudioGenerator_artefacts/Release/VST3/JUCEAudioGenerator.vst3 ~/.vst3/
```

## Testing the Plugin

### In a DAW
1. Install the plugin to your system's plugin directory
2. Launch your DAW (Logic Pro, Ableton Live, Reaper, etc.)
3. Scan for new plugins or restart the DAW
4. Create a new audio track
5. Load "JUCE Audio Generator" as an instrument or effect
6. Adjust the Volume and Frequency parameters
7. Test parameter automation by recording parameter changes

### Standalone Application
Run the standalone version directly:
```bash
# macOS
open build/JUCEAudioGenerator_artefacts/Release/Standalone/JUCEAudioGenerator.app

# Windows
build\JUCEAudioGenerator_artefacts\Release\Standalone\JUCEAudioGenerator.exe

# Linux
./build/JUCEAudioGenerator_artefacts/Release/Standalone/JUCEAudioGenerator
```

## Plugin Architecture Overview

### AudioProcessor (PluginProcessor.cpp)
- Implements JUCE's `AudioProcessor` interface
- Manages plugin parameters using `AudioProcessorValueTreeState`
- Integrates with existing `AudioEngine` for audio processing
- Handles plugin lifecycle (prepare, process, release)
- Manages state save/load functionality

### AudioProcessorEditor (PluginEditor.cpp)
- Implements JUCE's `AudioProcessorEditor` interface
- Provides GUI controls for plugin parameters
- Uses parameter attachments for automatic synchronization
- Handles host integration and resizing

### Parameter Management
- **AudioProcessorValueTreeState**: Manages all plugin parameters
- **Parameter Attachments**: Automatic GUI-parameter synchronization
- **Parameter Smoothing**: Prevents audio artifacts during parameter changes
- **Host Automation**: Full support for DAW automation systems

## Development Notes

### Key Differences from Standalone App
1. **Audio I/O**: Host provides audio buffers instead of managing audio devices
2. **Parameters**: Host-automatable parameters instead of internal state
3. **GUI**: Embedded editor instead of independent window
4. **Lifecycle**: Host controls plugin instantiation and destruction
5. **Threading**: Uses host's audio thread instead of managing own thread

### Plugin Validation
Test your plugin with:
- **macOS**: `auval -v aufx JAGn Flab` (for AU plugins)
- **Windows**: Use plugin validation tools in your DAW
- **Cross-platform**: Test in multiple DAWs for compatibility

### Common Issues and Solutions

#### Plugin Not Found in DAW
- Verify plugin is installed in correct directory
- Check plugin format compatibility (VST3 vs AU)
- Restart DAW or rescan plugins
- Check plugin validation logs

#### Audio Dropouts
- Optimize audio processing in `processBlock()`
- Use parameter smoothing for parameter changes
- Avoid memory allocation in audio thread
- Profile CPU usage

#### GUI Issues
- Test editor resizing in different hosts
- Verify parameter attachments are working
- Check component visibility and bounds
- Test with different host scaling factors

## Learning Resources

- **Tutorial**: See `tutorials/Module_04_Plugin_Architecture.md` for comprehensive guide
- **JUCE Documentation**: https://docs.juce.com/
- **Plugin Standards**: VST3 SDK, Audio Unit Programming Guide
- **Community**: JUCE Forum, Audio Developer Community

## License

This project is for educational purposes. Please refer to JUCE licensing terms for commercial use.

## Contributing

This is an educational project. Feel free to extend it with:
- Additional waveform types
- More sophisticated parameter controls
- Advanced audio effects
- MIDI input support
- Preset management system

# JUCE Tutorial Examples

This directory contains standalone code examples that demonstrate specific JUCE concepts and techniques. Each example is self-contained and can be built independently to focus on particular aspects of audio development.

## 📁 Directory Structure

```
examples/
├── README.md                    # This file - examples overview
├── 01-hello-juce/              # Basic JUCE application
├── 02-audio-player/            # Simple audio file player
├── 03-basic-synthesizer/       # Sine wave synthesizer
├── 04-gui-components/          # Interactive GUI elements
├── 05-audio-effects/           # Basic audio effects
├── 06-midi-processor/          # MIDI input/output handling
├── 07-plugin-template/         # Audio plugin template
├── 08-spectrum-analyzer/       # Real-time spectrum analysis
├── 09-modern-cpp-audio/        # C++20 features in audio
└── 10-performance-optimization/ # Optimization techniques
```

## 🎯 Learning Path

### Beginner Examples (Start Here)
1. **[Hello JUCE](01-hello-juce/)** - Basic application structure
2. **[Audio Player](02-audio-player/)** - File I/O and playback
3. **[GUI Components](04-gui-components/)** - Interactive interfaces

### Intermediate Examples
4. **[Basic Synthesizer](03-basic-synthesizer/)** - Audio generation
5. **[Audio Effects](05-audio-effects/)** - Signal processing
6. **[MIDI Processor](06-midi-processor/)** - MIDI handling

### Advanced Examples
7. **[Plugin Template](07-plugin-template/)** - Professional plugin development
8. **[Spectrum Analyzer](08-spectrum-analyzer/)** - Real-time analysis
9. **[Modern C++ Audio](09-modern-cpp-audio/)** - C++20 features
10. **[Performance Optimization](10-performance-optimization/)** - Optimization techniques

## 🔨 Building Examples

### Build All Examples
```bash
# From project root
mkdir examples-build
cd examples-build
cmake ../examples
cmake --build . --config Release
```

### Build Individual Example
```bash
# Navigate to specific example
cd examples/03-basic-synthesizer
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Platform-Specific Instructions

#### Windows
```bash
# Use Visual Studio generator
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### macOS
```bash
# Use Xcode generator
cmake .. -G Xcode
cmake --build . --config Release
```

#### Linux
```bash
# Use Unix Makefiles (default)
cmake ..
make -j$(nproc)
```

## 📚 Example Descriptions

### [01-hello-juce](01-hello-juce/)
**Concepts**: Basic JUCE application, window management, component hierarchy
- Minimal JUCE application structure
- Window creation and management
- Basic component painting
- Event handling fundamentals

### [02-audio-player](02-audio-player/)
**Concepts**: Audio file I/O, playback, transport controls
- Audio format management
- File loading and streaming
- Transport controls (play, pause, stop)
- Position tracking and seeking

### [03-basic-synthesizer](03-basic-synthesizer/)
**Concepts**: Audio generation, oscillators, MIDI input
- Sine wave oscillator
- MIDI note handling
- Polyphonic voice management
- Real-time audio synthesis

### [04-gui-components](04-gui-components/)
**Concepts**: Interactive GUI, layout management, event handling
- Buttons, sliders, and labels
- Custom component painting
- Layout managers (FlexBox, Grid)
- Mouse and keyboard interaction

### [05-audio-effects](05-audio-effects/)
**Concepts**: Digital signal processing, filters, effects
- Low-pass and high-pass filters
- Delay and reverb effects
- Parameter automation
- Real-time processing

### [06-midi-processor](06-midi-processor/)
**Concepts**: MIDI I/O, message processing, virtual instruments
- MIDI input/output handling
- Message parsing and generation
- Virtual MIDI devices
- MIDI-to-audio conversion

### [07-plugin-template](07-plugin-template/)
**Concepts**: Audio plugin development, VST3/AU formats
- Plugin architecture
- Parameter management
- State saving/loading
- Cross-platform plugin building

### [08-spectrum-analyzer](08-spectrum-analyzer/)
**Concepts**: FFT analysis, real-time visualization, graphics
- Fast Fourier Transform (FFT)
- Real-time spectrum analysis
- Custom graphics rendering
- Performance optimization

### [09-modern-cpp-audio](09-modern-cpp-audio/)
**Concepts**: C++20 features, concepts, coroutines
- Audio concepts and constraints
- Template metaprogramming
- Coroutines for async operations
- Modern memory management

### [10-performance-optimization](10-performance-optimization/)
**Concepts**: Real-time safety, SIMD, profiling
- Lock-free programming
- SIMD optimization
- Performance profiling
- Memory management

## 🎓 Usage Guidelines

### For Beginners
1. Start with `01-hello-juce` to understand basic structure
2. Progress through examples in numerical order
3. Experiment with code modifications
4. Read the accompanying documentation

### For Intermediate Developers
1. Focus on examples relevant to your project
2. Study the implementation patterns
3. Adapt code for your specific needs
4. Contribute improvements back to the project

### For Advanced Developers
1. Use examples as reference implementations
2. Study optimization techniques
3. Contribute new examples
4. Help maintain and improve existing examples

## 🔧 Customization

### Modifying Examples
Each example is designed to be:
- **Self-contained**: Minimal dependencies
- **Well-documented**: Clear code comments
- **Modifiable**: Easy to experiment with
- **Educational**: Demonstrates best practices

### Adding New Examples
To contribute a new example:
1. Create a new directory following the naming convention
2. Include a complete CMakeLists.txt
3. Add comprehensive README.md
4. Ensure cross-platform compatibility
5. Follow JUCE coding standards

## 📖 Additional Resources

### Related Documentation
- **[Main README](../README.md)** - Project overview
- **[Getting Started](../GETTING_STARTED.md)** - Setup instructions
- **[Tutorial Index](../tutorials/README.md)** - Comprehensive tutorials
- **[Quick Reference](../QUICK_REFERENCE.md)** - Fast lookup guide

### External Resources
- **[JUCE Examples](https://github.com/juce-framework/JUCE/tree/master/examples)** - Official JUCE examples
- **[JUCE Documentation](https://docs.juce.com/)** - Complete API reference
- **[Audio Developer Conference](https://audio.dev/)** - Industry presentations
- **[JUCE Forum](https://forum.juce.com/)** - Community support

## 🤝 Contributing

We welcome contributions to improve and expand the examples collection:

### How to Contribute
1. **Report Issues**: Found a bug or unclear documentation?
2. **Suggest Improvements**: Ideas for better examples or explanations?
3. **Submit Examples**: Have a great example to share?
4. **Fix Bugs**: Help improve existing examples

### Contribution Guidelines
- Follow existing code style and structure
- Include comprehensive documentation
- Test on multiple platforms
- Ensure examples are educational and clear
- Add appropriate comments and explanations

---

*These examples are part of the comprehensive JUCE Audio Development Tutorial Series. Each example builds upon concepts from the main tutorials while providing focused, practical implementations.*

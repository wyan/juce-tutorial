# Basic Synthesizer - Audio Generation Example

This example demonstrates fundamental audio synthesis concepts in JUCE. It creates a simple polyphonic synthesizer that generates sine waves in response to MIDI input or on-screen keyboard interaction.

## 🎯 Learning Objectives

- Understand real-time audio processing in JUCE
- Learn MIDI input handling and note management
- Implement polyphonic voice management
- Create interactive audio applications
- Master audio callback optimization

## 📁 Files Overview

```
03-basic-synthesizer/
├── README.md           # This documentation
├── CMakeLists.txt      # Build configuration
├── Source/
│   ├── Main.cpp        # Application entry point
│   ├── MainComponent.h # Main GUI and audio component
│   ├── SineWaveVoice.h # Individual synthesizer voice
│   └── SineWaveSound.h # Sound definition for voices
└── build/              # Build directory (created when building)
```

## 🔧 Building and Running

### Quick Start
```bash
cd examples/03-basic-synthesizer
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run the application
# Windows: ./Release/BasicSynthesizer.exe
# macOS: open ./Release/BasicSynthesizer.app
# Linux: ./BasicSynthesizer
```

## 🎵 What You'll Experience

When you run the synthesizer, you'll see:

1. **On-Screen Keyboard**: Click keys to play notes
2. **Volume Control**: Adjust overall output level
3. **MIDI Input**: Connect a MIDI keyboard for real input
4. **Polyphonic Playback**: Play multiple notes simultaneously
5. **Real-Time Audio**: Low-latency audio generation

## 📚 Key Concepts Demonstrated

### 1. Audio Processing Pipeline
```cpp
void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
{
    // Clear the buffer
    bufferToFill.clearActiveBufferRegion();

    // Let the synthesizer fill the buffer
    synthesizer.renderNextBlock(*bufferToFill.buffer,
                               midiMessages,
                               bufferToFill.startSample,
                               bufferToFill.numSamples);

    // Apply master volume
    bufferToFill.buffer->applyGain(masterVolume);
}
```

### 2. MIDI Note Handling
```cpp
void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override
{
    // Convert MIDI note to synthesizer message
    auto message = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    midiMessages.addEvent(message, 0);
}
```

### 3. Polyphonic Voice Management
```cpp
class SineWaveVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double tailOff = 0.0;
};
```

### 4. Real-Time Safe Programming
```cpp
void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0) // Note is releasing
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                ++startSample;

                tailOff *= 0.99; // Exponential decay

                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else // Note is playing
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
}
```

## 🎛️ Interactive Features

### On-Screen Keyboard
- **Mouse Click**: Click piano keys to play notes
- **Multiple Notes**: Hold multiple keys for chords
- **Visual Feedback**: Keys highlight when pressed

### MIDI Input Support
- **External Keyboards**: Connect USB/MIDI keyboards
- **Real-Time Response**: Low-latency note triggering
- **Velocity Sensitivity**: Responds to key velocity

### Audio Controls
- **Master Volume**: Overall output level control
- **Real-Time Adjustment**: Changes apply immediately
- **Safe Range**: Prevents audio clipping

## 🔬 Technical Deep Dive

### Synthesizer Architecture
```
MIDI Input → Note Events → Voice Allocation → Audio Generation → Output Buffer
     ↓              ↓              ↓                ↓              ↓
Keyboard/GUI → MidiBuffer → SynthesiserVoice → Sin Wave → Audio Device
```

### Voice Lifecycle
1. **Note On**: Allocate voice, set frequency and amplitude
2. **Sustain**: Generate continuous sine wave
3. **Note Off**: Begin exponential decay (tail-off)
4. **Release**: Voice becomes available for reuse

### Performance Considerations
- **Voice Pooling**: Reuse voice objects to avoid allocation
- **Lock-Free**: Audio thread never blocks on GUI thread
- **Efficient Math**: Optimized trigonometric calculations
- **Buffer Management**: Minimize memory allocations

## 🚀 Experiments to Try

### Beginner Modifications
1. **Change Waveform**: Replace sine wave with sawtooth or square
2. **Adjust Decay**: Modify the tail-off rate (0.99 factor)
3. **Add Vibrato**: Modulate frequency with LFO
4. **Volume Envelope**: Add attack/decay/sustain/release

### Intermediate Modifications
1. **Multiple Oscillators**: Layer different waveforms
2. **Filter**: Add low-pass or high-pass filtering
3. **Effects**: Add reverb or delay
4. **Preset System**: Save and load different sounds

### Advanced Modifications
1. **Wavetable Synthesis**: Use pre-computed waveforms
2. **FM Synthesis**: Implement frequency modulation
3. **Granular Synthesis**: Sample-based synthesis
4. **Plugin Version**: Convert to VST3/AU plugin

## 🎓 Learning Path Integration

### Prerequisites
- Complete [01-hello-juce](../01-hello-juce/) for basic JUCE concepts
- Understand basic trigonometry (sine waves)
- Familiarity with MIDI concepts

### Next Steps
- [05-audio-effects](../05-audio-effects/) - Add effects processing
- [06-midi-processor](../06-midi-processor/) - Advanced MIDI handling
- [07-plugin-template](../07-plugin-template/) - Convert to plugin

### Related Tutorials
- [Module 6: Advanced Audio Features](../../tutorials/Module_06_Advanced_Audio_Features.md)
- [Module 7: Modern C++ Integration](../../tutorials/Module_07_Modern_CPP_Integration.md)

## 🔧 Troubleshooting

### No Audio Output
1. Check audio device selection in system settings
2. Verify sample rate compatibility (44.1kHz/48kHz)
3. Ensure audio permissions are granted
4. Test with system audio applications first

### High CPU Usage
1. Increase audio buffer size (512-1024 samples)
2. Reduce number of simultaneous voices
3. Optimize mathematical calculations
4. Profile code to find bottlenecks

### MIDI Not Working
1. Connect MIDI device before starting application
2. Check MIDI device drivers and system recognition
3. Verify MIDI permissions (macOS/iOS)
4. Test MIDI device with other applications

## 📖 Additional Resources

### Audio Programming Concepts
- **[Digital Signal Processing](http://www.dspguide.com/)** - DSP fundamentals
- **[MIDI Specification](https://www.midi.org/)** - MIDI protocol details
- **[Audio Programming Book](http://www.aspbooks.com/)** - Comprehensive guide

### JUCE Documentation
- **[AudioSource](https://docs.juce.com/master/classAudioSource.html)** - Audio processing base class
- **[Synthesiser](https://docs.juce.com/master/classSynthesiser.html)** - Polyphonic synthesizer
- **[MidiKeyboardComponent](https://docs.juce.com/master/classMidiKeyboardComponent.html)** - On-screen keyboard

---

*This example demonstrates real-time audio synthesis and MIDI handling in JUCE. It serves as a foundation for more complex audio applications and synthesizer development.*

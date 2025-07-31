#pragma once

#include <JuceHeader.h>
#include <atomic>

class AudioEngine : public juce::AudioIODeviceCallback,
                   public juce::AudioSource
{
public:
    AudioEngine();
    ~AudioEngine() override;

    // AudioIODeviceCallback interface
    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples);

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // AudioSource interface
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    // Control interface (thread-safe)
    void setPlaying(bool shouldPlay);
    void setVolume(float newVolume);
    void setFrequency(float newFrequency);

    bool isPlaying() const { return playing.load(); }
    float getCurrentVolume() const { return volume.load(); }
    float getCurrentFrequency() const { return frequency.load(); }

    // Audio device management
    bool initializeAudio();
    void shutdownAudio();
    juce::String getAudioDeviceStatus() const;

private:
    // Audio processing
    void generateSineWave(float** outputChannelData, int numChannels, int numSamples);
    void updatePhaseIncrement();

    // Audio device management
    juce::AudioDeviceManager deviceManager;

    // Thread-safe parameters
    std::atomic<bool> playing{false};
    std::atomic<float> volume{0.5f};
    std::atomic<float> frequency{440.0f};

    // Audio processing state
    double sampleRate = 44100.0;
    float currentPhase = 0.0f;
    float phaseIncrement = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};

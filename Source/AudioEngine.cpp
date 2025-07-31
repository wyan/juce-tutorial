#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
    updatePhaseIncrement();
}

AudioEngine::~AudioEngine()
{
    shutdownAudio();
}

bool AudioEngine::initializeAudio()
{
    // Initialize with default audio devices
    auto result = deviceManager.initialiseWithDefaultDevices(0, 2);  // 0 inputs, 2 outputs

    if (result.isNotEmpty())
    {
        DBG("Audio initialization failed: " + result);
        return false;
    }

    // Set this as the audio callback
    deviceManager.addAudioCallback(this);

    return true;
}

void AudioEngine::shutdownAudio()
{
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
}

juce::String AudioEngine::getAudioDeviceStatus() const
{
    auto* device = deviceManager.getCurrentAudioDevice();
    if (device == nullptr)
        return "No audio device";

    juce::String status;
    status << "Device: " << device->getName() << "\n";
    status << "Sample Rate: " << device->getCurrentSampleRate() << " Hz\n";
    status << "Buffer Size: " << device->getCurrentBufferSizeSamples() << " samples\n";
    status << "Output Channels: " << device->getActiveOutputChannels().countNumberOfSetBits();

    return status;
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    updatePhaseIncrement();

    DBG("Audio device started: " + device->getName());
    DBG("Sample rate: " + juce::String(sampleRate));
    DBG("Buffer size: " + juce::String(device->getCurrentBufferSizeSamples()));
}

void AudioEngine::audioDeviceStopped()
{
    DBG("Audio device stopped");
}

void AudioEngine::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples)
{
    // Clear output buffers first
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] != nullptr)
        {
            juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
        }
    }

    // Generate audio if playing
    if (playing.load())
    {
        generateSineWave(outputChannelData, numOutputChannels, numSamples);
    }
}

void AudioEngine::generateSineWave(float** outputChannelData, int numChannels, int numSamples)
{
    auto currentVolume = volume.load();
    auto currentFreq = frequency.load();

    // Update phase increment if frequency changed
    auto expectedIncrement = currentFreq * juce::MathConstants<float>::twoPi / static_cast<float>(sampleRate);
    if (std::abs(phaseIncrement - expectedIncrement) > 0.001f)
    {
        phaseIncrement = expectedIncrement;
    }

    // Generate samples
    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto sampleValue = std::sin(currentPhase) * currentVolume;

        // Apply to all output channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            if (outputChannelData[channel] != nullptr)
            {
                outputChannelData[channel][sample] = sampleValue;
            }
        }

        // Update phase
        currentPhase += phaseIncrement;
        if (currentPhase >= juce::MathConstants<float>::twoPi)
            currentPhase -= juce::MathConstants<float>::twoPi;
    }
}

void AudioEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = sampleRate;
    updatePhaseIncrement();
}

void AudioEngine::releaseResources()
{
    // Cleanup resources when audio stops
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (playing.load())
    {
        auto* buffer = bufferToFill.buffer;
        auto startSample = bufferToFill.startSample;
        auto numSamples = bufferToFill.numSamples;

        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
        {
            auto* channelData = buffer->getWritePointer(channel, startSample);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                auto sampleValue = std::sin(currentPhase) * volume.load();
                channelData[sample] = sampleValue;

                currentPhase += phaseIncrement;
                if (currentPhase >= juce::MathConstants<float>::twoPi)
                    currentPhase -= juce::MathConstants<float>::twoPi;
            }
        }
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
}

void AudioEngine::setPlaying(bool shouldPlay)
{
    playing.store(shouldPlay);
}

void AudioEngine::setVolume(float newVolume)
{
    volume.store(juce::jlimit(0.0f, 1.0f, newVolume));
}

void AudioEngine::setFrequency(float newFrequency)
{
    frequency.store(juce::jlimit(20.0f, 20000.0f, newFrequency));
}

void AudioEngine::updatePhaseIncrement()
{
    phaseIncrement = frequency.load() * juce::MathConstants<float>::twoPi / static_cast<float>(sampleRate);
}

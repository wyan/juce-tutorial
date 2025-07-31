#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"

//==============================================================================
/**
    JUCE Audio Generator Plugin Processor

    This class implements the AudioProcessor interface to create a professional
    audio plugin that can be loaded in DAWs. It integrates our existing AudioEngine
    with JUCE's plugin architecture and provides parameter management through
    AudioProcessorValueTreeState.
*/
class JUCEAudioGeneratorProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    JUCEAudioGeneratorProcessor();
    ~JUCEAudioGeneratorProcessor() override;

    //==============================================================================
    // AudioProcessor interface implementation
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    //==============================================================================
    // Plugin information
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Programs/presets
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    // Parameter access for editor
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
    AudioEngine& getAudioEngine() { return audioEngine; }

private:
    //==============================================================================
    // Parameter management
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Audio processing
    AudioEngine audioEngine;

    // Parameter pointers for real-time access
    std::atomic<float>* volumeParameter = nullptr;
    std::atomic<float>* frequencyParameter = nullptr;
    std::atomic<float>* waveTypeParameter = nullptr;

    // Parameter smoothing to avoid zipper noise
    juce::SmoothedValue<float> smoothedVolume;
    juce::SmoothedValue<float> smoothedFrequency;

    // Performance monitoring
    void updateParametersFromValueTree();
    void initializeParameterPointers();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JUCEAudioGeneratorProcessor)
};

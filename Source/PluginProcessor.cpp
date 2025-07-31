#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JUCEAudioGeneratorProcessor::JUCEAudioGeneratorProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize parameter pointers for real-time access
    initializeParameterPointers();

    // Initialize parameter smoothing
    smoothedVolume.reset(44100.0, 0.05);    // 50ms smoothing
    smoothedFrequency.reset(44100.0, 0.1);  // 100ms smoothing
}

JUCEAudioGeneratorProcessor::~JUCEAudioGeneratorProcessor()
{
}

//==============================================================================
const juce::String JUCEAudioGeneratorProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JUCEAudioGeneratorProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JUCEAudioGeneratorProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JUCEAudioGeneratorProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JUCEAudioGeneratorProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JUCEAudioGeneratorProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JUCEAudioGeneratorProcessor::getCurrentProgram()
{
    return 0;
}

void JUCEAudioGeneratorProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String JUCEAudioGeneratorProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return "Default";
}

void JUCEAudioGeneratorProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void JUCEAudioGeneratorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the audio engine
    audioEngine.prepareToPlay(samplesPerBlock, sampleRate);

    // Initialize parameter smoothing with the correct sample rate
    smoothedVolume.reset(sampleRate, 0.05);
    smoothedFrequency.reset(sampleRate, 0.1);

    // Set initial parameter values
    updateParametersFromValueTree();
}

void JUCEAudioGeneratorProcessor::releaseResources()
{
    // Release audio engine resources
    audioEngine.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JUCEAudioGeneratorProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void JUCEAudioGeneratorProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update parameters from the parameter tree
    updateParametersFromValueTree();

    // Always generate audio in plugin mode (no play/stop button)
    audioEngine.setPlaying(true);

    // Update audio engine parameters with smoothed values
    audioEngine.setVolume(smoothedVolume.getNextValue());
    audioEngine.setFrequency(smoothedFrequency.getNextValue());

    // Process audio through the AudioEngine
    juce::AudioSourceChannelInfo channelInfo(buffer);
    audioEngine.getNextAudioBlock(channelInfo);
}

//==============================================================================
bool JUCEAudioGeneratorProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JUCEAudioGeneratorProcessor::createEditor()
{
    return new JUCEAudioGeneratorEditor (*this);
}

//==============================================================================
void JUCEAudioGeneratorProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the parameter state
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void JUCEAudioGeneratorProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the parameter state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout JUCEAudioGeneratorProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Volume parameter (0.0 to 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "volume",                                    // Parameter ID
        "Volume",                                   // Parameter name
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), // Range
        0.5f                                        // Default value
    ));

    // Frequency parameter (20 Hz to 20 kHz, logarithmic scaling)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "frequency",                                // Parameter ID
        "Frequency",                               // Parameter name
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), // Range with skew
        440.0f                                     // Default value
    ));

    // Wave type parameter (choice between different waveforms)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "waveType",                                // Parameter ID
        "Wave Type",                              // Parameter name
        juce::StringArray{"Sine", "Square", "Sawtooth", "Triangle"}, // Choices
        0                                         // Default index (Sine)
    ));

    return { params.begin(), params.end() };
}

void JUCEAudioGeneratorProcessor::initializeParameterPointers()
{
    // Get atomic parameter pointers for real-time access
    volumeParameter = parameters.getRawParameterValue("volume");
    frequencyParameter = parameters.getRawParameterValue("frequency");
    waveTypeParameter = parameters.getRawParameterValue("waveType");
}

void JUCEAudioGeneratorProcessor::updateParametersFromValueTree()
{
    // Update smoothed parameter values
    if (volumeParameter != nullptr)
        smoothedVolume.setTargetValue(volumeParameter->load());

    if (frequencyParameter != nullptr)
        smoothedFrequency.setTargetValue(frequencyParameter->load());

    // Wave type doesn't need smoothing as it's a discrete parameter
    if (waveTypeParameter != nullptr)
    {
        // For now, we only support sine waves in our AudioEngine
        // This could be extended to support different waveforms
        auto waveType = static_cast<int>(waveTypeParameter->load());
        juce::ignoreUnused(waveType); // Suppress unused variable warning
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JUCEAudioGeneratorProcessor();
}

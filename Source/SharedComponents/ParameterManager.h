#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace SharedComponents {

//==============================================================================
/**
 * Shared parameter management system for dual-target setup
 */
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
    ~ParameterManager() = default;

    // Parameter management
    void addParameter(const ParameterInfo& info);
    void setParameter(const juce::String& id, float value);
    float getParameter(const juce::String& id) const;

    const std::vector<ParameterInfo>& getAllParameters() const { return parameters; }

    // Target-specific parameter creation
    template<typename ParameterType>
    std::unique_ptr<ParameterType> createJUCEParameter(const juce::String& id) const;

    // Parameter listeners
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void parameterChanged(const juce::String& parameterID, float newValue) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    std::vector<ParameterInfo> parameters;
    std::unordered_map<juce::String, size_t> parameterMap;
    mutable std::mutex parameterMutex;
    juce::ListenerList<Listener> listeners;

    void notifyListeners(const juce::String& parameterID, float newValue);
};

//==============================================================================
// Template specializations for different parameter types
template<>
std::unique_ptr<juce::AudioParameterFloat>
ParameterManager::createJUCEParameter<juce::AudioParameterFloat>(const juce::String& id) const;

template<>
std::unique_ptr<juce::AudioParameterChoice>
ParameterManager::createJUCEParameter<juce::AudioParameterChoice>(const juce::String& id) const;

} // namespace SharedComponents

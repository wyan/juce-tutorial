#include "ParameterManager.h"

namespace SharedComponents {

//==============================================================================
ParameterManager::ParameterManager()
{
    // Add default parameters that all targets use
    addParameter({
        "volume", "Volume", 0.0f, 1.0f, 0.5f, 0.5f, "",
        [](float value) { return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
        [](const juce::String& text) { return juce::Decibels::decibelsToGain(text.getFloatValue()); }
    });

    addParameter({
        "frequency", "Frequency", 20.0f, 20000.0f, 440.0f, 440.0f, "Hz",
        [](float value) { return juce::String(value, 0) + " Hz"; },
        [](const juce::String& text) { return text.getFloatValue(); }
    });

    addParameter({
        "waveType", "Wave Type", 0.0f, 3.0f, 0.0f, 0.0f, "",
        [](float value) {
            const char* types[] = {"Sine", "Square", "Sawtooth", "Triangle"};
            return juce::String(types[juce::jlimit(0, 3, (int)value)]);
        },
        [](const juce::String& text) {
            if (text == "Sine") return 0.0f;
            if (text == "Square") return 1.0f;
            if (text == "Sawtooth") return 2.0f;
            if (text == "Triangle") return 3.0f;
            return 0.0f;
        }
    });
}

void ParameterManager::addParameter(const ParameterInfo& info)
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    parameterMap[info.id] = parameters.size();
    parameters.push_back(info);
}

void ParameterManager::setParameter(const juce::String& id, float value)
{
    {
        std::lock_guard<std::mutex> lock(parameterMutex);

        auto it = parameterMap.find(id);
        if (it != parameterMap.end())
        {
            auto& param = parameters[it->second];
            float newValue = juce::jlimit(param.minValue, param.maxValue, value);

            if (param.currentValue != newValue)
            {
                param.currentValue = newValue;
                // Notify listeners outside the lock to avoid deadlock
                juce::MessageManager::callAsync([this, id, newValue]() {
                    notifyListeners(id, newValue);
                });
            }
        }
    }
}

float ParameterManager::getParameter(const juce::String& id) const
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        return parameters[it->second].currentValue;
    }
    return 0.0f;
}

void ParameterManager::addListener(Listener* listener)
{
    listeners.add(listener);
}

void ParameterManager::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

void ParameterManager::notifyListeners(const juce::String& parameterID, float newValue)
{
    listeners.call([&](Listener& l) { l.parameterChanged(parameterID, newValue); });
}

//==============================================================================
// Template specializations

template<>
std::unique_ptr<juce::AudioParameterFloat>
ParameterManager::createJUCEParameter<juce::AudioParameterFloat>(const juce::String& id) const
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        const auto& info = parameters[it->second];
        return std::make_unique<juce::AudioParameterFloat>(
            info.id, info.name,
            juce::NormalisableRange<float>(info.minValue, info.maxValue),
            info.defaultValue,
            info.units,
            juce::AudioProcessorParameter::genericParameter,
            info.valueToText,
            info.textToValue
        );
    }
    return nullptr;
}

template<>
std::unique_ptr<juce::AudioParameterChoice>
ParameterManager::createJUCEParameter<juce::AudioParameterChoice>(const juce::String& id) const
{
    std::lock_guard<std::mutex> lock(parameterMutex);

    auto it = parameterMap.find(id);
    if (it != parameterMap.end())
    {
        const auto& info = parameters[it->second];

        // For choice parameters, create options based on the range
        juce::StringArray choices;
        for (int i = (int)info.minValue; i <= (int)info.maxValue; ++i)
        {
            choices.add(info.valueToText(static_cast<float>(i)));
        }

        return std::make_unique<juce::AudioParameterChoice>(
            info.id, info.name,
            choices,
            (int)info.defaultValue
        );
    }
    return nullptr;
}

} // namespace SharedComponents

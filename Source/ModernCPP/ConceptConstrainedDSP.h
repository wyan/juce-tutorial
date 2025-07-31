#pragma once
#include "AudioConcepts.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

namespace ModernAudio {

// Type-safe audio processing with concepts
template<AudioSample SampleType>
class ConceptConstrainedProcessor {
public:
    void processBuffer(std::vector<SampleType>& buffer)
        requires AudioSample<SampleType>
    {
        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
            [this](SampleType sample) { return processSample(sample); });
    }

    template<FilterType Filter>
    void applyFilter(std::vector<SampleType>& buffer, Filter& filter) {
        for (auto& sample : buffer) {
            sample = filter.process(sample);
        }
    }

    void setGain(SampleType newGain) requires std::floating_point<SampleType> {
        gain_ = std::clamp(newGain, SampleType(0), SampleType(10));
    }

    SampleType getGain() const { return gain_; }

    void reset() {
        // Reset internal state
        gain_ = SampleType(1);
    }

    int getLatency() const { return 0; }

    void processBlock(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = static_cast<float>(processSample(static_cast<SampleType>(buffer[i])));
        }
    }

private:
    SampleType processSample(SampleType input) {
        // Apply gain and soft clipping
        constexpr SampleType maxGain = SampleType(2.0);
        SampleType gained = input * gain_;
        return std::tanh(gained); // Soft clipping
    }

    SampleType gain_{1.0f};
};

// Concept-based function overloading for normalization
template<AudioSample T>
void normalizeAudio(std::vector<T>& buffer)
    requires std::floating_point<T>
{
    auto maxVal = *std::max_element(buffer.begin(), buffer.end(),
        [](T a, T b) { return std::abs(a) < std::abs(b); });

    if (std::abs(maxVal) > T(0)) {
        T scale = T(1) / std::abs(maxVal);
        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
            [scale](T sample) { return sample * scale; });
    }
}

template<AudioSample T>
void normalizeAudio(std::vector<T>& buffer)
    requires std::integral<T>
{
    constexpr T maxValue = std::numeric_limits<T>::max();
    auto maxVal = *std::max_element(buffer.begin(), buffer.end(),
        [](T a, T b) { return std::abs(a) < std::abs(b); });

    if (maxVal > 0) {
        double scale = static_cast<double>(maxValue) / maxVal;
        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
            [scale](T sample) { return static_cast<T>(sample * scale); });
    }
}

// Type-safe parameter class
template<std::floating_point T>
class TypeSafeParameter {
public:
    TypeSafeParameter(const std::string& name, T minVal, T maxVal, T defaultVal)
        : name_(name), minValue_(minVal), maxValue_(maxVal), value_(defaultVal) {}

    T getValue() const { return value_; }

    void setValue(T newValue) {
        value_ = std::clamp(newValue, minValue_, maxValue_);
    }

    std::string getName() const { return name_; }

    std::pair<T, T> getRange() const { return {minValue_, maxValue_}; }

    T getNormalizedValue() const {
        return (value_ - minValue_) / (maxValue_ - minValue_);
    }

    void setNormalizedValue(T normalizedValue) {
        T clampedNorm = std::clamp(normalizedValue, T(0), T(1));
        value_ = minValue_ + clampedNorm * (maxValue_ - minValue_);
    }

private:
    std::string name_;
    T minValue_;
    T maxValue_;
    T value_;
};

// Concept-constrained audio effect base class
template<AudioSample SampleType>
class ConceptConstrainedEffect {
public:
    virtual ~ConceptConstrainedEffect() = default;

    virtual SampleType process(SampleType input) = 0;
    virtual void reset() = 0;
    virtual void setSampleRate(double sampleRate) { sampleRate_ = sampleRate; }

    bool isBypassed() const { return bypassed_; }
    void setBypassed(bool bypass) { bypassed_ = bypass; }

    double getTailLengthSeconds() const { return 0.0; }

    // Satisfy AudioProcessor concept
    void processBlock(float* buffer, int numSamples) {
        if (bypassed_) return;

        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = static_cast<float>(process(static_cast<SampleType>(buffer[i])));
        }
    }

    int getLatency() const { return 0; }

protected:
    double sampleRate_ = 44100.0;
    bool bypassed_ = false;
};

// Example concept-constrained filter implementation
template<AudioSample SampleType>
class ConceptConstrainedLowPassFilter : public ConceptConstrainedEffect<SampleType> {
public:
    ConceptConstrainedLowPassFilter() = default;

    SampleType process(SampleType input) override {
        if (this->bypassed_) return input;

        // Simple one-pole lowpass filter
        output_ = output_ + cutoff_ * (input - output_);
        return output_;
    }

    void reset() override {
        output_ = SampleType(0);
    }

    void setFrequency(SampleType frequency) {
        // Convert frequency to cutoff coefficient
        cutoff_ = SampleType(1) - std::exp(-SampleType(2) * SampleType(M_PI) * frequency / SampleType(this->sampleRate_));
    }

    void setQ(SampleType q) {
        // Q factor not used in simple one-pole filter
        (void)q;
    }

private:
    SampleType output_{0};
    SampleType cutoff_{SampleType(0.1)};
};

// Concept validation helper
template<typename T>
constexpr bool validateAudioProcessor() {
    return AudioProcessor<T>;
}

template<typename T>
constexpr bool validateFilterType() {
    return FilterType<T>;
}

template<typename T>
constexpr bool validateAudioSample() {
    return AudioSample<T>;
}

} // namespace ModernAudio

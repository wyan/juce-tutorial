#pragma once
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>
#include <optional>
#include <map>
#include <cmath>

namespace ModernAudio {

// Function composition for audio effects
template<typename F, typename G>
auto compose(F&& f, G&& g) {
    return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&& x) {
        return f(g(std::forward<decltype(x)>(x)));
    };
}

// Audio effect as a function type
using AudioEffect = std::function<float(float)>;

// Higher-order function for creating audio effect chains
class EffectChain {
public:
    EffectChain& addEffect(AudioEffect effect) {
        if (chain_) {
            chain_ = compose(effect, *chain_);
        } else {
            chain_ = effect;
        }
        return *this;
    }

    float process(float input) const {
        return chain_ ? (*chain_)(input) : input;
    }

    void processBuffer(std::vector<float>& buffer) const {
        if (chain_) {
            std::transform(buffer.begin(), buffer.end(), buffer.begin(), *chain_);
        }
    }

    void clear() {
        chain_.reset();
    }

    bool isEmpty() const {
        return !chain_.has_value();
    }

private:
    std::optional<AudioEffect> chain_;
};

// Functional-style audio effects
namespace Effects {

AudioEffect gain(float gainValue) {
    return [gainValue](float input) {
        return input * gainValue;
    };
}

AudioEffect softClip(float threshold = 0.7f) {
    return [threshold](float input) {
        if (std::abs(input) <= threshold) {
            return input;
        }
        return std::copysign(threshold + (1.0f - threshold) *
                           std::tanh((std::abs(input) - threshold) / (1.0f - threshold)),
                           input);
    };
}

AudioEffect highpass(float cutoff, float sampleRate) {
    return [cutoff, sampleRate, prev = 0.0f](float input) mutable {
        const float rc = 1.0f / (2.0f * M_PI * cutoff);
        const float dt = 1.0f / sampleRate;
        const float alpha = rc / (rc + dt);

        float output = alpha * (prev + input - prev);
        prev = input;
        return output;
    };
}

AudioEffect lowpass(float cutoff, float sampleRate) {
    return [cutoff, sampleRate, prev = 0.0f](float input) mutable {
        const float rc = 1.0f / (2.0f * M_PI * cutoff);
        const float dt = 1.0f / sampleRate;
        const float alpha = dt / (rc + dt);

        prev = prev + alpha * (input - prev);
        return prev;
    };
}

AudioEffect delay(int delaySamples) {
    return [delaySamples, buffer = std::vector<float>(delaySamples, 0.0f),
            index = 0](float input) mutable {
        float output = buffer[index];
        buffer[index] = input;
        index = (index + 1) % delaySamples;
        return output;
    };
}

AudioEffect reverb(float roomSize, float damping, float wetLevel) {
    return [roomSize, damping, wetLevel,
            delays = std::vector<std::vector<float>>{
                std::vector<float>(static_cast<size_t>(roomSize * 1000), 0.0f),
                std::vector<float>(static_cast<size_t>(roomSize * 1100), 0.0f),
                std::vector<float>(static_cast<size_t>(roomSize * 1200), 0.0f),
                std::vector<float>(static_cast<size_t>(roomSize * 1300), 0.0f)
            },
            indices = std::vector<size_t>(4, 0)](float input) mutable {

        float reverbSum = 0.0f;

        for (size_t i = 0; i < delays.size(); ++i) {
            float delayed = delays[i][indices[i]];
            delays[i][indices[i]] = input + delayed * damping;
            indices[i] = (indices[i] + 1) % delays[i].size();
            reverbSum += delayed;
        }

        return input * (1.0f - wetLevel) + reverbSum * wetLevel * 0.25f;
    };
}

AudioEffect distortion(float drive, float mix = 1.0f) {
    return [drive, mix](float input) {
        float driven = input * drive;
        float distorted = std::tanh(driven);
        return input * (1.0f - mix) + distorted * mix;
    };
}

} // namespace Effects

// Immutable audio processing patterns
class ImmutableAudioProcessor {
public:
    struct ProcessingState {
        std::vector<float> buffer;
        float gain;
        bool bypassed;

        ProcessingState withGain(float newGain) const {
            return {buffer, newGain, bypassed};
        }

        ProcessingState withBypass(bool bypass) const {
            return {buffer, gain, bypass};
        }

        ProcessingState processedWith(const AudioEffect& effect) const {
            if (bypassed) return *this;

            auto newBuffer = buffer;
            std::transform(newBuffer.begin(), newBuffer.end(),
                          newBuffer.begin(), effect);
            return {newBuffer, gain, bypassed};
        }

        ProcessingState normalized() const {
            if (bypassed || buffer.empty()) return *this;

            auto newBuffer = buffer;
            auto maxVal = *std::max_element(newBuffer.begin(), newBuffer.end(),
                [](float a, float b) { return std::abs(a) < std::abs(b); });

            if (std::abs(maxVal) > 0.0f) {
                float scale = 1.0f / std::abs(maxVal);
                std::transform(newBuffer.begin(), newBuffer.end(), newBuffer.begin(),
                    [scale](float sample) { return sample * scale; });
            }

            return {newBuffer, gain, bypassed};
        }

        float getRMS() const {
            if (buffer.empty()) return 0.0f;

            float sumSquares = std::accumulate(buffer.begin(), buffer.end(), 0.0f,
                [](float acc, float val) { return acc + val * val; });
            return std::sqrt(sumSquares / buffer.size());
        }

        float getPeak() const {
            if (buffer.empty()) return 0.0f;

            auto maxElement = std::max_element(buffer.begin(), buffer.end(),
                [](float a, float b) { return std::abs(a) < std::abs(b); });
            return std::abs(*maxElement);
        }
    };

    static ProcessingState createState(const std::vector<float>& inputBuffer) {
        return {inputBuffer, 1.0f, false};
    }
};

// Monadic error handling for audio operations
template<typename T>
class AudioResult {
public:
    static AudioResult success(T value) {
        return AudioResult(std::move(value), "");
    }

    static AudioResult error(const std::string& message) {
        return AudioResult(std::nullopt, message);
    }

    bool isSuccess() const { return value_.has_value(); }
    bool isError() const { return !value_.has_value(); }

    const T& getValue() const { return *value_; }
    const std::string& getError() const { return error_; }

    template<typename F>
    auto map(F&& func) -> AudioResult<decltype(func(*value_))> {
        if (isError()) {
            return AudioResult<decltype(func(*value_))>::error(error_);
        }
        return AudioResult<decltype(func(*value_))>::success(func(*value_));
    }

    template<typename F>
    auto flatMap(F&& func) -> decltype(func(*value_)) {
        if (isError()) {
            return decltype(func(*value_))::error(error_);
        }
        return func(*value_);
    }

    template<typename F>
    AudioResult<T> onError(F&& errorHandler) const {
        if (isError()) {
            errorHandler(error_);
        }
        return *this;
    }

    T valueOr(const T& defaultValue) const {
        return isSuccess() ? *value_ : defaultValue;
    }

private:
    AudioResult(std::optional<T> value, std::string error)
        : value_(std::move(value)), error_(std::move(error)) {}

    std::optional<T> value_;
    std::string error_;
};

// Functional parameter mapping
class ParameterMapper {
public:
    using MappingFunction = std::function<float(float)>;

    static MappingFunction linear(float min, float max) {
        return [min, max](float normalized) {
            return min + normalized * (max - min);
        };
    }

    static MappingFunction exponential(float min, float max, float curve = 2.0f) {
        return [min, max, curve](float normalized) {
            return min + (max - min) * std::pow(normalized, curve);
        };
    }

    static MappingFunction logarithmic(float min, float max) {
        return [min, max](float normalized) {
            if (normalized <= 0.0f) return min;
            return min + (max - min) * std::log10(1.0f + 9.0f * normalized);
        };
    }

    static MappingFunction inverse(const MappingFunction& forward) {
        return [forward](float value) {
            // Simple binary search for inverse (not efficient, but functional)
            float low = 0.0f, high = 1.0f;
            for (int i = 0; i < 20; ++i) { // 20 iterations for precision
                float mid = (low + high) * 0.5f;
                float result = forward(mid);
                if (result < value) {
                    low = mid;
                } else {
                    high = mid;
                }
            }
            return (low + high) * 0.5f;
        };
    }
};

// Higher-order functions for audio analysis
namespace Analysis {

template<typename Predicate>
std::vector<size_t> findPeaks(const std::vector<float>& buffer, Predicate pred) {
    std::vector<size_t> peaks;

    for (size_t i = 1; i < buffer.size() - 1; ++i) {
        if (buffer[i] > buffer[i-1] && buffer[i] > buffer[i+1] && pred(buffer[i])) {
            peaks.push_back(i);
        }
    }

    return peaks;
}

template<typename Transform>
std::vector<float> transformBuffer(const std::vector<float>& buffer, Transform transform) {
    std::vector<float> result;
    result.reserve(buffer.size());

    std::transform(buffer.begin(), buffer.end(), std::back_inserter(result), transform);
    return result;
}

template<typename Reducer>
float reduceBuffer(const std::vector<float>& buffer, float initial, Reducer reducer) {
    return std::accumulate(buffer.begin(), buffer.end(), initial, reducer);
}

// Functional windowing
std::vector<float> applyWindow(const std::vector<float>& buffer,
                              const std::function<float(size_t, size_t)>& windowFunc) {
    std::vector<float> windowed;
    windowed.reserve(buffer.size());

    for (size_t i = 0; i < buffer.size(); ++i) {
        windowed.push_back(buffer[i] * windowFunc(i, buffer.size()));
    }

    return windowed;
}

// Common window functions
std::function<float(size_t, size_t)> hannWindow() {
    return [](size_t i, size_t size) {
        return 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
    };
}

std::function<float(size_t, size_t)> hammingWindow() {
    return [](size_t i, size_t size) {
        return 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (size - 1));
    };
}

} // namespace Analysis

// Functional audio generator
class FunctionalGenerator {
public:
    using GeneratorFunction = std::function<float(float)>; // time -> sample

    static GeneratorFunction sine(float frequency, float amplitude = 1.0f) {
        return [frequency, amplitude](float time) {
            return amplitude * std::sin(2.0f * M_PI * frequency * time);
        };
    }

    static GeneratorFunction sawtooth(float frequency, float amplitude = 1.0f) {
        return [frequency, amplitude](float time) {
            float phase = std::fmod(frequency * time, 1.0f);
            return amplitude * (2.0f * phase - 1.0f);
        };
    }

    static GeneratorFunction square(float frequency, float amplitude = 1.0f) {
        return [frequency, amplitude](float time) {
            float phase = std::fmod(frequency * time, 1.0f);
            return amplitude * (phase < 0.5f ? 1.0f : -1.0f);
        };
    }

    static GeneratorFunction noise(float amplitude = 1.0f) {
        return [amplitude](float time) {
            (void)time; // Noise doesn't depend on time
            return amplitude * (2.0f * static_cast<float>(std::rand()) / RAND_MAX - 1.0f);
        };
    }

    static std::vector<float> generate(const GeneratorFunction& generator,
                                     float duration, float sampleRate) {
        size_t numSamples = static_cast<size_t>(duration * sampleRate);
        std::vector<float> buffer;
        buffer.reserve(numSamples);

        for (size_t i = 0; i < numSamples; ++i) {
            float time = static_cast<float>(i) / sampleRate;
            buffer.push_back(generator(time));
        }

        return buffer;
    }
};

} // namespace ModernAudio

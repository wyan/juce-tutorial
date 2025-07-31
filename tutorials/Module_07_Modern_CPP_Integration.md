# Module 7: Modern C++ Concepts Integration

## Overview

This final module demonstrates how to leverage cutting-edge C++20 features and modern programming techniques to create robust, efficient, and maintainable audio applications. We'll explore concepts, coroutines, advanced template metaprogramming, and modern testing practices in the context of professional audio development.

## Learning Objectives

By the end of this module, you will:

- Master C++20 concepts and constraints for type-safe audio programming
- Implement advanced template metaprogramming for DSP optimization
- Learn modern memory management and RAII patterns
- Understand coroutines and async programming in audio contexts
- Apply functional programming concepts to audio processing
- Implement compile-time optimization techniques
- Master modern C++ testing and debugging approaches

## Table of Contents

1. [C++20 Concepts in Audio Programming](#cpp20-concepts)
2. [Advanced Template Metaprogramming](#template-metaprogramming)
3. [Modern Memory Management](#memory-management)
4. [Coroutines and Async Programming](#coroutines-async)
5. [Functional Programming in Audio](#functional-programming)
6. [Compile-Time Optimization](#compile-time-optimization)
7. [Modern Testing and Debugging](#testing-debugging)
8. [Performance Profiling](#performance-profiling)
9. [Practical Exercises](#exercises)

---

## 1. C++20 Concepts in Audio Programming {#cpp20-concepts}

C++20 concepts provide a powerful way to express constraints on template parameters, making audio code more readable, maintainable, and providing better error messages.

### Audio-Specific Concepts

```cpp
// Source/ModernCPP/AudioConcepts.h
#pragma once
#include <concepts>
#include <type_traits>

namespace ModernAudio {

// Basic audio sample concept
template<typename T>
concept AudioSample = std::floating_point<T> || std::integral<T>;

// Audio processor concept
template<typename T>
concept AudioProcessor = requires(T processor, float* buffer, int numSamples) {
    processor.processBlock(buffer, numSamples);
    { processor.getLatency() } -> std::convertible_to<int>;
    { processor.reset() } -> std::same_as<void>;
};

// Filter type concept
template<typename T>
concept FilterType = requires(T filter, float input, float frequency, float q) {
    { filter.process(input) } -> AudioSample;
    filter.setFrequency(frequency);
    filter.setQ(q);
    filter.reset();
};

// Parameter concept for type-safe parameter systems
template<typename T>
concept AudioParameter = requires(T param) {
    { param.getValue() } -> std::floating_point;
    param.setValue(std::declval<float>());
    { param.getName() } -> std::convertible_to<std::string>;
    { param.getRange() } -> std::same_as<std::pair<float, float>>;
};

// Plugin validation concept
template<typename T>
concept AudioPlugin = AudioProcessor<T> && requires(T plugin) {
    { plugin.getName() } -> std::convertible_to<std::string>;
    { plugin.getVersion() } -> std::convertible_to<std::string>;
    { plugin.getNumParameters() } -> std::convertible_to<int>;
};

} // namespace ModernAudio
```

### Concept-Constrained Template Functions

```cpp
// Source/ModernCPP/ConceptConstrainedDSP.h
#pragma once
#include "AudioConcepts.h"
#include <vector>
#include <algorithm>

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

private:
    SampleType processSample(SampleType input) {
        // Apply gain and soft clipping
        constexpr SampleType maxGain = SampleType(2.0);
        SampleType gained = input * gain_;
        return std::tanh(gained); // Soft clipping
    }

    SampleType gain_{1.0f};
};

// Concept-based function overloading
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

} // namespace ModernAudio
```

---

## 2. Advanced Template Metaprogramming {#template-metaprogramming}

Modern template metaprogramming enables compile-time algorithm selection and optimization for DSP operations.

### Compile-Time DSP Algorithm Selection

```cpp
// Source/ModernCPP/MetaprogrammingDSP.h
#pragma once
#include <type_traits>
#include <array>
#include <cmath>

namespace ModernAudio {

// Filter type enumeration for compile-time selection
enum class FilterType { LowPass, HighPass, BandPass, Notch };

// SFINAE-based filter implementation selection
template<FilterType Type, typename SampleType>
class MetaFilter {
private:
    static constexpr bool isLowPass = (Type == FilterType::LowPass);
    static constexpr bool isHighPass = (Type == FilterType::HighPass);
    static constexpr bool isBandPass = (Type == FilterType::BandPass);
    static constexpr bool isNotch = (Type == FilterType::Notch);

public:
    SampleType process(SampleType input) {
        if constexpr (isLowPass) {
            return processLowPass(input);
        } else if constexpr (isHighPass) {
            return processHighPass(input);
        } else if constexpr (isBandPass) {
            return processBandPass(input);
        } else if constexpr (isNotch) {
            return processNotch(input);
        }
    }

    void setParameters(SampleType frequency, SampleType q, SampleType sampleRate) {
        calculateCoefficients(frequency, q, sampleRate);
    }

private:
    std::array<SampleType, 3> b_{}; // Feedforward coefficients
    std::array<SampleType, 3> a_{}; // Feedback coefficients
    std::array<SampleType, 2> x_{}; // Input history
    std::array<SampleType, 2> y_{}; // Output history

    void calculateCoefficients(SampleType freq, SampleType q, SampleType sr) {
        const SampleType omega = 2 * M_PI * freq / sr;
        const SampleType sin_omega = std::sin(omega);
        const SampleType cos_omega = std::cos(omega);
        const SampleType alpha = sin_omega / (2 * q);

        if constexpr (isLowPass) {
            b_[0] = (1 - cos_omega) / 2;
            b_[1] = 1 - cos_omega;
            b_[2] = (1 - cos_omega) / 2;
        } else if constexpr (isHighPass) {
            b_[0] = (1 + cos_omega) / 2;
            b_[1] = -(1 + cos_omega);
            b_[2] = (1 + cos_omega) / 2;
        } else if constexpr (isBandPass) {
            b_[0] = alpha;
            b_[1] = 0;
            b_[2] = -alpha;
        } else if constexpr (isNotch) {
            b_[0] = 1;
            b_[1] = -2 * cos_omega;
            b_[2] = 1;
        }

        // Common denominator coefficients
        a_[0] = 1 + alpha;
        a_[1] = -2 * cos_omega;
        a_[2] = 1 - alpha;

        // Normalize
        for (auto& coeff : b_) coeff /= a_[0];
        a_[1] /= a_[0];
        a_[2] /= a_[0];
        a_[0] = 1;
    }

    SampleType processLowPass(SampleType input) { return processBiquad(input); }
    SampleType processHighPass(SampleType input) { return processBiquad(input); }
    SampleType processBandPass(SampleType input) { return processBiquad(input); }
    SampleType processNotch(SampleType input) { return processBiquad(input); }

    SampleType processBiquad(SampleType input) {
        SampleType output = b_[0] * input + b_[1] * x_[0] + b_[2] * x_[1]
                          - a_[1] * y_[0] - a_[2] * y_[1];

        // Update history
        x_[1] = x_[0];
        x_[0] = input;
        y_[1] = y_[0];
        y_[0] = output;

        return output;
    }
};

// Template-based audio processing pipeline
template<typename... Processors>
class ProcessingPipeline {
public:
    template<typename SampleType>
    SampleType process(SampleType input) {
        return processImpl<0>(input);
    }

private:
    std::tuple<Processors...> processors_;

    template<std::size_t Index, typename SampleType>
    SampleType processImpl(SampleType input) {
        if constexpr (Index < sizeof...(Processors)) {
            auto processed = std::get<Index>(processors_).process(input);
            return processImpl<Index + 1>(processed);
        } else {
            return input;
        }
    }
};

} // namespace ModernAudio
```

---

## 3. Modern Memory Management {#memory-management}

Modern C++ provides powerful tools for safe and efficient memory management in real-time audio applications.

### Smart Pointers and RAII Patterns

```cpp
// Source/ModernCPP/ModernMemoryManagement.h
#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace ModernAudio {

// RAII-based audio buffer management
class AudioBufferManager {
public:
    explicit AudioBufferManager(size_t bufferSize, size_t numChannels)
        : bufferSize_(bufferSize), numChannels_(numChannels) {
        allocateBuffers();
    }

    // Move-only semantics for efficient resource management
    AudioBufferManager(const AudioBufferManager&) = delete;
    AudioBufferManager& operator=(const AudioBufferManager&) = delete;

    AudioBufferManager(AudioBufferManager&&) = default;
    AudioBufferManager& operator=(AudioBufferManager&&) = default;

    ~AudioBufferManager() = default; // RAII cleanup

    float* getChannelData(size_t channel) {
        if (channel < buffers_.size()) {
            return buffers_[channel].get();
        }
        return nullptr;
    }

    void resizeBuffers(size_t newSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        bufferSize_ = newSize;
        allocateBuffers();
    }

private:
    size_t bufferSize_;
    size_t numChannels_;
    std::vector<std::unique_ptr<float[]>> buffers_;
    std::mutex mutex_;

    void allocateBuffers() {
        buffers_.clear();
        buffers_.reserve(numChannels_);

        for (size_t i = 0; i < numChannels_; ++i) {
            buffers_.emplace_back(std::make_unique<float[]>(bufferSize_));
        }
    }
};

// Custom allocator for real-time audio processing
template<typename T>
class RealtimeAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = RealtimeAllocator<U>;
    };

    RealtimeAllocator() = default;

    template<typename U>
    RealtimeAllocator(const RealtimeAllocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }

        // Use memory pool for real-time safety
        return static_cast<pointer>(memoryPool_.allocate(n * sizeof(T)));
    }

    void deallocate(pointer p, size_type n) noexcept {
        memoryPool_.deallocate(p, n * sizeof(T));
    }

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

private:
    static MemoryPool memoryPool_;
};

// Lock-free circular buffer for real-time audio
template<typename T, size_t Size>
class LockFreeCircularBuffer {
public:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");

    bool push(const T& item) {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = increment(current_tail);

        if (next_tail != head_.load(std::memory_order_acquire)) {
            buffer_[current_tail] = item;
            tail_.store(next_tail, std::memory_order_release);
            return true;
        }
        return false; // Buffer full
    }

    bool pop(T& item) {
        const auto current_head = head_.load(std::memory_order_relaxed);

        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }

        item = buffer_[current_head];
        head_.store(increment(current_head), std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    bool full() const {
        const auto next_tail = increment(tail_.load(std::memory_order_acquire));
        return next_tail == head_.load(std::memory_order_acquire);
    }

private:
    static constexpr size_t increment(size_t idx) {
        return (idx + 1) & (Size - 1);
    }

    std::array<T, Size> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

} // namespace ModernAudio
```

---

## 4. Coroutines and Async Programming {#coroutines-async}

C++20 coroutines enable elegant asynchronous programming patterns for audio applications.

### Async Audio File Processing

```cpp
// Source/ModernCPP/AsyncAudioProcessing.h
#pragma once
#include <coroutine>
#include <future>
#include <vector>
#include <string>
#include <memory>

namespace ModernAudio {

// Simple coroutine task type
template<typename T>
class Task {
public:
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_value(T value) {
            result_ = std::move(value);
        }

        void unhandled_exception() {
            exception_ = std::current_exception();
        }

        T result_;
        std::exception_ptr exception_;
    };

    explicit Task(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    T get() {
        if (handle_.promise().exception_) {
            std::rethrow_exception(handle_.promise().exception_);
        }
        return std::move(handle_.promise().result_);
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

// Async audio file loader
class AsyncAudioLoader {
public:
    Task<std::vector<float>> loadAudioFileAsync(const std::string& filename) {
        // Simulate async file loading
        auto future = std::async(std::launch::async, [filename]() {
            return loadAudioFile(filename);
        });

        // Yield control while waiting
        co_await std::suspend_always{};

        auto audioData = future.get();
        co_return audioData;
    }

    Task<void> processAudioBatchAsync(const std::vector<std::string>& filenames) {
        std::vector<Task<std::vector<float>>> tasks;

        // Start all loading tasks
        for (const auto& filename : filenames) {
            tasks.emplace_back(loadAudioFileAsync(filename));
        }

        // Process results as they become available
        for (auto& task : tasks) {
            auto audioData = task.get();
            processAudioData(audioData);
            co_await std::suspend_always{}; // Yield between processing
        }

        co_return;
    }

private:
    static std::vector<float> loadAudioFile(const std::string& filename) {
        // Simulate file loading with some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Return dummy audio data for demonstration
        std::vector<float> audioData(44100); // 1 second at 44.1kHz
        std::generate(audioData.begin(), audioData.end(),
            []() { return static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f; });

        return audioData;
    }

    void processAudioData(const std::vector<float>& data) {
        // Simulate audio processing
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};

// Coroutine-based parameter automation
class ParameterAutomation {
public:
    Task<void> animateParameter(float startValue, float endValue,
                               float durationSeconds, float sampleRate) {
        const int totalSamples = static_cast<int>(durationSeconds * sampleRate);
        const float increment = (endValue - startValue) / totalSamples;

        float currentValue = startValue;

        for (int i = 0; i < totalSamples; ++i) {
            setParameterValue(currentValue);
            currentValue += increment;

            // Yield every 64 samples to avoid blocking
            if (i % 64 == 0) {
                co_await std::suspend_always{};
            }
        }

        setParameterValue(endValue); // Ensure exact end value
        co_return;
    }

private:
    void setParameterValue(float value) {
        // Update parameter value
        currentValue_.store(value, std::memory_order_release);
    }

    std::atomic<float> currentValue_{0.0f};
};

} // namespace ModernAudio
```

---

## 5. Functional Programming in Audio {#functional-programming}

Functional programming concepts can make audio processing code more composable and testable.

### Higher-Order Functions for Audio Processing

```cpp
// Source/ModernCPP/FunctionalAudio.h
#pragma once
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>
#include <optional>

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

AudioEffect delay(int delaySamples) {
    return [delaySamples, buffer = std::vector<float>(delaySamples, 0.0f),
            index = 0](float input) mutable {
        float output = buffer[index];
        buffer[index] = input;
        index = (index + 1) % delaySamples;
        return output;
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
};

} // namespace ModernAudio
```

---

## 6. Compile-Time Optimization {#compile-time-optimization}

Modern C++ provides powerful compile-time computation capabilities that can significantly optimize audio processing.

### Constexpr and Consteval for Audio Calculations

```cpp
// Source/ModernCPP/CompileTimeOptimization.h
#pragma once
#include <array>
#include <cmath>
#include <numbers>

namespace ModernAudio {

// Compile-time audio format detection
enum class AudioFormat { PCM16, PCM24, PCM32, Float32, Float64 };

template<typename T>
consteval AudioFormat detectAudioFormat() {
    if constexpr (std::is_same_v<T, int16_t>) {
        return AudioFormat::PCM16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        if constexpr (sizeof(T) == 3) {
            return AudioFormat::PCM24;
        } else {
            return AudioFormat::PCM32;
        }
    } else if constexpr (std::is_same_v<T, float>) {
        return AudioFormat::Float32;
    } else if constexpr (std::is_same_v<T, double>) {
        return AudioFormat::Float64;
    } else {
        static_assert(false, "Unsupported audio format");
    }
}

// Compile-time sine wave table generation
template<size_t TableSize>
constexpr std::array<float, TableSize> generateSineTable() {
    std::array<float, TableSize> table{};

    for (size_t i = 0; i < TableSize; ++i) {
        table[i] = std::sin(2.0 * std::numbers::pi * i / TableSize);
    }

    return table;
}

// Compile-time window function generation
enum class WindowType { Hann, Hamming, Blackman, Kaiser };

template<WindowType Type, size_t Size>
constexpr std::array<float, Size> generateWindow() {
    std::array<float, Size> window{};

    if constexpr (Type == WindowType::Hann) {
        for (size_t i = 0; i < Size; ++i) {
            window[i] = 0.5f * (1.0f - std::cos(2.0 * std::numbers::pi * i / (Size - 1)));
        }
    } else if constexpr (Type == WindowType::Hamming) {
        for (size_t i = 0; i < Size; ++i) {
            window[i] = 0.54f - 0.46f * std::cos(2.0 * std::numbers::pi * i / (Size - 1));
        }
    } else if constexpr (Type == WindowType::Blackman) {
        for (size_t i = 0; i < Size; ++i) {
            const float n = static_cast<float>(i) / (Size - 1);
            window[i] = 0.42f - 0.5f * std::cos(2.0 * std::numbers::pi * n)
                       + 0.08f * std::cos(4.0 * std::numbers::pi * n);
        }
    }

    return window;
}

// Compile-time filter coefficient calculation
template<size_t Order>
struct ButterworthCoefficients {
    std::array<double, Order + 1> b;
    std::array<double, Order + 1> a;

    constexpr ButterworthCoefficients(double cutoff, double sampleRate) {
        // Simplified Butterworth coefficient calculation
        const double omega = 2.0 * std::numbers::pi * cutoff / sampleRate;
        const double cosOmega = std::cos(omega);
        const double sinOmega = std::sin(omega);

        // First-order case for demonstration
        if constexpr (Order == 1) {
            const double alpha = sinOmega / 2.0;
            b[0] = (1.0 - cosOmega) / 2.0;
            b[1] = 1.0 - cosOmega;
            a[0] = 1.0 + alpha;
            a[1] = -2.0 * cosOmega;
        }
    }
};

// Template-based loop unrolling for DSP
template<size_t N>
constexpr float dotProduct(const std::array<float, N>& a, const std::array<float, N>& b) {
    float result = 0.0f;

    // Unroll loop at compile time
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((result += a[Is] * b[Is]), ...);
    }(std::make_index_sequence<N>{});

    return result;
}

// Compile-time plugin metadata generation
struct PluginMetadata {
    const char* name;
    const char* version;
    const char* manufacturer;
    int numParameters;
    bool isSynth;
};

template<typename PluginType>
consteval PluginMetadata generatePluginMetadata() {
    if constexpr (std::is_same_v<PluginType, class Reverb>) {
        return {"Modern Reverb", "1.0.0", "ModernAudio", 8, false};
    } else if constexpr (std::is_same_v<PluginType, class Synthesizer>) {
        return {"Modern Synth", "1.0.0", "ModernAudio", 16, true};
    } else {
        return {"Unknown Plugin", "0.0.0", "Unknown", 0, false};
    }
}

// Compile-time audio processing optimization
template<size_t BufferSize>
class OptimizedProcessor {
public:
    void processBlock(std::array<float, BufferSize>& buffer) {
        // Use compile-time constants for optimization
        static constexpr auto sineTable = generateSineTable<1024>();
        static constexpr auto window = generateWindow<WindowType::Hann, BufferSize>();

        // Process with compile-time optimizations
        for (size_t i = 0; i < BufferSize; ++i) {
            buffer[i] *= window[i];
        }
    }
};

} // namespace ModernAudio
```

---

## 7. Modern Testing and Debugging {#testing-debugging}

Modern C++ testing frameworks provide powerful tools for ensuring audio code quality and performance.

### Setting Up Catch2 Testing Framework

```cpp
// Source/Tests/ModernAudioTests.cpp
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../ModernCPP/AudioConcepts.h"
#include "../ModernCPP/ConceptConstrainedDSP.h"
#include "../ModernCPP/FunctionalAudio.h"

using namespace ModernAudio;

// Concept validation tests
TEST_CASE("Audio Concepts Validation", "[concepts]") {
    SECTION("AudioSample concept") {
        REQUIRE(AudioSample<float>);
        REQUIRE(AudioSample<double>);
        REQUIRE(AudioSample<int16_t>);
        REQUIRE_FALSE(AudioSample<std::string>);
    }

    SECTION("AudioProcessor concept") {
        class TestProcessor {
        public:
            void processBlock(float* buffer, int numSamples) {}
            int getLatency() const { return 0; }
            void reset() {}
        };

        REQUIRE(AudioProcessor<TestProcessor>);
    }
}

// Property-based testing for audio algorithms
TEST_CASE("Audio Processing Properties", "[properties]") {
    SECTION("Gain processing preserves DC offset") {
        const float gain = 2.0f;
        const float dcOffset = 0.5f;
        std::vector<float> buffer(1024, dcOffset);

        ConceptConstrainedProcessor<float> processor;
        // Apply gain processing

        // Property: DC component should be scaled by gain
        float sum = std::accumulate(buffer.begin(), buffer.end(), 0.0f);
        float averageAfter = sum / buffer.size();

        REQUIRE(std::abs(averageAfter - dcOffset * gain) < 0.001f);
    }

    SECTION("Normalization preserves signal shape") {
        std::vector<float> original = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
        std::vector<float> normalized = original;

        normalizeAudio(normalized);

        // Property: relative amplitudes should be preserved
        for (size_t i = 1; i < original.size(); ++i) {
            float originalRatio = original[i] / original[0];
            float normalizedRatio = normalized[i] / normalized[0];
            REQUIRE(std::abs(originalRatio - normalizedRatio) < 0.001f);
        }
    }
}

// Performance benchmarks
TEST_CASE("Performance Benchmarks", "[benchmark]") {
    SECTION("Filter processing benchmark") {
        constexpr size_t bufferSize = 1024;
        std::vector<float> buffer(bufferSize);
        std::generate(buffer.begin(), buffer.end(),
            []() { return static_cast<float>(std::rand()) / RAND_MAX; });

        MetaFilter<FilterType::LowPass, float> filter;
        filter.setParameters(1000.0f, 0.707f, 44100.0f);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; ++i) {
            for (auto& sample : buffer) {
                sample = filter.process(sample);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Benchmark assertion - should process 1M samples in reasonable time
        REQUIRE(duration.count() < 100000); // Less than 100ms
    }
}

// Unit tests for functional audio processing
TEST_CASE("Functional Audio Processing", "[functional]") {
    SECTION("Effect chain composition") {
        EffectChain chain;
        chain.addEffect(Effects::gain(2.0f))
             .addEffect(Effects::softClip(0.8f));

        float input = 0.5f;
        float output = chain.process(input);

        // Should apply gain then soft clipping
        REQUIRE(output > input);
        REQUIRE(output <= 0.8f); // Clipped at threshold
    }

    SECTION("Immutable processing state") {
        std::vector<float> inputBuffer = {0.1f, 0.2f, 0.3f};
        auto initialState = ImmutableAudioProcessor::createState(inputBuffer);

        auto gainedState = initialState.withGain(2.0f);
        auto processedState = gainedState.processedWith(Effects::gain(2.0f));

        // Original state should be unchanged
        REQUIRE(initialState.gain == 1.0f);
        REQUIRE(gainedState.gain == 2.0f);
    }
}

// Memory management tests
TEST_CASE("Memory Management", "[memory]") {
    SECTION("RAII buffer management") {
        {
            AudioBufferManager manager(1024, 2);
            auto* channel0 = manager.getChannelData(0);
            auto* channel1 = manager.getChannelData(1);

            REQUIRE(channel0 != nullptr);
            REQUIRE(channel1 != nullptr);
            REQUIRE(channel0 != channel1);
        }
        // Buffers should be automatically cleaned up
    }

    SECTION("Lock-free circular buffer") {
        LockFreeCircularBuffer<float, 16> buffer;

        REQUIRE(buffer.empty());
        REQUIRE_FALSE(buffer.full());

        // Fill buffer
        for (int i = 0; i < 15; ++i) {
            REQUIRE(buffer.push(static_cast<float>(i)));
        }

        REQUIRE(buffer.full());

        // Empty buffer
        float value;
        for (int i = 0; i < 15; ++i) {
            REQUIRE(buffer.pop(value));
            REQUIRE(value == static_cast<float>(i));
        }

        REQUIRE(buffer.empty());
    }
}
```

### Modern Debugging Techniques

```cpp
// Source/ModernCPP/DebuggingTools.h
#pragma once
#include <iostream>
#include <chrono>
#include <string>
#include <source_location>

namespace ModernAudio::Debug {

// Modern logging with source location
class Logger {
public:
    template<typename... Args>
    static void log(const std::string& message,
                   const std::source_location& location = std::source_location::current(),
                   Args&&... args) {
        std::cout << "[" << location.file_name() << ":" << location.line() << "] "
                  << message;
        ((std::cout << " " << args), ...);
        std::cout << std::endl;
    }

    template<typename... Args>
    static void logAudio(const std::string& message, Args&&... args) {
        log("[AUDIO] " + message, std::source_location::current(), args...);
    }
};

// RAII performance profiler
class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name)
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedProfiler() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        Logger::log("Profile [{}]: {} μs", name_, duration.count());
    }

private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
};

#define PROFILE_SCOPE(name) ScopedProfiler _prof(name)
#define PROFILE_FUNCTION() ScopedProfiler _prof(__FUNCTION__)

// Audio buffer analyzer for debugging
class AudioAnalyzer {
public:
    struct BufferStats {
        float min, max, mean, rms;
        bool hasClipping;
        bool hasSilence;
    };

    static BufferStats analyzeBuffer(const std::vector<float>& buffer) {
        BufferStats stats{};

        if (buffer.empty()) return stats;

        stats.min = *std::min_element(buffer.begin(), buffer.end());
        stats.max = *std::max_element(buffer.begin(), buffer.end());

        float sum = std::accumulate(buffer.begin(), buffer.end(), 0.0f);
        stats.mean = sum / buffer.size();

        float sumSquares = std::accumulate(buffer.begin(), buffer.end(), 0.0f,
            [](float acc, float val) { return acc + val * val; });
        stats.rms = std::sqrt(sumSquares / buffer.size());

        stats.hasClipping = (stats.min <= -1.0f) || (stats.max >= 1.0f);
        stats.hasSilence = (stats.rms < 1e-6f);

        return stats;
    }

    static void printBufferStats(const std::vector<float>& buffer,
                                const std::string& name = "Buffer") {
        auto stats = analyzeBuffer(buffer);
        Logger::logAudio("{} Stats - Min: {:.6f}, Max: {:.6f}, Mean: {:.6f}, RMS: {:.6f}",
                        name, stats.min, stats.max, stats.mean, stats.rms);

        if (stats.hasClipping) {
            Logger::logAudio("WARNING: {} contains clipping!", name);
        }
        if (stats.hasSilence) {
            Logger::logAudio("INFO: {} appears to be silent", name);
        }
    }
};

} // namespace ModernAudio::Debug
```

---

## 8. Performance Profiling {#performance-profiling}

Modern profiling techniques help optimize audio processing performance.

```cpp
// Source/ModernCPP/PerformanceProfiler.h
#pragma once
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <string>

namespace ModernAudio::Profiling {

class PerformanceProfiler {
public:
    struct ProfileData {
        std::vector<double> samples;
        double totalTime = 0.0;
        size_t callCount = 0;

        double getAverage() const {
            return callCount > 0 ? totalTime / callCount : 0.0;
        }

        double getMin() const {
            return samples.empty() ? 0.0 : *std::min_element(samples.begin(), samples.end());
        }

        double getMax() const {
            return samples.empty() ? 0.0 : *std::max_element(samples.begin(), samples.end());
        }

        double getPercentile(double p) const {
            if (samples.empty()) return 0.0;

            auto sorted = samples;
            std::sort(sorted.begin(), sorted.end());

            size_t index = static_cast<size_t>(p * (sorted.size() - 1));
            return sorted[index];
        }
    };

    class ScopedTimer {
    public:
        ScopedTimer(PerformanceProfiler& profiler, const std::string& name)
            : profiler_(profiler), name_(name),
              start_(std::chrono::high_resolution_clock::now()) {}

        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::micro>(end - start_);
            profiler_.addSample(name_, duration.count());
        }

    private:
        PerformanceProfiler& profiler_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point start_;
    };

    void addSample(const std::string& name, double microseconds) {
        auto& data = profiles_[name];
        data.samples.push_back(microseconds);
        data.totalTime += microseconds;
        data.callCount++;

        // Keep only recent samples to avoid memory growth
        if (data.samples.size() > maxSamples_) {
            data.samples.erase(data.samples.begin());
        }
    }

    ScopedTimer createTimer(const std::string& name) {
        return ScopedTimer(*this, name);
    }

    void printReport() const {
        std::cout << "\n=== Performance Profile Report ===\n";
        std::cout << std::left << std::setw(20) << "Function"
                  << std::setw(10) << "Calls"
                  << std::setw(12) << "Avg (μs)"
                  << std::setw(12) << "Min (μs)"
                  << std::setw(12) << "Max (μs)"
                  << std::setw(12) << "95th %ile"
                  << "\n";
        std::cout << std::string(78, '-') << "\n";

        for (const auto& [name, data] : profiles_) {
            std::cout << std::left << std::setw(20) << name
                      << std::setw(10) << data.callCount
                      << std::setw(12) << std::fixed << std::setprecision(2) << data.getAverage()
                      << std::setw(12) << data.getMin()
                      << std::setw(12) << data.getMax()
                      << std::setw(12) << data.getPercentile(0.95)
                      << "\n";
        }
        std::cout << std::string(78, '=') << "\n";
    }

    void reset() {
        profiles_.clear();
    }

private:
    std::map<std::string, ProfileData> profiles_;
    static constexpr size_t maxSamples_ = 1000;
};

// Global profiler instance
inline PerformanceProfiler& getGlobalProfiler() {
    static PerformanceProfiler profiler;
    return profiler;
}

#define PROFILE_FUNCTION_GLOBAL() \
    auto _timer = getGlobalProfiler().createTimer(__FUNCTION__)

#define PROFILE_SCOPE_GLOBAL(name) \
    auto _timer = getGlobalProfiler().createTimer(name)

// Real-time safe profiler for audio threads
class RealtimeProfiler {
public:
    struct RealtimeStats {
        std::atomic<uint64_t> totalCycles{0};
        std::atomic<uint64_t> callCount{0};
        std::atomic<uint64_t> maxCycles{0};

        double getAverageMicroseconds() const {
            uint64_t calls = callCount.load();
            if (calls == 0) return 0.0;

            uint64_t total = totalCycles.load();
            return cyclesToMicroseconds(total / calls);
        }

        double getMaxMicroseconds() const {
            return cyclesToMicroseconds(maxCycles.load());
        }

    private:
        static double cyclesToMicroseconds(uint64_t cycles) {
            // Approximate conversion - would need calibration for accuracy
            return static_cast<double>(cycles) / 3000.0; // Assuming 3GHz CPU
        }
    };

    class RealtimeTimer {
    public:
        RealtimeTimer(RealtimeStats& stats) : stats_(stats) {
            start_ = __builtin_ia32_rdtsc(); // x86-specific cycle counter
        }

        ~RealtimeTimer() {
            uint64_t end = __builtin_ia32_rdtsc();
            uint64_t cycles = end - start_;

            stats_.totalCycles.fetch_add(cycles, std::memory_order_relaxed);
            stats_.callCount.fetch_add(1, std::memory_order_relaxed);

            // Update max cycles
            uint64_t currentMax = stats_.maxCycles.load(std::memory_order_relaxed);
            while (cycles > currentMax &&
                   !stats_.maxCycles.compare_exchange_weak(currentMax, cycles,
                                                          std::memory_order_relaxed)) {
                // Retry if another thread updated maxCycles
            }
        }

    private:
        RealtimeStats& stats_;
        uint64_t start_;
    };

    RealtimeTimer createTimer(const std::string& name) {
        return RealtimeTimer(stats_[name]);
    }

    void printStats() const {
        std::cout << "\n=== Realtime Profile Stats ===\n";
        for (const auto& [name, stats] : stats_) {
            std::cout << name << ": "
                      << "Avg: " << stats.getAverageMicroseconds() << "μs, "
                      << "Max: " << stats.getMaxMicroseconds() << "μs, "
                      << "Calls: " << stats.callCount.load() << "\n";
        }
    }

private:
    std::map<std::string, RealtimeStats> stats_;
};

} // namespace ModernAudio::Profiling
```

---

## 9. Practical Exercises {#exercises}

### Exercise 1: Refactor DSP Code with C++20 Concepts

**Objective**: Take existing DSP code and refactor it using C++20 concepts for better type safety and error messages.

**Task**: Refactor the following legacy filter code:

```cpp
// Legacy code to refactor
template<typename T>
class LegacyFilter {
public:
    void setFrequency(T freq) { frequency_ = freq; }
    T process(T input) { /* implementation */ }
private:
    T frequency_;
};
```

**Solution**:
```cpp
// Modern refactored version
template<AudioSample T>
class ModernFilter {
public:
    void setFrequency(T freq) requires std::floating_point<T> {
        static_assert(std::is_floating_point_v<T>, "Frequency must be floating point");
        frequency_ = freq;
    }

    T process(T input) requires AudioSample<T> {
        // Type-safe processing with concept constraints
        return applyFilter(input);
    }

private:
    T frequency_{440.0};

    T applyFilter(T input) {
        // Implementation with compile-time optimizations
        return input * std::sin(2 * std::numbers::pi * frequency_ / 44100);
    }
};
```

### Exercise 2: Async Audio File Processing

**Objective**: Implement an async audio file processor using C++20 coroutines.

**Task**: Create a system that can load and process multiple audio files concurrently.

**Solution**:
```cpp
// Implementation in Source/Exercises/AsyncFileProcessor.h
#include "../ModernCPP/AsyncAudioProcessing.h"

class AudioFileProcessor {
public:
    Task<std::vector<ProcessedAudio>> processBatch(
        const std::vector<std::string>& filenames) {

        std::vector<Task<ProcessedAudio>> tasks;

        for (const auto& filename : filenames) {
            tasks.emplace_back(processFileAsync(filename));
        }

        std::vector<ProcessedAudio> results;
        for (auto& task : tasks) {
            results.emplace_back(co_await task);
        }

        co_return results;
    }

private:
    Task<ProcessedAudio> processFileAsync(const std::string& filename) {
        auto audioData = co_await loadAudioAsync(filename);
        auto processed = co_await applyEffectsAsync(audioData);
        co_return processed;
    }
};
```

### Exercise 3: Compile-Time Optimized Processing Pipeline

**Objective**: Create a processing pipeline that uses compile-time optimization techniques.

**Task**: Build a multi-stage audio processor with compile-time algorithm selection.

**Solution**:
```cpp
// Compile-time optimized pipeline
template<FilterType... Filters>
class OptimizedPipeline {
public:
    template<AudioSample T>
    T process(T input) {
        return processStages<0>(input);
    }

private:
    std::tuple<MetaFilter<Filters, float>...> filters_;

    template<size_t Index, AudioSample T>
    T processStages(T input) {
        if constexpr (Index < sizeof...(Filters)) {
            auto processed = std::get<Index>(filters_).process(input);
            return processStages<Index + 1>(processed);
        } else {
            return input;
        }
    }
};

// Usage
using MyPipeline = OptimizedPipeline<FilterType::HighPass,
                                   FilterType::LowPass,
                                   FilterType::BandPass>;
```

### Exercise 4: Comprehensive Test Suite

**Objective**: Create a comprehensive test suite using modern C++ testing practices.

**Task**: Implement unit tests, integration tests, and performance benchmarks for the audio processing components.

**Solution**: See the complete test implementation in the Testing section above.

---

## Conclusion

This module has demonstrated how modern C++ features can dramatically improve audio application development. Key takeaways include:

1. **C++20 Concepts** provide type safety and better error messages for audio code
2. **Template Metaprogramming** enables compile-time optimizations for DSP algorithms
3. **Modern Memory Management** ensures safe and efficient resource handling
4. **Coroutines** enable elegant asynchronous programming patterns
5. **Functional Programming** concepts improve code composability and testability
6. **Compile-Time Optimization** can significantly improve runtime performance
7. **Modern Testing** frameworks provide comprehensive quality assurance

By applying these techniques, you can create audio applications that are not only performant but also maintainable, safe, and expressive. The combination of type safety, compile-time optimization, and modern programming patterns represents the future of professional audio software development.

## Next Steps

- Experiment with the provided code examples in your own projects
- Explore additional C++20 and C++23 features as they become available
- Consider contributing to open-source audio libraries using these modern techniques
- Stay updated with the evolving C++ standards and their applications in audio programming

The journey of mastering modern C++ in audio development is ongoing, but the foundation provided in this tutorial series will serve you well as the language and ecosystem continue to evolve.

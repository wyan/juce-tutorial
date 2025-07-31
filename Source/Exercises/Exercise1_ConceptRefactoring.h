#pragma once
#include "../ModernCPP/AudioConcepts.h"
#include <vector>
#include <cmath>
#include <type_traits>

namespace ModernAudio::Exercises {

// Exercise 1: Refactor DSP Code with C++20 Concepts
//
// OBJECTIVE: Take existing DSP code and refactor it using C++20 concepts
// for better type safety and error messages.

// ============================================================================
// LEGACY CODE (Before Refactoring)
// ============================================================================

template<typename T>
class LegacyFilter {
public:
    void setFrequency(T freq) {
        frequency_ = freq;
    }

    T process(T input) {
        // Simple one-pole lowpass filter
        output_ = output_ + cutoff_ * (input - output_);
        return output_;
    }

    void setSampleRate(T sampleRate) {
        sampleRate_ = sampleRate;
        updateCutoff();
    }

private:
    T frequency_{1000.0};
    T sampleRate_{44100.0};
    T output_{0.0};
    T cutoff_{0.1};

    void updateCutoff() {
        cutoff_ = 1.0 - std::exp(-2.0 * M_PI * frequency_ / sampleRate_);
    }
};

// Problems with legacy code:
// 1. No type constraints - can be instantiated with inappropriate types
// 2. No compile-time validation
// 3. Poor error messages when used incorrectly
// 4. No interface guarantees

// ============================================================================
// MODERN REFACTORED VERSION (Using C++20 Concepts)
// ============================================================================

// Step 1: Define audio-specific concepts (if not using AudioConcepts.h)
template<typename T>
concept NumericType = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

template<typename T>
concept FloatingPointSample = std::floating_point<T>;

template<typename T>
concept AudioSampleType = NumericType<T>;

// Step 2: Create concept-constrained filter
template<AudioSampleType T>
class ModernFilter {
public:
    // Frequency must be floating point for precision
    void setFrequency(T freq) requires FloatingPointSample<T> {
        static_assert(std::is_floating_point_v<T>, "Frequency must be floating point");
        if (freq <= T(0) || freq >= sampleRate_ / T(2)) {
            throw std::invalid_argument("Frequency must be positive and below Nyquist");
        }
        frequency_ = freq;
        updateCutoff();
    }

    // Process method with concept constraints
    T process(T input) requires AudioSampleType<T> {
        // Type-safe processing with compile-time optimizations
        return applyFilter(input);
    }

    void setSampleRate(T sampleRate) requires FloatingPointSample<T> {
        if (sampleRate <= T(0)) {
            throw std::invalid_argument("Sample rate must be positive");
        }
        sampleRate_ = sampleRate;
        updateCutoff();
    }

    void reset() {
        output_ = T(0);
    }

    // Satisfy AudioProcessor concept requirements
    void processBlock(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = static_cast<float>(process(static_cast<T>(buffer[i])));
        }
    }

    int getLatency() const { return 0; }

    // Additional concept-constrained methods
    T getFrequency() const { return frequency_; }
    T getSampleRate() const { return sampleRate_; }

private:
    T frequency_{T(1000)};
    T sampleRate_{T(44100)};
    T output_{T(0)};
    T cutoff_{T(0.1)};

    T applyFilter(T input) {
        // Implementation with compile-time optimizations
        if constexpr (std::is_floating_point_v<T>) {
            // Use high-precision calculation for floating point
            output_ = output_ + cutoff_ * (input - output_);
        } else {
            // Use integer-safe calculation for integer types
            auto temp = static_cast<double>(output_) +
                       static_cast<double>(cutoff_) *
                       (static_cast<double>(input) - static_cast<double>(output_));
            output_ = static_cast<T>(temp);
        }
        return output_;
    }

    void updateCutoff() {
        if constexpr (std::is_floating_point_v<T>) {
            cutoff_ = T(1) - std::exp(-T(2) * T(M_PI) * frequency_ / sampleRate_);
        } else {
            // Approximation for integer types
            double temp = 1.0 - std::exp(-2.0 * M_PI *
                         static_cast<double>(frequency_) / static_cast<double>(sampleRate_));
            cutoff_ = static_cast<T>(temp * std::numeric_limits<T>::max());
        }
    }
};

// Step 3: Concept validation and testing
template<typename T>
constexpr bool validateModernFilter() {
    // Compile-time validation
    static_assert(AudioSampleType<T>, "T must be a valid audio sample type");
    return true;
}

// Step 4: Usage examples and demonstrations
class Exercise1Demo {
public:
    static void runDemo() {
        std::cout << "=== Exercise 1: Concept Refactoring Demo ===\n";

        // Valid usage examples
        demonstrateValidUsage();

        // Show improved error messages
        demonstrateErrorMessages();

        // Performance comparison
        performanceComparison();
    }

private:
    static void demonstrateValidUsage() {
        std::cout << "\n--- Valid Usage Examples ---\n";

        // Float filter
        ModernFilter<float> floatFilter;
        floatFilter.setFrequency(1000.0f);
        floatFilter.setSampleRate(44100.0f);

        float input = 0.5f;
        float output = floatFilter.process(input);
        std::cout << "Float filter: " << input << " -> " << output << "\n";

        // Double filter for high precision
        ModernFilter<double> doubleFilter;
        doubleFilter.setFrequency(1000.0);
        doubleFilter.setSampleRate(96000.0);

        double preciseInput = 0.123456789;
        double preciseOutput = doubleFilter.process(preciseInput);
        std::cout << "Double filter: " << preciseInput << " -> " << preciseOutput << "\n";

        // Integer filter (with appropriate scaling)
        ModernFilter<int16_t> intFilter;
        intFilter.setSampleRate(44100.0f);
        intFilter.setFrequency(500.0f);

        int16_t intInput = 16384; // Half scale
        int16_t intOutput = intFilter.process(intInput);
        std::cout << "Int16 filter: " << intInput << " -> " << intOutput << "\n";
    }

    static void demonstrateErrorMessages() {
        std::cout << "\n--- Error Handling Examples ---\n";

        try {
            ModernFilter<float> filter;
            filter.setSampleRate(44100.0f);
            filter.setFrequency(-100.0f); // Invalid negative frequency
        } catch (const std::exception& e) {
            std::cout << "Caught expected error: " << e.what() << "\n";
        }

        try {
            ModernFilter<float> filter;
            filter.setSampleRate(44100.0f);
            filter.setFrequency(25000.0f); // Above Nyquist
        } catch (const std::exception& e) {
            std::cout << "Caught expected error: " << e.what() << "\n";
        }

        // Compile-time validation examples
        static_assert(validateModernFilter<float>());
        static_assert(validateModernFilter<double>());
        static_assert(validateModernFilter<int16_t>());
        // static_assert(validateModernFilter<std::string>()); // Would fail at compile time

        std::cout << "Compile-time validations passed!\n";
    }

    static void performanceComparison() {
        std::cout << "\n--- Performance Comparison ---\n";

        const size_t numSamples = 100000;
        std::vector<float> testData(numSamples);

        // Generate test signal
        for (size_t i = 0; i < numSamples; ++i) {
            testData[i] = std::sin(2.0f * M_PI * 440.0f * i / 44100.0f);
        }

        // Test legacy filter
        LegacyFilter<float> legacyFilter;
        legacyFilter.setFrequency(1000.0f);
        legacyFilter.setSampleRate(44100.0f);

        auto start = std::chrono::high_resolution_clock::now();
        for (float& sample : testData) {
            sample = legacyFilter.process(sample);
        }
        auto legacyTime = std::chrono::high_resolution_clock::now() - start;

        // Reset test data
        for (size_t i = 0; i < numSamples; ++i) {
            testData[i] = std::sin(2.0f * M_PI * 440.0f * i / 44100.0f);
        }

        // Test modern filter
        ModernFilter<float> modernFilter;
        modernFilter.setFrequency(1000.0f);
        modernFilter.setSampleRate(44100.0f);

        start = std::chrono::high_resolution_clock::now();
        for (float& sample : testData) {
            sample = modernFilter.process(sample);
        }
        auto modernTime = std::chrono::high_resolution_clock::now() - start;

        auto legacyMicros = std::chrono::duration_cast<std::chrono::microseconds>(legacyTime).count();
        auto modernMicros = std::chrono::duration_cast<std::chrono::microseconds>(modernTime).count();

        std::cout << "Legacy filter time: " << legacyMicros << " μs\n";
        std::cout << "Modern filter time: " << modernMicros << " μs\n";
        std::cout << "Performance ratio: " << static_cast<double>(modernMicros) / legacyMicros << "x\n";
    }
};

// Step 5: Advanced concept-constrained filter with multiple algorithms
template<AudioSampleType SampleType>
class AdvancedConceptFilter {
public:
    enum class FilterMode { LowPass, HighPass, BandPass, Notch };

    AdvancedConceptFilter(FilterMode mode = FilterMode::LowPass) : mode_(mode) {}

    void setParameters(SampleType frequency, SampleType q, SampleType sampleRate)
        requires FloatingPointSample<SampleType> {
        frequency_ = frequency;
        q_ = q;
        sampleRate_ = sampleRate;
        calculateCoefficients();
    }

    SampleType process(SampleType input) requires AudioSampleType<SampleType> {
        if constexpr (std::is_floating_point_v<SampleType>) {
            return processBiquad(input);
        } else {
            // Integer processing with overflow protection
            return processIntegerSafe(input);
        }
    }

    void setMode(FilterMode mode) {
        mode_ = mode;
        calculateCoefficients();
    }

    void reset() {
        x1_ = x2_ = y1_ = y2_ = SampleType(0);
    }

    // Concept compliance
    void processBlock(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = static_cast<float>(process(static_cast<SampleType>(buffer[i])));
        }
    }

    int getLatency() const { return 0; }

private:
    FilterMode mode_;
    SampleType frequency_{SampleType(1000)};
    SampleType q_{SampleType(0.707)};
    SampleType sampleRate_{SampleType(44100)};

    // Biquad coefficients
    SampleType b0_, b1_, b2_, a1_, a2_;

    // State variables
    SampleType x1_{0}, x2_{0}, y1_{0}, y2_{0};

    void calculateCoefficients() {
        if constexpr (std::is_floating_point_v<SampleType>) {
            const SampleType omega = SampleType(2) * SampleType(M_PI) * frequency_ / sampleRate_;
            const SampleType sin_omega = std::sin(omega);
            const SampleType cos_omega = std::cos(omega);
            const SampleType alpha = sin_omega / (SampleType(2) * q_);

            switch (mode_) {
                case FilterMode::LowPass:
                    b0_ = (SampleType(1) - cos_omega) / SampleType(2);
                    b1_ = SampleType(1) - cos_omega;
                    b2_ = (SampleType(1) - cos_omega) / SampleType(2);
                    break;
                case FilterMode::HighPass:
                    b0_ = (SampleType(1) + cos_omega) / SampleType(2);
                    b1_ = -(SampleType(1) + cos_omega);
                    b2_ = (SampleType(1) + cos_omega) / SampleType(2);
                    break;
                case FilterMode::BandPass:
                    b0_ = alpha;
                    b1_ = SampleType(0);
                    b2_ = -alpha;
                    break;
                case FilterMode::Notch:
                    b0_ = SampleType(1);
                    b1_ = -SampleType(2) * cos_omega;
                    b2_ = SampleType(1);
                    break;
            }

            SampleType a0 = SampleType(1) + alpha;
            a1_ = -SampleType(2) * cos_omega;
            a2_ = SampleType(1) - alpha;

            // Normalize
            b0_ /= a0;
            b1_ /= a0;
            b2_ /= a0;
            a1_ /= a0;
            a2_ /= a0;
        } else {
            // Simplified coefficients for integer types
            b0_ = b2_ = SampleType(1);
            b1_ = SampleType(2);
            a1_ = SampleType(0);
            a2_ = SampleType(0);
        }
    }

    SampleType processBiquad(SampleType input) {
        SampleType output = b0_ * input + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

        // Update state
        x2_ = x1_;
        x1_ = input;
        y2_ = y1_;
        y1_ = output;

        return output;
    }

    SampleType processIntegerSafe(SampleType input) {
        // Integer processing with overflow protection
        using WideType = std::conditional_t<sizeof(SampleType) <= 2, int32_t, int64_t>;

        WideType output = static_cast<WideType>(b0_) * input +
                         static_cast<WideType>(b1_) * x1_ +
                         static_cast<WideType>(b2_) * x2_ -
                         static_cast<WideType>(a1_) * y1_ -
                         static_cast<WideType>(a2_) * y2_;

        // Clamp to valid range
        output = std::clamp(output,
                           static_cast<WideType>(std::numeric_limits<SampleType>::min()),
                           static_cast<WideType>(std::numeric_limits<SampleType>::max()));

        SampleType result = static_cast<SampleType>(output);

        // Update state
        x2_ = x1_;
        x1_ = input;
        y2_ = y1_;
        y1_ = result;

        return result;
    }
};

} // namespace ModernAudio::Exercises

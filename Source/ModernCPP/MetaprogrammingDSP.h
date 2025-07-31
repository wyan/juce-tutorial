#pragma once
#include <type_traits>
#include <array>
#include <vector>
#include <cmath>
#include <tuple>
#include <algorithm>

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

    void reset() {
        x_.fill(SampleType(0));
        y_.fill(SampleType(0));
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

    template<size_t Index>
    auto& getProcessor() {
        return std::get<Index>(processors_);
    }

    void reset() {
        resetImpl<0>();
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

    template<std::size_t Index>
    void resetImpl() {
        if constexpr (Index < sizeof...(Processors)) {
            std::get<Index>(processors_).reset();
            resetImpl<Index + 1>();
        }
    }
};

// Compile-time algorithm selection based on sample type
template<typename SampleType>
struct OptimalAlgorithm {
    static constexpr bool useDoubleprecision = std::is_same_v<SampleType, double>;
    static constexpr bool useFloatPrecision = std::is_same_v<SampleType, float>;
    static constexpr bool useIntegerMath = std::is_integral_v<SampleType>;

    // Select optimal sine approximation based on type
    static SampleType fastSin(SampleType x) {
        if constexpr (useDoubleprecision) {
            return std::sin(x); // Use full precision for double
        } else if constexpr (useFloatPrecision) {
            // Fast sine approximation for float
            return fastSinApprox(x);
        } else {
            // Integer sine using lookup table
            return integerSin(x);
        }
    }

private:
    static SampleType fastSinApprox(SampleType x) {
        // Bhaskara I's sine approximation
        constexpr SampleType pi = SampleType(M_PI);
        x = std::fmod(x + pi, 2 * pi) - pi; // Normalize to [-π, π]
        return (16 * x * (pi - std::abs(x))) / (5 * pi * pi - 4 * std::abs(x) * (pi - std::abs(x)));
    }

    static SampleType integerSin(SampleType x) {
        // Simple integer sine using lookup table
        static constexpr std::array<int, 256> sineTable = generateIntSineTable();
        int index = static_cast<int>(x * 256 / (2 * M_PI)) & 255;
        return static_cast<SampleType>(sineTable[index]) / 32767;
    }

    static constexpr std::array<int, 256> generateIntSineTable() {
        std::array<int, 256> table{};
        for (size_t i = 0; i < 256; ++i) {
            table[i] = static_cast<int>(32767 * std::sin(2.0 * M_PI * i / 256));
        }
        return table;
    }
};

// Template specialization for different buffer types
template<typename BufferType>
struct BufferTraits;

template<typename T>
struct BufferTraits<std::vector<T>> {
    using SampleType = T;
    static constexpr bool isContiguous = true;
    static constexpr bool isDynamic = true;

    static size_t size(const std::vector<T>& buffer) {
        return buffer.size();
    }

    static T* data(std::vector<T>& buffer) {
        return buffer.data();
    }

    static const T* data(const std::vector<T>& buffer) {
        return buffer.data();
    }
};

template<typename T, size_t N>
struct BufferTraits<std::array<T, N>> {
    using SampleType = T;
    static constexpr bool isContiguous = true;
    static constexpr bool isDynamic = false;
    static constexpr size_t staticSize = N;

    static constexpr size_t size(const std::array<T, N>&) {
        return N;
    }

    static T* data(std::array<T, N>& buffer) {
        return buffer.data();
    }

    static const T* data(const std::array<T, N>& buffer) {
        return buffer.data();
    }
};

// Generic buffer processor using traits
template<typename BufferType>
class MetaBufferProcessor {
public:
    using Traits = BufferTraits<BufferType>;
    using SampleType = typename Traits::SampleType;

    template<typename ProcessorType>
    void processBuffer(BufferType& buffer, ProcessorType& processor) {
        auto* data = Traits::data(buffer);
        const size_t size = Traits::size(buffer);

        if constexpr (Traits::isContiguous) {
            // Optimized path for contiguous memory
            for (size_t i = 0; i < size; ++i) {
                data[i] = processor.process(data[i]);
            }
        } else {
            // Fallback for non-contiguous buffers
            for (size_t i = 0; i < size; ++i) {
                buffer[i] = processor.process(buffer[i]);
            }
        }
    }

    void applyGain(BufferType& buffer, SampleType gain) {
        auto* data = Traits::data(buffer);
        const size_t size = Traits::size(buffer);

        // Vectorizable loop for gain application
        for (size_t i = 0; i < size; ++i) {
            data[i] *= gain;
        }
    }
};

// Compile-time DSP chain builder
template<FilterType... Filters>
using FilterChain = ProcessingPipeline<MetaFilter<Filters, float>...>;

// Type aliases for common filter chains
using HighLowChain = FilterChain<FilterType::HighPass, FilterType::LowPass>;
using BandPassChain = FilterChain<FilterType::HighPass, FilterType::LowPass>;
using NotchChain = FilterChain<FilterType::Notch>;

// Compile-time validation
template<typename T>
constexpr bool isValidSampleType() {
    return std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;
}

template<FilterType Type>
constexpr bool isValidFilterType() {
    return Type == FilterType::LowPass ||
           Type == FilterType::HighPass ||
           Type == FilterType::BandPass ||
           Type == FilterType::Notch;
}

} // namespace ModernAudio

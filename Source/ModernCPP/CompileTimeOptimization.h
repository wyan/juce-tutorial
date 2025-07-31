#pragma once
#include <array>
#include <cmath>
#include <numbers>
#include <type_traits>

namespace ModernAudio {

// Compile-time audio format detection
enum class AudioFormat { PCM16, PCM24, PCM32, Float32, Float64 };

template<typename T>
consteval AudioFormat detectAudioFormat() {
    if constexpr (std::is_same_v<T, int16_t>) {
        return AudioFormat::PCM16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return AudioFormat::PCM32;
    } else if constexpr (std::is_same_v<T, float>) {
        return AudioFormat::Float32;
    } else if constexpr (std::is_same_v<T, double>) {
        return AudioFormat::Float64;
    } else {
        static_assert(std::is_arithmetic_v<T>, "Unsupported audio format");
        return AudioFormat::Float32; // Default fallback
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
        // Simplified Butterworth coefficient calculation for demonstration
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
        } else {
            // Higher order coefficients would be calculated here
            // For now, initialize to safe values
            b.fill(0.0);
            a.fill(0.0);
            b[0] = 1.0;
            a[0] = 1.0;
        }
    }
};

// Template-based loop unrolling for DSP
template<size_t N>
constexpr float dotProduct(const std::array<float, N>& a, const std::array<float, N>& b) {
    float result = 0.0f;

    // Unroll loop at compile time using fold expression
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
    // This would be specialized for different plugin types
    return {"Generic Plugin", "1.0.0", "ModernAudio", 0, false};
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

    constexpr float getSampleRate() const {
        return 44100.0f; // Compile-time constant
    }

    static constexpr size_t getBufferSize() {
        return BufferSize;
    }
};

// Compile-time mathematical constants for audio
namespace Constants {
    constexpr double PI = std::numbers::pi;
    constexpr double TWO_PI = 2.0 * std::numbers::pi;
    constexpr double HALF_PI = std::numbers::pi / 2.0;
    constexpr double INV_PI = std::numbers::inv_pi;

    // Common sample rates
    constexpr double SAMPLE_RATE_44K = 44100.0;
    constexpr double SAMPLE_RATE_48K = 48000.0;
    constexpr double SAMPLE_RATE_96K = 96000.0;

    // Audio conversion constants
    constexpr double DB_TO_LINEAR_FACTOR = std::numbers::ln10 / 20.0;
    constexpr double LINEAR_TO_DB_FACTOR = 20.0 / std::numbers::ln10;

    // MIDI constants
    constexpr double MIDI_A4_NOTE = 69.0;
    constexpr double MIDI_A4_FREQ = 440.0;
    constexpr double MIDI_SEMITONE_RATIO = 1.0594630943592953; // 2^(1/12)
}

// Compile-time MIDI note to frequency conversion
constexpr double midiNoteToFrequency(int midiNote) {
    return Constants::MIDI_A4_FREQ *
           std::pow(Constants::MIDI_SEMITONE_RATIO, midiNote - Constants::MIDI_A4_NOTE);
}

// Compile-time decibel conversions
constexpr double dbToLinear(double db) {
    return std::exp(db * Constants::DB_TO_LINEAR_FACTOR);
}

constexpr double linearToDb(double linear) {
    return std::log(linear) * Constants::LINEAR_TO_DB_FACTOR;
}

// Compile-time sample format conversions
template<typename FromType, typename ToType>
constexpr ToType convertSample(FromType sample) {
    if constexpr (std::is_same_v<FromType, ToType>) {
        return sample;
    } else if constexpr (std::is_floating_point_v<FromType> && std::is_floating_point_v<ToType>) {
        return static_cast<ToType>(sample);
    } else if constexpr (std::is_floating_point_v<FromType> && std::is_integral_v<ToType>) {
        constexpr ToType maxVal = std::numeric_limits<ToType>::max();
        return static_cast<ToType>(sample * maxVal);
    } else if constexpr (std::is_integral_v<FromType> && std::is_floating_point_v<ToType>) {
        constexpr FromType maxVal = std::numeric_limits<FromType>::max();
        return static_cast<ToType>(sample) / static_cast<ToType>(maxVal);
    } else {
        // Integer to integer conversion
        constexpr int fromBits = sizeof(FromType) * 8;
        constexpr int toBits = sizeof(ToType) * 8;

        if constexpr (fromBits > toBits) {
            return static_cast<ToType>(sample >> (fromBits - toBits));
        } else {
            return static_cast<ToType>(sample << (toBits - fromBits));
        }
    }
}

// Compile-time buffer size validation
template<size_t BufferSize>
constexpr bool isValidBufferSize() {
    return BufferSize > 0 && BufferSize <= 8192 && (BufferSize & (BufferSize - 1)) == 0;
}

// Compile-time frequency validation
constexpr bool isValidFrequency(double frequency, double sampleRate) {
    return frequency > 0.0 && frequency < sampleRate / 2.0;
}

// Compile-time filter design
template<size_t Order, typename CoeffType = double>
struct CompileTimeFilter {
    std::array<CoeffType, Order + 1> b;
    std::array<CoeffType, Order + 1> a;

    constexpr CompileTimeFilter(CoeffType cutoff, CoeffType sampleRate) {
        // Simple first-order filter design
        if constexpr (Order == 1) {
            CoeffType omega = 2.0 * std::numbers::pi * cutoff / sampleRate;
            CoeffType alpha = std::sin(omega) / 2.0;

            b[0] = (1.0 - std::cos(omega)) / 2.0;
            b[1] = 1.0 - std::cos(omega);
            a[0] = 1.0 + alpha;
            a[1] = -2.0 * std::cos(omega);
        } else {
            // Initialize to pass-through for higher orders
            b.fill(0.0);
            a.fill(0.0);
            b[0] = 1.0;
            a[0] = 1.0;
        }
    }

    constexpr CoeffType process(CoeffType input, std::array<CoeffType, Order>& xHistory,
                               std::array<CoeffType, Order>& yHistory) const {
        CoeffType output = b[0] * input;

        // Add feedforward terms
        for (size_t i = 0; i < Order && i < b.size() - 1; ++i) {
            output += b[i + 1] * xHistory[i];
        }

        // Add feedback terms
        for (size_t i = 0; i < Order && i < a.size() - 1; ++i) {
            output -= a[i + 1] * yHistory[i];
        }

        output /= a[0];

        // Update history
        for (size_t i = Order - 1; i > 0; --i) {
            xHistory[i] = xHistory[i - 1];
            yHistory[i] = yHistory[i - 1];
        }
        if (Order > 0) {
            xHistory[0] = input;
            yHistory[0] = output;
        }

        return output;
    }
};

// Compile-time oscillator
template<size_t TableSize = 1024>
class CompileTimeOscillator {
public:
    constexpr CompileTimeOscillator(double frequency, double sampleRate)
        : phaseIncrement_(frequency * TableSize / sampleRate) {}

    constexpr float getNextSample() {
        static constexpr auto sineTable = generateSineTable<TableSize>();

        size_t index = static_cast<size_t>(phase_) % TableSize;
        phase_ += phaseIncrement_;

        return sineTable[index];
    }

    constexpr void setFrequency(double frequency, double sampleRate) {
        phaseIncrement_ = frequency * TableSize / sampleRate;
    }

private:
    double phase_ = 0.0;
    double phaseIncrement_;
};

// Compile-time validation helpers
template<typename T>
constexpr bool isAudioSampleType() {
    return std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;
}

template<size_t N>
constexpr bool isPowerOfTwo() {
    return N > 0 && (N & (N - 1)) == 0;
}

// Compile-time string hashing for parameter IDs
constexpr uint32_t hash(const char* str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<uint32_t>(*str++);
    }
    return hash;
}

// Macro for compile-time parameter ID generation
#define PARAM_ID(name) (ModernAudio::hash(name))

} // namespace ModernAudio

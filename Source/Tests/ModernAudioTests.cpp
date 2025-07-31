#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../ModernCPP/MetaprogrammingDSP.h"
#include "../ModernCPP/FunctionalAudio.h"
#include "../ModernCPP/ModernMemoryManagement.h"
#include "../ModernCPP/CompileTimeOptimization.h"
#include <chrono>
#include <random>

using namespace ModernAudio;

// Test fixtures for common setup
class AudioTestFixture {
protected:
    static constexpr size_t bufferSize = 1024;
    static constexpr float sampleRate = 44100.0f;
    std::vector<float> testBuffer;

    AudioTestFixture() {
        // Generate test signal
        testBuffer.resize(bufferSize);
        for (size_t i = 0; i < bufferSize; ++i) {
            testBuffer[i] = std::sin(2.0f * M_PI * 440.0f * i / sampleRate);
        }
    }

    std::vector<float> generateNoise(size_t size, float amplitude = 1.0f) {
        std::vector<float> noise(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-amplitude, amplitude);

        for (auto& sample : noise) {
            sample = dist(gen);
        }

        return noise;
    }

    float calculateRMS(const std::vector<float>& buffer) {
        float sum = 0.0f;
        for (float sample : buffer) {
            sum += sample * sample;
        }
        return std::sqrt(sum / buffer.size());
    }

    float calculatePeak(const std::vector<float>& buffer) {
        float peak = 0.0f;
        for (float sample : buffer) {
            peak = std::max(peak, std::abs(sample));
        }
        return peak;
    }
};

// Template metaprogramming tests
TEST_CASE_METHOD(AudioTestFixture, "MetaFilter Processing", "[metaprogramming]") {
    SECTION("LowPass filter processes correctly") {
        MetaFilter<FilterType::LowPass, float> filter;
        filter.setParameters(1000.0f, 0.707f, sampleRate);

        auto originalBuffer = testBuffer;
        for (auto& sample : testBuffer) {
            sample = filter.process(sample);
        }

        // Low pass should reduce high frequency content
        float originalRMS = calculateRMS(originalBuffer);
        float filteredRMS = calculateRMS(testBuffer);

        REQUIRE(filteredRMS > 0.0f);
        REQUIRE(filteredRMS <= originalRMS);
    }

    SECTION("HighPass filter processes correctly") {
        MetaFilter<FilterType::HighPass, float> filter;
        filter.setParameters(100.0f, 0.707f, sampleRate);

        // Test with DC signal
        std::vector<float> dcBuffer(bufferSize, 1.0f);
        for (auto& sample : dcBuffer) {
            sample = filter.process(sample);
        }

        // High pass should remove DC component
        float finalRMS = calculateRMS(dcBuffer);
        REQUIRE(finalRMS < 0.1f); // Should be close to zero
    }

    SECTION("Processing pipeline works correctly") {
        using TestPipeline = ProcessingPipeline<
            MetaFilter<FilterType::HighPass, float>,
            MetaFilter<FilterType::LowPass, float>
        >;

        TestPipeline pipeline;
        pipeline.getProcessor<0>().setParameters(100.0f, 0.707f, sampleRate);
        pipeline.getProcessor<1>().setParameters(5000.0f, 0.707f, sampleRate);

        float input = 1.0f;
        float output = pipeline.process(input);

        REQUIRE(std::isfinite(output));
    }
}

// Functional programming tests
TEST_CASE_METHOD(AudioTestFixture, "Functional Audio Processing", "[functional]") {
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

        // Buffer should be different
        REQUIRE(initialState.buffer != processedState.buffer);
    }

    SECTION("AudioResult monadic operations") {
        auto successResult = AudioResult<float>::success(42.0f);
        auto errorResult = AudioResult<float>::error("Test error");

        REQUIRE(successResult.isSuccess());
        REQUIRE(errorResult.isError());

        auto mappedResult = successResult.map([](float x) { return x * 2.0f; });
        REQUIRE(mappedResult.isSuccess());
        REQUIRE(mappedResult.getValue() == 84.0f);

        auto mappedError = errorResult.map([](float x) { return x * 2.0f; });
        REQUIRE(mappedError.isError());
    }

    SECTION("Parameter mapping functions") {
        auto linearMap = ParameterMapper::linear(0.0f, 100.0f);
        auto expMap = ParameterMapper::exponential(1.0f, 1000.0f, 2.0f);
        auto logMap = ParameterMapper::logarithmic(1.0f, 1000.0f);

        REQUIRE(linearMap(0.0f) == Approx(0.0f));
        REQUIRE(linearMap(1.0f) == Approx(100.0f));
        REQUIRE(linearMap(0.5f) == Approx(50.0f));

        REQUIRE(expMap(0.0f) == Approx(1.0f));
        REQUIRE(expMap(1.0f) == Approx(1000.0f));

        REQUIRE(logMap(0.0f) == Approx(1.0f));
        REQUIRE(logMap(1.0f) == Approx(1000.0f));
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
            REQUIRE(manager.getBufferSize() == 1024);
            REQUIRE(manager.getNumChannels() == 2);
        }
        // Buffers should be automatically cleaned up
    }

    SECTION("Lock-free circular buffer") {
        LockFreeCircularBuffer<float, 16> buffer;

        REQUIRE(buffer.empty());
        REQUIRE_FALSE(buffer.full());
        REQUIRE(buffer.size() == 0);

        // Fill buffer
        for (int i = 0; i < 15; ++i) {
            REQUIRE(buffer.push(static_cast<float>(i)));
        }

        REQUIRE(buffer.full());
        REQUIRE(buffer.size() == 15);

        // Empty buffer
        float value;
        for (int i = 0; i < 15; ++i) {
            REQUIRE(buffer.pop(value));
            REQUIRE(value == static_cast<float>(i));
        }

        REQUIRE(buffer.empty());
        REQUIRE(buffer.size() == 0);
    }

    SECTION("Memory pool allocation") {
        MemoryPool pool(1024);

        void* ptr1 = pool.allocate(100);
        void* ptr2 = pool.allocate(200);

        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(pool.getBytesUsed() >= 300);
        REQUIRE(pool.getBytesAvailable() <= 724);

        pool.reset();
        REQUIRE(pool.getBytesUsed() == 0);
    }

    SECTION("Audio device RAII") {
        {
            AudioDeviceRAII device(1);
            REQUIRE(device.isOpen());
            REQUIRE(device.getDeviceId() == 1);
        }
        // Device should be automatically closed

        // Test move semantics
        AudioDeviceRAII device1(2);
        AudioDeviceRAII device2 = std::move(device1);
        REQUIRE(device2.isOpen());
        REQUIRE_FALSE(device1.isOpen());
    }
}

// Compile-time optimization tests
TEST_CASE("Compile-Time Optimization", "[compile-time]") {
    SECTION("Audio format detection") {
        constexpr auto floatFormat = detectAudioFormat<float>();
        constexpr auto doubleFormat = detectAudioFormat<double>();
        constexpr auto int16Format = detectAudioFormat<int16_t>();
        constexpr auto int32Format = detectAudioFormat<int32_t>();

        REQUIRE(floatFormat == AudioFormat::Float32);
        REQUIRE(doubleFormat == AudioFormat::Float64);
        REQUIRE(int16Format == AudioFormat::PCM16);
        REQUIRE(int32Format == AudioFormat::PCM32);
    }

    SECTION("Compile-time sine table generation") {
        constexpr auto sineTable = generateSineTable<256>();

        REQUIRE(sineTable[0] == Approx(0.0f).margin(1e-6));
        REQUIRE(sineTable[64] == Approx(1.0f).margin(1e-6));  // π/2
        REQUIRE(sineTable[128] == Approx(0.0f).margin(1e-6)); // π
        REQUIRE(sineTable[192] == Approx(-1.0f).margin(1e-6)); // 3π/2
    }

    SECTION("Compile-time window generation") {
        constexpr auto hannWindow = generateWindow<WindowType::Hann, 512>();
        constexpr auto hammingWindow = generateWindow<WindowType::Hamming, 512>();

        // Hann window should be 0 at endpoints
        REQUIRE(hannWindow[0] == Approx(0.0f).margin(1e-6));
        REQUIRE(hannWindow[511] == Approx(0.0f).margin(1e-6));

        // Hamming window should be non-zero at endpoints
        REQUIRE(hammingWindow[0] > 0.0f);
        REQUIRE(hammingWindow[511] > 0.0f);
    }

    SECTION("MIDI note to frequency conversion") {
        constexpr double a4Freq = midiNoteToFrequency(69);
        constexpr double c4Freq = midiNoteToFrequency(60);

        REQUIRE(a4Freq == Approx(440.0).margin(1e-6));
        REQUIRE(c4Freq == Approx(261.626).margin(0.001));
    }

    SECTION("Decibel conversions") {
        constexpr double linear1 = dbToLinear(0.0);
        constexpr double linear2 = dbToLinear(20.0);
        constexpr double db1 = linearToDb(1.0);
        constexpr double db2 = linearToDb(10.0);

        REQUIRE(linear1 == Approx(1.0).margin(1e-6));
        REQUIRE(linear2 == Approx(10.0).margin(1e-3));
        REQUIRE(db1 == Approx(0.0).margin(1e-6));
        REQUIRE(db2 == Approx(20.0).margin(1e-3));
    }

    SECTION("Sample format conversions") {
        constexpr float floatSample = 0.5f;
        constexpr int16_t intSample = convertSample<float, int16_t>(floatSample);
        constexpr float backToFloat = convertSample<int16_t, float>(intSample);

        REQUIRE(intSample > 0);
        REQUIRE(backToFloat == Approx(floatSample).margin(1e-4));
    }

    SECTION("Compile-time validation") {
        static_assert(isValidBufferSize<1024>());
        static_assert(!isValidBufferSize<1023>());
        static_assert(isValidFrequency(1000.0, 44100.0));
        static_assert(!isValidFrequency(25000.0, 44100.0));
        static_assert(isPowerOfTwo<1024>());
        static_assert(!isPowerOfTwo<1023>());
    }
}

// Performance benchmarks
TEST_CASE_METHOD(AudioTestFixture, "Performance Benchmarks", "[benchmark]") {
    SECTION("Filter processing benchmark") {
        MetaFilter<FilterType::LowPass, float> filter;
        filter.setParameters(1000.0f, 0.707f, sampleRate);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; ++i) {
            for (auto& sample : testBuffer) {
                sample = filter.process(sample);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Benchmark assertion - should process 1M samples in reasonable time
        REQUIRE(duration.count() < 100000); // Less than 100ms

        // Log performance for reference
        INFO("Filter processing: " << duration.count() << " microseconds for 1M samples");
    }

    SECTION("Effect chain benchmark") {
        EffectChain chain;
        chain.addEffect(Effects::gain(1.5f))
             .addEffect(Effects::softClip(0.8f))
             .addEffect(Effects::highpass(100.0f, sampleRate));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; ++i) {
            chain.processBuffer(testBuffer);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        INFO("Effect chain processing: " << duration.count() << " microseconds for 100 iterations");
        REQUIRE(duration.count() < 50000); // Less than 50ms
    }

    SECTION("Memory allocation benchmark") {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<AudioBufferPtr> buffers;
        for (int i = 0; i < 1000; ++i) {
            buffers.push_back(makeAudioBuffer(1024, 2));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        INFO("Buffer allocation: " << duration.count() << " microseconds for 1000 buffers");
        REQUIRE(duration.count() < 10000); // Less than 10ms
    }
}

// Property-based testing for audio algorithms
TEST_CASE_METHOD(AudioTestFixture, "Audio Processing Properties", "[properties]") {
    SECTION("Gain processing preserves signal shape") {
        const float gain = 2.0f;
        auto originalBuffer = testBuffer;

        // Apply gain
        std::transform(testBuffer.begin(), testBuffer.end(), testBuffer.begin(),
            [gain](float sample) { return sample * gain; });

        // Property: relative amplitudes should be preserved
        for (size_t i = 1; i < originalBuffer.size(); ++i) {
            if (originalBuffer[0] != 0.0f) {
                float originalRatio = originalBuffer[i] / originalBuffer[0];
                float gainedRatio = testBuffer[i] / testBuffer[0];
                REQUIRE(originalRatio == Approx(gainedRatio).margin(1e-6));
            }
        }
    }

    SECTION("Filter stability property") {
        MetaFilter<FilterType::LowPass, float> filter;
        filter.setParameters(1000.0f, 0.707f, sampleRate);

        // Feed impulse and check for stability
        std::vector<float> impulseResponse;
        impulseResponse.push_back(filter.process(1.0f)); // Impulse

        for (int i = 0; i < 1000; ++i) {
            impulseResponse.push_back(filter.process(0.0f));
        }

        // Property: stable filter should decay to zero
        float finalValue = std::abs(impulseResponse.back());
        REQUIRE(finalValue < 0.01f);

        // Property: no sample should exceed reasonable bounds
        for (float sample : impulseResponse) {
            REQUIRE(std::abs(sample) < 10.0f);
        }
    }

    SECTION("Normalization preserves zero crossings") {
        // Create signal with known zero crossings
        std::vector<float> signal;
        for (size_t i = 0; i < 100; ++i) {
            signal.push_back(std::sin(2.0f * M_PI * i / 20.0f));
        }

        // Count zero crossings before normalization
        int zeroCrossingsBefore = 0;
        for (size_t i = 1; i < signal.size(); ++i) {
            if ((signal[i-1] >= 0) != (signal[i] >= 0)) {
                zeroCrossingsBefore++;
            }
        }

        // Normalize
        float maxVal = *std::max_element(signal.begin(), signal.end(),
            [](float a, float b) { return std::abs(a) < std::abs(b); });
        if (maxVal != 0.0f) {
            float scale = 1.0f / std::abs(maxVal);
            std::transform(signal.begin(), signal.end(), signal.begin(),
                [scale](float sample) { return sample * scale; });
        }

        // Count zero crossings after normalization
        int zeroCrossingsAfter = 0;
        for (size_t i = 1; i < signal.size(); ++i) {
            if ((signal[i-1] >= 0) != (signal[i] >= 0)) {
                zeroCrossingsAfter++;
            }
        }

        // Property: normalization should preserve zero crossings
        REQUIRE(zeroCrossingsBefore == zeroCrossingsAfter);
    }
}

// Integration tests
TEST_CASE_METHOD(AudioTestFixture, "Integration Tests", "[integration]") {
    SECTION("Complete audio processing chain") {
        // Create a complete processing chain
        AudioBufferManager bufferManager(bufferSize, 2);

        // Fill buffers with test data
        auto* leftChannel = bufferManager.getChannelData(0);
        auto* rightChannel = bufferManager.getChannelData(1);

        std::copy(testBuffer.begin(), testBuffer.end(), leftChannel);
        std::copy(testBuffer.begin(), testBuffer.end(), rightChannel);

        // Create processing chain
        EffectChain chain;
        chain.addEffect(Effects::highpass(80.0f, sampleRate))
             .addEffect(Effects::gain(1.2f))
             .addEffect(Effects::softClip(0.9f))
             .addEffect(Effects::lowpass(8000.0f, sampleRate));

        // Process both channels
        std::vector<float> leftBuffer(leftChannel, leftChannel + bufferSize);
        std::vector<float> rightBuffer(rightChannel, rightChannel + bufferSize);

        chain.processBuffer(leftBuffer);
        chain.processBuffer(rightBuffer);

        // Verify processing
        REQUIRE(calculateRMS(leftBuffer) > 0.0f);
        REQUIRE(calculateRMS(rightBuffer) > 0.0f);
        REQUIRE(calculatePeak(leftBuffer) <= 1.0f);
        REQUIRE(calculatePeak(rightBuffer) <= 1.0f);
    }
}

# Module 6: Advanced Audio Features

## Learning Objectives

By the end of this module, you will:
- Implement advanced DSP algorithms (EQ, compression, delay effects)
- Create real-time audio visualization (spectrum analyzer, waveform display)
- Learn MIDI integration and processing
- Implement advanced parameter modulation and automation
- Master performance optimization for real-time audio
- Advanced C++ techniques (CRTP, template specialization, constexpr)

## Table of Contents

1. [Advanced DSP Concepts](#advanced-dsp-concepts)
2. [Multi-Band EQ Implementation](#multi-band-eq-implementation)
3. [Dynamic Range Compression](#dynamic-range-compression)
4. [Delay Effects and Modulation](#delay-effects-and-modulation)
5. [Real-Time Audio Visualization](#real-time-audio-visualization)
6. [MIDI Integration and Processing](#midi-integration-and-processing)
7. [Advanced Parameter Modulation](#advanced-parameter-modulation)
8. [Performance Optimization](#performance-optimization)
9. [Professional Audio Features](#professional-audio-features)
10. [Advanced C++ Techniques](#advanced-cpp-techniques)
11. [Practical Implementation](#practical-implementation)
12. [Practical Exercises](#practical-exercises)
13. [Code Examples and Best Practices](#code-examples-and-best-practices)

---

## Advanced DSP Concepts

### Understanding Digital Signal Processing

Digital Signal Processing (DSP) is the mathematical manipulation of audio signals in the digital domain. Advanced DSP techniques allow us to create sophisticated audio effects and analysis tools.

#### Core DSP Principles

```cpp
namespace AdvancedDSP
{
    // Fundamental DSP concepts
    template<typename SampleType>
    class DSPProcessor
    {
    public:
        static_assert(std::is_floating_point_v<SampleType>, "Sample type must be floating point");

        virtual ~DSPProcessor() = default;
        virtual void prepare(double sampleRate, int maxBlockSize) = 0;
        virtual void process(SampleType* samples, int numSamples) = 0;
        virtual void reset() = 0;

    protected:
        double currentSampleRate = 44100.0;
        int maxBufferSize = 512;
    };

    // Biquad filter - foundation for many DSP effects
    template<typename SampleType>
    class BiquadFilter : public DSPProcessor<SampleType>
    {
    public:
        enum FilterType
        {
            LowPass,
            HighPass,
            BandPass,
            Notch,
            AllPass,
            LowShelf,
            HighShelf,
            Peak
        };

        void setCoefficients(SampleType b0, SampleType b1, SampleType b2,
                           SampleType a1, SampleType a2)
        {
            coeffs.b0 = b0;
            coeffs.b1 = b1;
            coeffs.b2 = b2;
            coeffs.a1 = a1;
            coeffs.a2 = a2;
        }

        void setFilter(FilterType type, SampleType frequency, SampleType Q, SampleType gain = 1.0)
        {
            auto omega = juce::MathConstants<SampleType>::twoPi * frequency / this->currentSampleRate;
            auto sin_omega = std::sin(omega);
            auto cos_omega = std::cos(omega);
            auto alpha = sin_omega / (2.0 * Q);
            auto A = std::sqrt(gain);

            switch (type)
            {
                case LowPass:
                    setLowPassCoefficients(cos_omega, alpha);
                    break;
                case HighPass:
                    setHighPassCoefficients(cos_omega, alpha);
                    break;
                case BandPass:
                    setBandPassCoefficients(sin_omega, alpha);
                    break;
                case Peak:
                    setPeakCoefficients(cos_omega, alpha, A);
                    break;
                // ... other filter types
            }
        }

        void prepare(double sampleRate, int maxBlockSize) override
        {
            this->currentSampleRate = sampleRate;
            this->maxBufferSize = maxBlockSize;
            reset();
        }

        void process(SampleType* samples, int numSamples) override
        {
            for (int i = 0; i < numSamples; ++i)
            {
                samples[i] = processSample(samples[i]);
            }
        }

        SampleType processSample(SampleType input)
        {
            auto output = coeffs.b0 * input + coeffs.b1 * state.x1 + coeffs.b2 * state.x2
                         - coeffs.a1 * state.y1 - coeffs.a2 * state.y2;

            // Update state
            state.x2 = state.x1;
            state.x1 = input;
            state.y2 = state.y1;
            state.y1 = output;

            return output;
        }

        void reset() override
        {
            state = {};
        }

    private:
        struct Coefficients
        {
            SampleType b0 = 1, b1 = 0, b2 = 0;
            SampleType a1 = 0, a2 = 0;
        } coeffs;

        struct State
        {
            SampleType x1 = 0, x2 = 0;
            SampleType y1 = 0, y2 = 0;
        } state;

        void setLowPassCoefficients(SampleType cos_omega, SampleType alpha)
        {
            auto norm = 1.0 / (1.0 + alpha);
            coeffs.b0 = (1.0 - cos_omega) * 0.5 * norm;
            coeffs.b1 = (1.0 - cos_omega) * norm;
            coeffs.b2 = coeffs.b0;
            coeffs.a1 = -2.0 * cos_omega * norm;
            coeffs.a2 = (1.0 - alpha) * norm;
        }

        void setHighPassCoefficients(SampleType cos_omega, SampleType alpha)
        {
            auto norm = 1.0 / (1.0 + alpha);
            coeffs.b0 = (1.0 + cos_omega) * 0.5 * norm;
            coeffs.b1 = -(1.0 + cos_omega) * norm;
            coeffs.b2 = coeffs.b0;
            coeffs.a1 = -2.0 * cos_omega * norm;
            coeffs.a2 = (1.0 - alpha) * norm;
        }

        void setBandPassCoefficients(SampleType sin_omega, SampleType alpha)
        {
            auto norm = 1.0 / (1.0 + alpha);
            coeffs.b0 = alpha * norm;
            coeffs.b1 = 0;
            coeffs.b2 = -alpha * norm;
            coeffs.a1 = -2.0 * std::cos(sin_omega) * norm;
            coeffs.a2 = (1.0 - alpha) * norm;
        }

        void setPeakCoefficients(SampleType cos_omega, SampleType alpha, SampleType A)
        {
            auto norm = 1.0 / (1.0 + alpha / A);
            coeffs.b0 = (1.0 + alpha * A) * norm;
            coeffs.b1 = -2.0 * cos_omega * norm;
            coeffs.b2 = (1.0 - alpha * A) * norm;
            coeffs.a1 = coeffs.b1;
            coeffs.a2 = (1.0 - alpha / A) * norm;
        }
    };
}
```

### Audio Analysis Tools

```cpp
namespace AudioAnalysis
{
    template<typename SampleType>
    class RMSAnalyzer
    {
    public:
        RMSAnalyzer(int windowSize = 1024) : bufferSize(windowSize)
        {
            buffer.resize(bufferSize);
            reset();
        }

        void reset()
        {
            buffer.fill(0);
            writeIndex = 0;
            sum = 0;
        }

        SampleType process(SampleType input)
        {
            // Remove old sample from sum
            sum -= buffer[writeIndex] * buffer[writeIndex];

            // Add new sample
            buffer[writeIndex] = input;
            sum += input * input;

            // Advance write index
            writeIndex = (writeIndex + 1) % bufferSize;

            // Return RMS value
            return std::sqrt(sum / bufferSize);
        }

        SampleType getCurrentRMS() const
        {
            return std::sqrt(sum / bufferSize);
        }

    private:
        std::vector<SampleType> buffer;
        int bufferSize;
        int writeIndex = 0;
        SampleType sum = 0;
    };

    template<typename SampleType>
    class PeakDetector
    {
    public:
        PeakDetector(SampleType attackTime = 0.001, SampleType releaseTime = 0.1)
            : attackCoeff(0), releaseCoeff(0)
        {
            setTimes(attackTime, releaseTime);
        }

        void prepare(double sampleRate)
        {
            this->sampleRate = sampleRate;
            updateCoefficients();
        }

        void setTimes(SampleType attack, SampleType release)
        {
            attackTime = attack;
            releaseTime = release;
            updateCoefficients();
        }

        SampleType process(SampleType input)
        {
            auto inputLevel = std::abs(input);

            if (inputLevel > currentPeak)
            {
                // Attack phase
                currentPeak = inputLevel + attackCoeff * (currentPeak - inputLevel);
            }
            else
            {
                // Release phase
                currentPeak = inputLevel + releaseCoeff * (currentPeak - inputLevel);
            }

            return currentPeak;
        }

        SampleType getCurrentPeak() const { return currentPeak; }
        void reset() { currentPeak = 0; }

    private:
        double sampleRate = 44100.0;
        SampleType attackTime, releaseTime;
        SampleType attackCoeff, releaseCoeff;
        SampleType currentPeak = 0;

        void updateCoefficients()
        {
            if (sampleRate > 0)
            {
                attackCoeff = std::exp(-1.0 / (attackTime * sampleRate));
                releaseCoeff = std::exp(-1.0 / (releaseTime * sampleRate));
            }
        }
    };
}
```

---

## Multi-Band EQ Implementation

### Professional EQ Design

A multi-band equalizer divides the frequency spectrum into multiple bands, each with independent gain control. This is essential for professional audio processing.

```cpp
class MultiBandEQ
{
public:
    static constexpr int NumBands = 5;

    struct Band
    {
        float frequency = 1000.0f;
        float gain = 0.0f;        // dB
        float Q = 0.707f;
        AdvancedDSP::BiquadFilter<float>::FilterType type = AdvancedDSP::BiquadFilter<float>::Peak;
        bool enabled = true;
    };

    MultiBandEQ()
    {
        // Initialize default bands
        bands[0] = {80.0f, 0.0f, 0.707f, AdvancedDSP::BiquadFilter<float>::HighPass, true};
        bands[1] = {250.0f, 0.0f, 1.0f, AdvancedDSP::BiquadFilter<float>::Peak, true};
        bands[2] = {1000.0f, 0.0f, 1.0f, AdvancedDSP::BiquadFilter<float>::Peak, true};
        bands[3] = {4000.0f, 0.0f, 1.0f, AdvancedDSP::BiquadFilter<float>::Peak, true};
        bands[4] = {12000.0f, 0.0f, 0.707f, AdvancedDSP::BiquadFilter<float>::LowPass, true};

        for (int i = 0; i < NumBands; ++i)
        {
            filters[i] = std::make_unique<AdvancedDSP::BiquadFilter<float>>();
        }
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;

        for (int i = 0; i < NumBands; ++i)
        {
            filters[i]->prepare(sampleRate, maxBlockSize);
            updateBand(i);
        }
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float processedSample = channelData[sample];

                // Process through each enabled band
                for (int band = 0; band < NumBands; ++band)
                {
                    if (bands[band].enabled)
                    {
                        processedSample = filters[band]->processSample(processedSample);
                    }
                }

                channelData[sample] = processedSample;
            }
        }
    }

    void setBandParameters(int bandIndex, float frequency, float gain, float Q)
    {
        if (bandIndex >= 0 && bandIndex < NumBands)
        {
            bands[bandIndex].frequency = frequency;
            bands[bandIndex].gain = gain;
            bands[bandIndex].Q = Q;
            updateBand(bandIndex);
        }
    }

    void setBandEnabled(int bandIndex, bool enabled)
    {
        if (bandIndex >= 0 && bandIndex < NumBands)
        {
            bands[bandIndex].enabled = enabled;
        }
    }

    const Band& getBand(int index) const
    {
        return bands[juce::jlimit(0, NumBands - 1, index)];
    }

    // Get frequency response for visualization
    std::vector<float> getFrequencyResponse(const std::vector<float>& frequencies) const
    {
        std::vector<float> response(frequencies.size(), 1.0f);

        for (size_t i = 0; i < frequencies.size(); ++i)
        {
            std::complex<float> totalResponse(1.0f, 0.0f);

            for (int band = 0; band < NumBands; ++band)
            {
                if (bands[band].enabled)
                {
                    auto bandResponse = calculateBandResponse(band, frequencies[i]);
                    totalResponse *= bandResponse;
                }
            }

            response[i] = std::abs(totalResponse);
        }

        return response;
    }

private:
    std::array<Band, NumBands> bands;
    std::array<std::unique_ptr<AdvancedDSP::BiquadFilter<float>>, NumBands> filters;
    double sampleRate = 44100.0;

    void updateBand(int bandIndex)
    {
        if (bandIndex >= 0 && bandIndex < NumBands && sampleRate > 0)
        {
            const auto& band = bands[bandIndex];
            auto gainLinear = juce::Decibels::decibelsToGain(band.gain);

            filters[bandIndex]->setFilter(band.type, band.frequency, band.Q, gainLinear);
        }
    }

    std::complex<float> calculateBandResponse(int bandIndex, float frequency) const
    {
        // Simplified frequency response calculation
        const auto& band = bands[bandIndex];
        auto omega = juce::MathConstants<float>::twoPi * frequency / sampleRate;
        auto gainLinear = juce::Decibels::decibelsToGain(band.gain);

        // This is a simplified calculation - real implementation would be more complex
        return std::complex<float>(gainLinear, 0.0f);
    }
};
```

### EQ Visualization Component

```cpp
class EQVisualizationComponent : public juce::Component,
                                public juce::Timer
{
public:
    EQVisualizationComponent(MultiBandEQ& eq) : equalizer(eq)
    {
        setSize(400, 200);
        startTimer(30); // 30 FPS

        // Generate frequency points for visualization
        generateFrequencyPoints();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        // Draw frequency grid
        drawFrequencyGrid(g);

        // Draw EQ curve
        drawEQCurve(g);

        // Draw band controls
        drawBandControls(g);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Find closest band to mouse position
        selectedBand = findClosestBand(e.getPosition());
        if (selectedBand >= 0)
        {
            isDragging = true;
            updateBandFromMouse(e.getPosition());
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (isDragging && selectedBand >= 0)
        {
            updateBandFromMouse(e.getPosition());
        }
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        isDragging = false;
        selectedBand = -1;
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    MultiBandEQ& equalizer;
    std::vector<float> frequencyPoints;
    std::vector<float> responsePoints;
    int selectedBand = -1;
    bool isDragging = false;

    void generateFrequencyPoints()
    {
        frequencyPoints.clear();

        // Generate logarithmic frequency points from 20Hz to 20kHz
        for (int i = 0; i < 200; ++i)
        {
            auto freq = 20.0f * std::pow(1000.0f, i / 199.0f);
            frequencyPoints.push_back(freq);
        }
    }

    void drawFrequencyGrid(juce::Graphics& g)
    {
        g.setColour(juce::Colours::darkgrey);

        auto bounds = getLocalBounds().toFloat();

        // Draw frequency lines
        std::vector<float> freqLines = {100, 1000, 10000};
        for (auto freq : freqLines)
        {
            auto x = frequencyToX(freq, bounds);
            g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
        }

        // Draw gain lines
        for (int db = -12; db <= 12; db += 6)
        {
            auto y = gainToY(db, bounds);
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
        }

        // Draw 0dB line more prominently
        g.setColour(juce::Colours::grey);
        auto zeroY = gainToY(0, bounds);
        g.drawHorizontalLine(zeroY, bounds.getX(), bounds.getRight());
    }

    void drawEQCurve(juce::Graphics& g)
    {
        responsePoints = equalizer.getFrequencyResponse(frequencyPoints);

        g.setColour(juce::Colours::cyan);

        juce::Path curvePath;
        auto bounds = getLocalBounds().toFloat();

        for (size_t i = 0; i < frequencyPoints.size(); ++i)
        {
            auto x = frequencyToX(frequencyPoints[i], bounds);
            auto gainDb = juce::Decibels::gainToDecibels(responsePoints[i]);
            auto y = gainToY(gainDb, bounds);

            if (i == 0)
                curvePath.startNewSubPath(x, y);
            else
                curvePath.lineTo(x, y);
        }

        g.strokePath(curvePath, juce::PathStrokeType(2.0f));
    }

    void drawBandControls(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        for (int i = 0; i < MultiBandEQ::NumBands; ++i)
        {
            const auto& band = equalizer.getBand(i);
            if (!band.enabled) continue;

            auto x = frequencyToX(band.frequency, bounds);
            auto y = gainToY(band.gain, bounds);

            // Draw band control point
            g.setColour(i == selectedBand ? juce::Colours::yellow : juce::Colours::white);
            g.fillEllipse(x - 4, y - 4, 8, 8);

            // Draw band number
            g.setColour(juce::Colours::white);
            g.drawText(juce::String(i + 1), x - 10, y - 20, 20, 15, juce::Justification::centred);
        }
    }

    float frequencyToX(float frequency, juce::Rectangle<float> bounds) const
    {
        auto logFreq = std::log10(frequency / 20.0f) / std::log10(1000.0f);
        return bounds.getX() + logFreq * bounds.getWidth();
    }

    float gainToY(float gainDb, juce::Rectangle<float> bounds) const
    {
        auto normalizedGain = (gainDb + 15.0f) / 30.0f; // -15dB to +15dB range
        return bounds.getBottom() - normalizedGain * bounds.getHeight();
    }

    float xToFrequency(float x, juce::Rectangle<float> bounds) const
    {
        auto normalizedX = (x - bounds.getX()) / bounds.getWidth();
        return 20.0f * std::pow(1000.0f, normalizedX);
    }

    float yToGain(float y, juce::Rectangle<float> bounds) const
    {
        auto normalizedY = (bounds.getBottom() - y) / bounds.getHeight();
        return normalizedY * 30.0f - 15.0f; // -15dB to +15dB range
    }

    int findClosestBand(juce::Point<int> mousePos)
    {
        auto bounds = getLocalBounds().toFloat();
        float minDistance = std::numeric_limits<float>::max();
        int closestBand = -1;

        for (int i = 0; i < MultiBandEQ::NumBands; ++i)
        {
            const auto& band = equalizer.getBand(i);
            if (!band.enabled) continue;

            auto x = frequencyToX(band.frequency, bounds);
            auto y = gainToY(band.gain, bounds);

            auto distance = std::sqrt(std::pow(mousePos.x - x, 2) + std::pow(mousePos.y - y, 2));

            if (distance < minDistance && distance < 20.0f) // 20 pixel threshold
            {
                minDistance = distance;
                closestBand = i;
            }
        }

        return closestBand;
    }

    void updateBandFromMouse(juce::Point<int> mousePos)
    {
        if (selectedBand < 0) return;

        auto bounds = getLocalBounds().toFloat();
        auto frequency = xToFrequency(mousePos.x, bounds);
        auto gain = yToGain(mousePos.y, bounds);

        // Constrain values
        frequency = juce::jlimit(20.0f, 20000.0f, frequency);
        gain = juce::jlimit(-15.0f, 15.0f, gain);

        const auto& currentBand = equalizer.getBand(selectedBand);
        equalizer.setBandParameters(selectedBand, frequency, gain, currentBand.Q);
    }
};
```

---

## Dynamic Range Compression

### Compressor Implementation

Dynamic range compression reduces the volume of loud sounds and can optionally amplify quiet sounds, effectively reducing the dynamic range of an audio signal.

```cpp
class DynamicCompressor
{
public:
    struct Parameters
    {
        float threshold = -10.0f;    // dB
        float ratio = 4.0f;          // 4:1 compression
        float attack = 0.003f;       // 3ms
        float release = 0.1f;        // 100ms
        float knee = 2.0f;           // dB
        float makeupGain = 0.0f;     // dB
        bool enabled = true;
    };

    DynamicCompressor() = default;

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;

        // Initialize envelope followers
        peakDetector.prepare(sampleRate);
        peakDetector.setTimes(params.attack, params.release);

        // Initialize gain smoothing
        gainSmoother.reset(sampleRate, 0.01); // 10ms smoothing

        reset();
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        if (!params.enabled) return;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get stereo peak for sidechain
            float peakLevel = 0.0f;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto sampleValue = std::abs(buffer.getSample(channel, sample));
                peakLevel = std::max(peakLevel, sampleValue);
            }

            // Convert to dB
            auto peakDb = juce::Decibels::gainToDecibels(peakLevel);

            // Calculate compression
            auto gainReduction = calculateGainReduction(peakDb);
            auto targetGain = juce::Decibels::decibelsToGain(gainReduction + params.makeupGain);

            // Smooth gain changes
            gainSmoother.setTargetValue(targetGain);
            auto currentGain = gainSmoother.getNextValue();

            // Apply gain to all channels
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto currentSample = buffer.getSample(channel, sample);
                buffer.setSample(channel, sample, currentSample * currentGain);
            }

            // Update meters
            updateMeters(peakDb, gainReduction);
        }
    }

    void setParameters(const Parameters& newParams)
    {
        params = newParams;
        peakDetector.setTimes(params.attack, params.release);
    }

    const Parameters& getParameters() const { return params; }

    // Metering
    float getInputLevel() const { return inputMeter.getCurrentRMS(); }
    float getOutputLevel() const { return outputMeter.getCurrentRMS(); }
    float getGainReduction() const { return currentGainReduction; }

    void reset()
    {
        peakDetector.reset();
        gainSmoother.reset();
        inputMeter.reset();
        outputMeter.reset();
        currentGainReduction = 0.0f;
    }

private:
    Parameters params;
    double sampleRate = 44100.0;

    AudioAnalysis::PeakDetector<float> peakDetector;
    juce::SmoothedValue<float> gainSmoother;

    // Metering
    AudioAnalysis::RMSAnalyzer<float> inputMeter{1024};
    AudioAnalysis::RMSAnalyzer<float> outputMeter{1024};
    float currentGainReduction = 0.0f;

    float calculateGainReduction(float inputDb)
    {
        if (inputDb <= params.threshold - params.knee * 0.5f)
        {
            // Below threshold - no compression
            return 0.0f;
        }
        else if (inputDb >= params.threshold + params.knee * 0.5f)
        {
            // Above knee - full compression
            auto overThreshold = inputDb - params.threshold;
            return -(overThreshold * (1.0f - 1.0f / params.ratio));
        }
        else
        {
            // In knee region - soft compression
            auto kneeInput = inputDb - params.threshold + params.knee * 0.5f;
            auto kneeRatio = kneeInput / params.knee;
            auto softRatio = 1.0f + (params.ratio - 1.0f) * kneeRatio * kneeRatio;
            auto overThreshold = inputDb - params.threshold;
            return -(overThreshold * (1.0f - 1.0f / softRatio));
        }
    }

    void updateMeters(float inputDb, float gainReduction)
    {
        currentGainReduction = gainReduction;
        // Meter updates would be handled by separate metering components
    }
};
```

### Compressor Visualization

```cpp
class CompressorVisualization : public juce::Component,
                               public juce::Timer
{
public:
    CompressorVisualization(DynamicCompressor& comp) : compressor(comp)
    {
        setSize(300, 200);
        startTimer(30);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto bounds = getLocalBounds().toFloat();

        // Draw compression curve
        drawCompressionCurve(g, bounds);

        // Draw current operating point
        drawOperatingPoint(g, bounds);

        // Draw gain reduction meter
        drawGainReductionMeter(g, bounds);
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    DynamicCompressor& compressor;

    void drawCompressionCurve(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        const auto& params = compressor.getParameters();

        g.setColour(juce::Colours::darkgrey);

        // Draw grid
        for (int db = -60; db <= 0; db += 10)
        {
            auto x =

            auto x = dbToX(db, bounds);
            g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
        }

        // Draw 1:1 line (no compression)
        g.setColour(juce::Colours::grey);
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getY());

        // Draw compression curve
        g.setColour(juce::Colours::cyan);
        juce::Path curvePath;

        for (int db = -60; db <= 0; ++db)
        {
            auto inputDb = static_cast<float>(db);
            auto outputDb = inputDb + calculateGainReduction(inputDb, params);

            auto x = dbToX(inputDb, bounds);
            auto y = dbToY(outputDb, bounds);

            if (db == -60)
                curvePath.startNewSubPath(x, y);
            else
                curvePath.lineTo(x, y);
        }

        g.strokePath(curvePath, juce::PathStrokeType(2.0f));
    }

    void drawOperatingPoint(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto inputLevel = compressor.getInputLevel();
        auto outputLevel = compressor.getOutputLevel();

        if (inputLevel > 0.0f && outputLevel > 0.0f)
        {
            auto inputDb = juce::Decibels::gainToDecibels(inputLevel);
            auto outputDb = juce::Decibels::gainToDecibels(outputLevel);

            auto x = dbToX(inputDb, bounds);
            auto y = dbToY(outputDb, bounds);

            g.setColour(juce::Colours::yellow);
            g.fillEllipse(x - 3, y - 3, 6, 6);
        }
    }

    void drawGainReductionMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto meterBounds = bounds.removeFromRight(30).reduced(5);

        g.setColour(juce::Colours::darkgrey);
        g.fillRect(meterBounds);

        auto gainReduction = compressor.getGainReduction();
        if (gainReduction < 0.0f)
        {
            auto meterHeight = (-gainReduction / 20.0f) * meterBounds.getHeight();
            auto meterRect = meterBounds.removeFromBottom(meterHeight);

            g.setColour(juce::Colours::red);
            g.fillRect(meterRect);
        }

        g.setColour(juce::Colours::white);
        g.drawRect(meterBounds, 1);
        g.drawText("GR", meterBounds.removeFromBottom(15), juce::Justification::centred, false);
    }

    float dbToX(float db, juce::Rectangle<float> bounds) const
    {
        auto normalizedDb = (db + 60.0f) / 60.0f; // -60dB to 0dB range
        return bounds.getX() + normalizedDb * bounds.getWidth();
    }

    float dbToY(float db, juce::Rectangle<float> bounds) const
    {
        auto normalizedDb = (db + 60.0f) / 60.0f; // -60dB to 0dB range
        return bounds.getBottom() - normalizedDb * bounds.getHeight();
    }

    float calculateGainReduction(float inputDb, const DynamicCompressor::Parameters& params) const
    {
        if (inputDb <= params.threshold - params.knee * 0.5f)
        {
            return 0.0f;
        }
        else if (inputDb >= params.threshold + params.knee * 0.5f)
        {
            auto overThreshold = inputDb - params.threshold;
            return -(overThreshold * (1.0f - 1.0f / params.ratio));
        }
        else
        {
            auto kneeInput = inputDb - params.threshold + params.knee * 0.5f;
            auto kneeRatio = kneeInput / params.knee;
            auto softRatio = 1.0f + (params.ratio - 1.0f) * kneeRatio * kneeRatio;
            auto overThreshold = inputDb - params.threshold;
            return -(overThreshold * (1.0f - 1.0f / softRatio));
        }
    }
};
```

---

## Delay Effects and Modulation

### Advanced Delay Line Implementation

Delay effects are fundamental to many audio processing techniques, from simple echo to complex modulation effects.

```cpp
template<typename SampleType>
class AdvancedDelayLine
{
public:
    AdvancedDelayLine(int maxDelayInSamples = 192000) // 4 seconds at 48kHz
        : maxDelay(maxDelayInSamples)
    {
        buffer.resize(maxDelay + 1);
        reset();
    }

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), SampleType(0));
        writeIndex = 0;
    }

    void setDelay(SampleType delayInSamples)
    {
        currentDelay = juce::jlimit(SampleType(0), SampleType(maxDelay), delayInSamples);
    }

    SampleType process(SampleType input)
    {
        // Write input to buffer
        buffer[writeIndex] = input;

        // Calculate read position with fractional delay
        auto readPosition = writeIndex - currentDelay;
        if (readPosition < 0)
            readPosition += maxDelay;

        // Interpolated read
        auto output = interpolatedRead(readPosition);

        // Advance write index
        writeIndex = (writeIndex + 1) % maxDelay;

        return output;
    }

    SampleType tapOut(SampleType delayInSamples) const
    {
        auto readPosition = writeIndex - delayInSamples;
        if (readPosition < 0)
            readPosition += maxDelay;

        return interpolatedRead(readPosition);
    }

private:
    std::vector<SampleType> buffer;
    int maxDelay;
    int writeIndex = 0;
    SampleType currentDelay = 0;
    double sampleRate = 44100.0;

    SampleType interpolatedRead(SampleType readPosition) const
    {
        auto index1 = static_cast<int>(std::floor(readPosition));
        auto index2 = (index1 + 1) % maxDelay;
        auto fraction = readPosition - index1;

        // Linear interpolation
        return buffer[index1] * (1.0f - fraction) + buffer[index2] * fraction;
    }
};

class ModulatedDelay
{
public:
    struct Parameters
    {
        float delayTime = 0.25f;     // seconds
        float feedback = 0.3f;       // 0-1
        float wetLevel = 0.5f;       // 0-1
        float modDepth = 0.0f;       // 0-1
        float modRate = 1.0f;        // Hz
        bool enabled = true;
    };

    ModulatedDelay()
    {
        // Initialize LFO for modulation
        lfo.initialise([](float x) { return std::sin(x); });
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;

        // Prepare delay lines for stereo
        leftDelay.prepare(sampleRate);
        rightDelay.prepare(sampleRate);

        // Prepare LFO
        lfo.prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
        lfo.setFrequency(params.modRate);

        // Prepare filters for feedback path
        feedbackFilter.prepare(sampleRate, maxBlockSize);
        feedbackFilter.setFilter(AdvancedDSP::BiquadFilter<float>::LowPass, 8000.0f, 0.707f);

        updateParameters();
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        if (!params.enabled) return;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get LFO value for modulation
            auto lfoValue = lfo.processSample(0.0f);
            auto modulation = lfoValue * params.modDepth * maxModulationSamples;

            // Process left channel
            if (buffer.getNumChannels() > 0)
            {
                auto input = buffer.getSample(0, sample);
                auto delayedSample = leftDelay.process(input + leftFeedback);
                auto output = input * (1.0f - params.wetLevel) + delayedSample * params.wetLevel;

                // Apply modulation to delay time
                leftDelay.setDelay(baseDelaySamples + modulation);

                // Update feedback with filtering
                leftFeedback = feedbackFilter.processSample(delayedSample * params.feedback);

                buffer.setSample(0, sample, output);
            }

            // Process right channel (with slight offset for stereo width)
            if (buffer.getNumChannels() > 1)
            {
                auto input = buffer.getSample(1, sample);
                auto delayedSample = rightDelay.process(input + rightFeedback);
                auto output = input * (1.0f - params.wetLevel) + delayedSample * params.wetLevel;

                // Apply modulation with phase offset
                rightDelay.setDelay(baseDelaySamples - modulation);

                rightFeedback = feedbackFilter.processSample(delayedSample * params.feedback);

                buffer.setSample(1, sample, output);
            }
        }
    }

    void setParameters(const Parameters& newParams)
    {
        params = newParams;
        lfo.setFrequency(params.modRate);
        updateParameters();
    }

    const Parameters& getParameters() const { return params; }

private:
    Parameters params;
    double sampleRate = 44100.0;

    AdvancedDelayLine<float> leftDelay;
    AdvancedDelayLine<float> rightDelay;

    juce::dsp::Oscillator<float> lfo;
    AdvancedDSP::BiquadFilter<float> feedbackFilter;

    float baseDelaySamples = 0;
    float maxModulationSamples = 0;
    float leftFeedback = 0;
    float rightFeedback = 0;

    void updateParameters()
    {
        if (sampleRate > 0)
        {
            baseDelaySamples = params.delayTime * sampleRate;
            maxModulationSamples = 0.01f * sampleRate; // Max 10ms modulation
        }
    }
};
```

---

## Real-Time Audio Visualization

### Spectrum Analyzer Implementation

Real-time frequency analysis is essential for professional audio applications.

```cpp
class SpectrumAnalyzer : public juce::Component,
                        public juce::Timer
{
public:
    SpectrumAnalyzer(int fftOrder = 11) // 2048 samples
        : fftSize(1 << fftOrder)
        , fft(fftOrder)
    {
        // Initialize FFT buffers
        fftBuffer.resize(fftSize * 2);
        frequencyBins.resize(fftSize / 2);
        smoothedBins.resize(fftSize / 2);

        // Initialize window function (Hann window)
        window.resize(fftSize);
        for (int i = 0; i < fftSize; ++i)
        {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
        }

        setSize(400, 200);
        startTimer(30); // 30 FPS
    }

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;

        // Calculate frequency bin centers
        binFrequencies.clear();
        for (int i = 0; i < fftSize / 2; ++i)
        {
            binFrequencies.push_back(i * sampleRate / fftSize);
        }
    }

    void processAudioBlock(const juce::AudioBuffer<float>& buffer)
    {
        // Process mono or take left channel
        auto* channelData = buffer.getReadPointer(0);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Add sample to circular buffer
            inputBuffer[inputBufferIndex] = channelData[sample];
            inputBufferIndex = (inputBufferIndex + 1) % fftSize;

            // Check if we have enough samples for FFT
            if (++samplesUntilNextFFT >= fftSize / 4) // 75% overlap
            {
                samplesUntilNextFFT = 0;
                performFFT();
            }
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto bounds = getLocalBounds().toFloat();

        // Draw frequency grid
        drawFrequencyGrid(g, bounds);

        // Draw spectrum
        drawSpectrum(g, bounds);

        // Draw peak markers
        drawPeakMarkers(g, bounds);
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    int fftSize;
    juce::dsp::FFT fft;
    std::vector<float> fftBuffer;
    std::vector<float> frequencyBins;
    std::vector<float> smoothedBins;
    std::vector<float> window;
    std::vector<float> binFrequencies;

    std::vector<float> inputBuffer{2048, 0.0f};
    int inputBufferIndex = 0;
    int samplesUntilNextFFT = 0;

    double sampleRate = 44100.0;

    // Peak detection
    std::vector<std::pair<float, float>> peaks; // frequency, magnitude

    void performFFT()
    {
        // Copy input buffer to FFT buffer with windowing
        for (int i = 0; i < fftSize; ++i)
        {
            auto bufferIndex = (inputBufferIndex + i) % fftSize;
            fftBuffer[i] = inputBuffer[bufferIndex] * window[i];
        }

        // Zero pad imaginary part
        for (int i = fftSize; i < fftSize * 2; ++i)
        {
            fftBuffer[i] = 0.0f;
        }

        // Perform FFT
        fft.performFrequencyOnlyForwardTransform(fftBuffer.data());

        // Convert to dB and smooth
        for (int i = 0; i < fftSize / 2; ++i)
        {
            auto magnitude = fftBuffer[i];
            auto magnitudeDb = juce::Decibels::gainToDecibels(magnitude + 1e-10f);

            // Smooth the spectrum
            smoothedBins[i] = smoothedBins[i] * 0.8f + magnitudeDb * 0.2f;
        }

        // Detect peaks
        detectPeaks();
    }

    void detectPeaks()
    {
        peaks.clear();

        // Simple peak detection
        for (int i = 2; i < static_cast<int>(smoothedBins.size()) - 2; ++i)
        {
            if (smoothedBins[i] > smoothedBins[i-1] &&
                smoothedBins[i] > smoothedBins[i+1] &&
                smoothedBins[i] > smoothedBins[i-2] &&
                smoothedBins[i] > smoothedBins[i+2] &&
                smoothedBins[i] > -40.0f) // Minimum threshold
            {
                peaks.emplace_back(binFrequencies[i], smoothedBins[i]);
            }
        }

        // Sort by magnitude and keep top peaks
        std::sort(peaks.begin(), peaks.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        if (peaks.size() > 10)
            peaks.resize(10);
    }

    void drawFrequencyGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::darkgrey);

        // Draw frequency lines
        std::vector<float> freqLines = {100, 1000, 10000};
        for (auto freq : freqLines)
        {
            auto x = frequencyToX(freq, bounds);
            g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());

            // Label
            g.setColour(juce::Colours::lightgrey);
            g.drawText(juce::String(freq < 1000 ? freq : freq / 1000) + (freq < 1000 ? "Hz" : "kHz"),
                      x - 20, bounds.getBottom() - 20, 40, 15, juce::Justification::centred);
            g.setColour(juce::Colours::darkgrey);
        }

        // Draw magnitude lines
        for (int db = -60; db <= 0; db += 20)
        {
            auto y = magnitudeToY(db, bounds);
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());

            // Label
            g.setColour(juce::Colours::lightgrey);
            g.drawText(juce::String(db) + "dB", bounds.getX() + 5, y - 7, 40, 15, juce::Justification::centredLeft);
            g.setColour(juce::Colours::darkgrey);
        }
    }

    void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::cyan);

        juce::Path spectrumPath;
        bool pathStarted = false;

        for (int i = 1; i < static_cast<int>(smoothedBins.size()); ++i)
        {
            auto frequency = binFrequencies[i];
            if (frequency < 20.0f || frequency > 20000.0f) continue;

            auto x = frequencyToX(frequency, bounds);
            auto y = magnitudeToY(smoothedBins[i], bounds);

            if (!pathStarted)
            {
                spectrumPath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                spectrumPath.lineTo(x, y);
            }
        }

        g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));
    }

    void drawPeakMarkers(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::yellow);

        for (const auto& peak : peaks)
        {
            auto x = frequencyToX(peak.first, bounds);
            auto y = magnitudeToY(peak.second, bounds);

            // Draw peak marker
            g.fillEllipse(x - 2, y - 2, 4, 4);

            // Draw frequency label
            g.drawText(juce::String(peak.first, 0) + "Hz",
                      x - 25, y - 20, 50, 15, juce::Justification::centred);
        }
    }

    float frequencyToX(float frequency, juce::Rectangle<float> bounds) const
    {
        auto logFreq = std::log10(frequency / 20.0f) / std::log10(1000.0f);
        return bounds.getX() + logFreq * bounds.getWidth();
    }

    float magnitudeToY(float magnitudeDb, juce::Rectangle<float> bounds) const
    {
        auto normalizedMag = (magnitudeDb + 80.0f) / 80.0f; // -80dB to 0dB range
        return bounds.getBottom() - normalizedMag * bounds.getHeight();
    }
};
```

### Real-Time Waveform Display

```cpp
class WaveformDisplay : public juce::Component,
                       public juce::Timer
{
public:
    WaveformDisplay(int bufferSize = 1024) : displayBufferSize(bufferSize)
    {
        displayBuffer.resize(displayBufferSize);
        setSize(400, 100);
        startTimer(60); // 60 FPS for smooth waveform
    }

    void processAudioBlock(const juce::AudioBuffer<float>& buffer)
    {
        // Downsample audio for display
        auto samplesPerPixel = buffer.getNumSamples() / displayBufferSize;
        if (samplesPerPixel < 1) samplesPerPixel = 1;

        auto* channelData = buffer.getReadPointer(0);

        for (int i = 0; i < displayBufferSize && i * samplesPerPixel < buffer.getNumSamples(); ++i)
        {
            float maxSample = 0.0f;

            // Find peak in this segment
            for (int j = 0; j < samplesPerPixel && (i * samplesPerPixel + j) < buffer.getNumSamples(); ++j)
            {
                auto sample = std::abs(channelData[i * samplesPerPixel + j]);
                maxSample = std::max(maxSample, sample);
            }

            displayBuffer[i] = maxSample;
        }

        // Trigger repaint
        triggerAsyncUpdate();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto bounds = getLocalBounds().toFloat();
        auto centerY = bounds.getCentreY();

        // Draw center line
        g.setColour(juce::Colours::darkgrey);
        g.drawHorizontalLine(centerY, bounds.getX(), bounds.getRight());

        // Draw waveform
        g.setColour(juce::Colours::green);

        auto pixelWidth = bounds.getWidth() / displayBufferSize;

        for (int i = 0; i < displayBufferSize; ++i)
        {
            auto x = bounds.getX() + i * pixelWidth;
            auto amplitude = displayBuffer[i] * bounds.getHeight() * 0.4f;

            // Draw vertical line representing amplitude
            g.drawVerticalLine(x, centerY - amplitude, centerY + amplitude);
        }

        // Draw level indicators
        drawLevelIndicators(g, bounds);
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    std::vector<float> displayBuffer;
    int displayBufferSize;

    void drawLevelIndicators(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(10.0f);

        // Draw amplitude markers
        auto centerY = bounds.getCentreY();
        auto quarterHeight = bounds.getHeight() * 0.25f;

        g.drawText("1.0", bounds.getRight() - 25, bounds.getY() + 2, 20, 12, juce::Justification::centred);
        g.drawText("0.5", bounds.getRight() - 25, centerY - quarterHeight - 6, 20, 12, juce::Justification::centred);
        g.drawText("0.0", bounds.getRight() - 25, centerY - 6, 20, 12, juce::Justification::centred);
        g.drawText("-0.5", bounds.getRight() - 25, centerY + quarterHeight - 6, 20, 12, juce::Justification::centred);
        g.drawText("-1.0", bounds.getRight() - 25, bounds.getBottom() - 14, 20, 12, juce::Justification::centred);
    }
};
```

---

## MIDI Integration and Processing

### MIDI Input Processing

MIDI integration allows for expressive control and musical interaction with audio processing.

```cpp
class MIDIProcessor : public juce::MidiInputCallback
{
public:
    struct MIDINote
    {
        int noteNumber;
        int velocity;
        int channel;
        double startTime;
        bool isActive;
    };

    MIDIProcessor()
    {
        // Initialize MIDI input
        auto midiInputs = juce::MidiInput::getAvailableDevices();

        if (!midiInputs.isEmpty())
        {
            midiInput = juce::MidiInput::openDevice(midiInputs[0].identifier, this);
            if (midiInput)
                midiInput->start();
        }
    }

    ~MIDIProcessor()
    {
        if (midiInput)
            midiInput->stop();
    }

    // MidiInputCallback interface
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        juce::ignoreUnused(source);

        if (message.isNoteOn())
        {
            handleNoteOn(message.getNoteNumber(), message.getVelocity(), message.getChannel());
        }
        else if (message.isNoteOff())
        {
            handleNoteOff(message.getNoteNumber(), message.getChannel());
        }
        else if (message.isControllerOfType(1)) // Mod wheel
        {
            handleModWheel(message.getControllerValue());
        }
        else if (message.isPitchWheel())
        {
            handlePitchBend(message.getPitchWheelValue());
        }
        else if (message.isControllerOfType(64)) // Sustain pedal
        {
            handleSustainPedal(message.getControllerValue() >= 64);
        }
    }

    // MIDI Learn functionality
    void startMIDILearn(const juce::String& parameterID)
    {
        midiLearnMode = true;
        midiLearnParameter = parameterID;
    }

    void stopMIDILearn()
    {
        midiLearnMode = false;
        midiLearnParameter.clear();
    }

    // Get current MIDI state
    std::vector<MIDINote> getActiveNotes() const
    {
        std::vector<MIDINote> activeNotes;
        for (const auto& note : currentNotes)
        {
            if (note.second.isActive)
                activeNotes.push_back(note.second);
        }
        return activeNotes;
    }

    float getModWheelValue() const { return modWheelValue; }
    float getPitchBendValue() const { return pitchBendValue; }
    bool getSustainPedalState() const { return sustainPedalDown; }

    // Parameter mapping
    void mapControllerToParameter(int controllerNumber, const juce::String& parameterID)
    {
        controllerMappings[controllerNumber] = parameterID;
    }

    std::function<void(const juce::String&, float)> onParameterChange;
    std::function<void(int, int, int)> onNoteOn;
    std::function<void(int, int)> onNoteOff;

private:
    std::unique_ptr<juce::MidiInput> midiInput;
    std::map<int, MIDINote> currentNotes; // noteNumber -> MIDINote
    std::map<int, juce::String> controllerMappings; // controller -> parameter ID

    float modWheelValue = 0.0f;
    float pitchBendValue = 0.0f;
    bool sustainPedalDown = false;

    bool midiLearnMode = false;
    juce::String midiLearnParameter;

    void handleNoteOn(int noteNumber, int velocity, int channel)
    {
        MIDINote note;
        note.noteNumber = noteNumber;
        note.velocity = velocity;
        note.channel = channel;
        note.startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        note.isActive = true;

        currentNotes[noteNumber] = note;

        if (onNoteOn)
            onNoteOn(noteNumber, velocity, channel);
    }

    void handleNoteOff(int noteNumber, int channel)
    {
        auto it = currentNotes.find(noteNumber);
        if (it != currentNotes.end())
        {
            it->second.isActive = false;

            if (!sustainPedalDown)
                currentNotes.erase(it);
        }

        if (onNoteOff)
            onNoteOff(noteNumber, channel);
    }

    void handleModWheel(int value)
    {
        modWheelValue = value / 127.0f;

        // Check for mapped parameter
        auto it = controllerMappings.find(1);
        if (it != controllerMappings.end() && onParameterChange)
        {
            onParameterChange(it->second, modWheelValue);
        }
    }

    void handlePitchBend(int value)
    {
        pitchBendValue = (value - 8192) / 8192.0f; // -1 to +1 range
    }

    void handleSustainPedal(bool isDown)
    {
        sustainPedalDown = isDown;

        // If pedal released, remove inactive notes
        if (!isDown)
        {
            auto it = currentNotes.begin();
            while (it != currentNotes.end())
            {
                if (!it->second.isActive)
                    it = currentNotes.erase(it);
                else
                    ++it;
            }
        }
    }

    void handleControllerMessage(int controllerNumber, int value)
    {
        if (midiLe

arnMode && !midiLearnParameter.isEmpty())
        {
            // Map this controller to the parameter being learned
            mapControllerToParameter(controllerNumber, midiLearnParameter);
            stopMIDILearn();
        }

        // Check for existing mapping
        auto it = controllerMappings.find(controllerNumber);
        if (it != controllerMappings.end() && onParameterChange)
        {
            auto normalizedValue = value / 127.0f;
            onParameterChange(it->second, normalizedValue);
        }
    }
};
```

### MPE (MIDI Polyphonic Expression) Support

```cpp
class MPEProcessor
{
public:
    struct MPENote
    {
        int noteNumber;
        int channel;
        float velocity;
        float pitchBend = 0.0f;
        float pressure = 0.0f;
        float timbre = 0.0f;
        double startTime;
        bool isActive = true;
    };

    MPEProcessor()
    {
        // Configure MPE zone (channels 2-15 for notes, channel 1 for master)
        masterChannel = 1;
        for (int i = 2; i <= 15; ++i)
        {
            availableChannels.push(i);
        }
    }

    void handleMidiMessage(const juce::MidiMessage& message)
    {
        auto channel = message.getChannel();

        if (message.isNoteOn())
        {
            handleMPENoteOn(message.getNoteNumber(), message.getVelocity(), channel);
        }
        else if (message.isNoteOff())
        {
            handleMPENoteOff(message.getNoteNumber(), channel);
        }
        else if (message.isPitchWheel())
        {
            handleMPEPitchBend(channel, message.getPitchWheelValue());
        }
        else if (message.isChannelPressure())
        {
            handleMPEPressure(channel, message.getChannelPressureValue());
        }
        else if (message.isControllerOfType(74)) // Timbre (CC74)
        {
            handleMPETimbre(channel, message.getControllerValue());
        }
    }

    std::vector<MPENote> getActiveNotes() const
    {
        std::vector<MPENote> activeNotes;
        for (const auto& note : activeNotes_)
        {
            if (note.second.isActive)
                activeNotes.push_back(note.second);
        }
        return activeNotes;
    }

    std::function<void(const MPENote&)> onNoteAdded;
    std::function<void(const MPENote&)> onNoteChanged;
    std::function<void(int)> onNoteRemoved;

private:
    int masterChannel;
    std::queue<int> availableChannels;
    std::map<int, MPENote> activeNotes_; // channel -> MPENote
    std::map<int, int> noteToChannel; // noteNumber -> channel

    void handleMPENoteOn(int noteNumber, int velocity, int channel)
    {
        // Assign channel if note on master channel
        if (channel == masterChannel && !availableChannels.empty())
        {
            channel = availableChannels.front();
            availableChannels.pop();
        }

        MPENote note;
        note.noteNumber = noteNumber;
        note.channel = channel;
        note.velocity = velocity / 127.0f;
        note.startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;

        activeNotes_[channel] = note;
        noteToChannel[noteNumber] = channel;

        if (onNoteAdded)
            onNoteAdded(note);
    }

    void handleMPENoteOff(int noteNumber, int channel)
    {
        auto it = activeNotes_.find(channel);
        if (it != activeNotes_.end())
        {
            it->second.isActive = false;

            if (onNoteRemoved)
                onNoteRemoved(noteNumber);

            activeNotes_.erase(it);
            noteToChannel.erase(noteNumber);

            // Return channel to available pool
            if (channel != masterChannel)
                availableChannels.push(channel);
        }
    }

    void handleMPEPitchBend(int channel, int pitchWheelValue)
    {
        auto it = activeNotes_.find(channel);
        if (it != activeNotes_.end())
        {
            it->second.pitchBend = (pitchWheelValue - 8192) / 8192.0f;

            if (onNoteChanged)
                onNoteChanged(it->second);
        }
    }

    void handleMPEPressure(int channel, int pressureValue)
    {
        auto it = activeNotes_.find(channel);
        if (it != activeNotes_.end())
        {
            it->second.pressure = pressureValue / 127.0f;

            if (onNoteChanged)
                onNoteChanged(it->second);
        }
    }

    void handleMPETimbre(int channel, int timbreValue)
    {
        auto it = activeNotes_.find(channel);
        if (it != activeNotes_.end())
        {
            it->second.timbre = timbreValue / 127.0f;

            if (onNoteChanged)
                onNoteChanged(it->second);
        }
    }
};
```

---

## Advanced Parameter Modulation

### Modulation Matrix System

A modulation matrix allows flexible routing of modulation sources to parameters.

```cpp
class ModulationMatrix
{
public:
    enum class ModulationSource
    {
        LFO1, LFO2, LFO3,
        Envelope1, Envelope2,
        ModWheel, PitchBend, Velocity,
        Random, Constant
    };

    struct ModulationConnection
    {
        ModulationSource source;
        juce::String targetParameter;
        float amount = 0.0f;
        bool enabled = true;
        int sourceIndex = 0; // For multiple instances of same source type
    };

    ModulationMatrix()
    {
        // Initialize LFOs
        for (auto& lfo : lfos)
        {
            lfo.initialise([](float x) { return std::sin(x); });
        }

        // Initialize envelopes
        for (auto& env : envelopes)
        {
            env.setParameters({0.1f, 0.5f, 0.7f, 1.0f}); // ADSR
        }
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;

        // Prepare LFOs
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = 1;

        for (auto& lfo : lfos)
        {
            lfo.prepare(spec);
            lfo.setFrequency(1.0f); // 1 Hz default
        }

        // Prepare envelopes
        for (auto& env : envelopes)
        {
            env.setSampleRate(sampleRate);
        }
    }

    void processBlock(int numSamples)
    {
        // Update modulation sources
        updateModulationSources(numSamples);

        // Apply modulations to parameters
        applyModulations();
    }

    void addConnection(ModulationSource source, const juce::String& targetParameter,
                      float amount, int sourceIndex = 0)
    {
        ModulationConnection connection;
        connection.source = source;
        connection.targetParameter = targetParameter;
        connection.amount = amount;
        connection.sourceIndex = sourceIndex;

        connections.push_back(connection);
    }

    void removeConnection(ModulationSource source, const juce::String& targetParameter)
    {
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [source, targetParameter](const ModulationConnection& conn) {
                    return conn.source == source && conn.targetParameter == targetParameter;
                }),
            connections.end());
    }

    void setLFOFrequency(int lfoIndex, float frequency)
    {
        if (lfoIndex >= 0 && lfoIndex < static_cast<int>(lfos.size()))
        {
            lfos[lfoIndex].setFrequency(frequency);
        }
    }

    void setLFOWaveform(int lfoIndex, std::function<float(float)> waveform)
    {
        if (lfoIndex >= 0 && lfoIndex < static_cast<int>(lfos.size()))
        {
            lfos[lfoIndex].initialise(waveform);
        }
    }

    void triggerEnvelope(int envelopeIndex)
    {
        if (envelopeIndex >= 0 && envelopeIndex < static_cast<int>(envelopes.size()))
        {
            envelopes[envelopeIndex].noteOn();
        }
    }

    void releaseEnvelope(int envelopeIndex)
    {
        if (envelopeIndex >= 0 && envelopeIndex < static_cast<int>(envelopes.size()))
        {
            envelopes[envelopeIndex].noteOff();
        }
    }

    // External modulation inputs
    void setModWheelValue(float value) { modWheelValue = value; }
    void setPitchBendValue(float value) { pitchBendValue = value; }
    void setVelocityValue(float value) { velocityValue = value; }

    std::function<void(const juce::String&, float)> onParameterModulation;

private:
    std::vector<ModulationConnection> connections;

    // Modulation sources
    std::array<juce::dsp::Oscillator<float>, 3> lfos;
    std::array<juce::ADSR, 2> envelopes;

    // Current modulation values
    std::array<float, 3> lfoValues{};
    std::array<float, 2> envelopeValues{};
    float modWheelValue = 0.0f;
    float pitchBendValue = 0.0f;
    float velocityValue = 0.0f;
    float randomValue = 0.0f;

    double sampleRate = 44100.0;
    juce::Random random;

    void updateModulationSources(int numSamples)
    {
        // Update LFOs
        for (int i = 0; i < static_cast<int>(lfos.size()); ++i)
        {
            lfoValues[i] = lfos[i].processSample(0.0f);
        }

        // Update envelopes
        for (int i = 0; i < static_cast<int>(envelopes.size()); ++i)
        {
            envelopeValues[i] = envelopes[i].getNextSample();
        }

        // Update random value occasionally
        static int randomCounter = 0;
        if (++randomCounter >= 1024) // Update every ~23ms at 44.1kHz
        {
            randomValue = random.nextFloat() * 2.0f - 1.0f; // -1 to +1
            randomCounter = 0;
        }
    }

    void applyModulations()
    {
        // Calculate modulation amounts for each parameter
        std::map<juce::String, float> parameterModulations;

        for (const auto& connection : connections)
        {
            if (!connection.enabled) continue;

            float sourceValue = getSourceValue(connection.source, connection.sourceIndex);
            float modulation = sourceValue * connection.amount;

            parameterModulations[connection.targetParameter] += modulation;
        }

        // Apply modulations
        if (onParameterModulation)
        {
            for (const auto& modulation : parameterModulations)
            {
                onParameterModulation(modulation.first, modulation.second);
            }
        }
    }

    float getSourceValue(ModulationSource source, int sourceIndex) const
    {
        switch (source)
        {
            case ModulationSource::LFO1:
            case ModulationSource::LFO2:
            case ModulationSource::LFO3:
            {
                int lfoIndex = static_cast<int>(source) - static_cast<int>(ModulationSource::LFO1);
                if (lfoIndex >= 0 && lfoIndex < static_cast<int>(lfoValues.size()))
                    return lfoValues[lfoIndex];
                break;
            }

            case ModulationSource::Envelope1:
            case ModulationSource::Envelope2:
            {
                int envIndex = static_cast<int>(source) - static_cast<int>(ModulationSource::Envelope1);
                if (envIndex >= 0 && envIndex < static_cast<int>(envelopeValues.size()))
                    return envelopeValues[envIndex];
                break;
            }

            case ModulationSource::ModWheel:
                return modWheelValue;

            case ModulationSource::PitchBend:
                return pitchBendValue;

            case ModulationSource::Velocity:
                return velocityValue;

            case ModulationSource::Random:
                return randomValue;

            case ModulationSource::Constant:
                return 1.0f;
        }

        return 0.0f;
    }
};
```

### Macro Controls System

```cpp
class MacroControlSystem
{
public:
    struct MacroControl
    {
        juce::String name;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        std::vector<std::pair<juce::String, float>> parameterMappings; // parameter ID, amount
    };

    MacroControlSystem(int numMacros = 8)
    {
        macroControls.resize(numMacros);

        // Initialize with default names
        for (int i = 0; i < numMacros; ++i)
        {
            macroControls[i].name = "Macro " + juce::String(i + 1);
        }
    }

    void setMacroValue(int macroIndex, float value)
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            auto& macro = macroControls[macroIndex];
            macro.value = juce::jlimit(macro.minValue, macro.maxValue, value);

            // Apply to mapped parameters
            applyMacroToParameters(macroIndex);
        }
    }

    float getMacroValue(int macroIndex) const
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            return macroControls[macroIndex].value;
        }
        return 0.0f;
    }

    void mapParameterToMacro(int macroIndex, const juce::String& parameterID, float amount)
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            auto& mappings = macroControls[macroIndex].parameterMappings;

            // Remove existing mapping if it exists
            mappings.erase(
                std::remove_if(mappings.begin(), mappings.end(),
                    [parameterID](const auto& mapping) {
                        return mapping.first == parameterID;
                    }),
                mappings.end());

            // Add new mapping
            if (std::abs(amount) > 0.001f) // Only add if amount is significant
            {
                mappings.emplace_back(parameterID, amount);
            }
        }
    }

    void setMacroRange(int macroIndex, float minValue, float maxValue)
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            auto& macro = macroControls[macroIndex];
            macro.minValue = minValue;
            macro.maxValue = maxValue;
            macro.value = juce::jlimit(minValue, maxValue, macro.value);
        }
    }

    void setMacroName(int macroIndex, const juce::String& name)
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            macroControls[macroIndex].name = name;
        }
    }

    const MacroControl& getMacroControl(int macroIndex) const
    {
        static MacroControl dummy;
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            return macroControls[macroIndex];
        }
        return dummy;
    }

    // Morphing between macro states
    void morphToState(const std::vector<float>& targetValues, float morphAmount)
    {
        for (int i = 0; i < static_cast<int>(macroControls.size()) && i < static_cast<int>(targetValues.size()); ++i)
        {
            auto currentValue = macroControls[i].value;
            auto targetValue = targetValues[i];
            auto morphedValue = currentValue + (targetValue - currentValue) * morphAmount;

            setMacroValue(i, morphedValue);
        }
    }

    std::function<void(const juce::String&, float)> onParameterChange;

private:
    std::vector<MacroControl> macroControls;

    void applyMacroToParameters(int macroIndex)
    {
        if (macroIndex >= 0 && macroIndex < static_cast<int>(macroControls.size()))
        {
            const auto& macro = macroControls[macroIndex];

            for (const auto& mapping : macro.parameterMappings)
            {
                auto parameterID = mapping.first;
                auto amount = mapping.second;
                auto modulation = macro.value * amount;

                if (onParameterChange)
                {
                    onParameterChange(parameterID, modulation);
                }
            }
        }
    }
};
```

---

## Performance Optimization

### SIMD Optimizations

Single Instruction, Multiple Data (SIMD) operations can significantly improve DSP performance.

```cpp
namespace SIMDOptimizations
{
    // SIMD-optimized audio processing functions
    class SIMDAudioProcessor
    {
    public:
        // Process audio buffer with SIMD operations
        static void processBufferSIMD(juce::AudioBuffer<float>& buffer, float gain)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                auto numSamples = buffer.getNumSamples();

                // Process in chunks of 4 samples using SIMD
                int simdSamples = numSamples & ~3; // Round down to multiple of 4

                for (int i = 0; i < simdSamples; i += 4)
                {
                    // Load 4 samples
                    auto samples = juce::FloatVectorOperations::load(&channelData[i]);

                    // Apply gain
                    samples = juce::FloatVectorOperations::multiply(samples, gain);

                    // Store back
                    juce::FloatVectorOperations::store(&channelData[i], samples);
                }

                // Process remaining samples
                for (int i = simdSamples; i < numSamples; ++i)
                {
                    channelData[i] *= gain;
                }
            }
        }

        // SIMD-optimized convolution
        static void convolveSIMD(const float* input, const float* impulse,
                               float* output, int inputLength, int impulseLength)
        {
            for (int i = 0; i < inputLength; ++i)
            {
                float sum = 0.0f;
                int j = 0;

                // SIMD processing for impulse response
                for (; j < impulseLength - 3; j += 4)
                {
                    if (i - j >= 0 && i - j - 3 >= 0)
                    {
                        // Load 4 input samples (reversed order for convolution)
                        juce::FloatVectorOperations::FloatType inputVec[4] = {
                            input[i - j], input[i - j - 1],
                            input[i - j - 2], input[i - j - 3]
                        };

                        // Load 4 impulse samples
                        auto impulseVec = juce::FloatVectorOperations::load(&impulse[j]);
                        auto inputVecSIMD = juce::FloatVectorOperations::load(inputVec);

                        // Multiply and accumulate
                        auto product = juce::FloatVectorOperations::multiply(inputVecSIMD, impulseVec);
                        sum += juce::FloatVectorOperations::sum(product);
                    }
                }

                // Process remaining samples
                for (; j < impulseLength; ++j)
                {
                    if (i - j >= 0)
                        sum += input[i - j] * impulse[j];
                }

                output[i] = sum;
            }
        }

        // SIMD-optimized biquad filter processing
        static void processBiquadSIMD(float* samples, int numSamples,
                                    float b0, float b1, float b2, float a1, float a2,
                                    float& x1, float& x2, float& y1, float& y2)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                auto input = samples[i];
                auto output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

                // Update state
                x2 = x1;
                x1 = input;
                y2 = y1;
                y1 = output;

                samples[i] = output;
            }
        }
    };

    // Template-based SIMD operations
    template<typename SampleType>
    class SIMDOperations
    {
    public:
        static void applyGain(SampleType* samples, int numSamples, SampleType gain)
        {
            if constexpr (std::is_same_v<SampleType, float>)
            {
                juce::FloatVectorOperations::multiply(samples, gain, numSamples);
            }
            else
            {
                // Fallback for non-float types
                for (int i = 0; i < numSamples; ++i)
                    samples[i] *= gain;
            }
        }

        static void addBuffers(SampleType* dest, const SampleType* src, int numSamples)
        {
            if constexpr (std::is_same_v<SampleType, float>)
            {
                juce::FloatVectorOperations::add(dest, src, numSamples);
            }
            else
            {
                for (int i = 0; i < numSamples; ++i)
                    dest[i] += src[i];
            }
        }

        static SampleType findPeak(const SampleType* samples, int numSamples)
        {
            if constexpr (std::is_same_v<SampleType, float>)
            {
                return juce::FloatVectorOperations::findMaximum(samples, numSamples);
            }
            else
            {
                SampleType peak = 0;
                for (int i = 0; i < numSamples; ++i)
                    peak = std::max(peak, std::abs(samples[i]));
                return peak;
            }
        }
    };
}
```

### Lock-Free Data Structures

```cpp
namespace LockFreeStructures
{
    // Lock-free circular buffer for audio/GUI communication
    template<typename T, size_t Size>
    class LockFreeCircularBuffer
    {
    public:
        static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");

        LockFreeCircularBuffer() : writeIndex(0), readIndex(0) {}

        bool push(const T& item)
        {
            auto currentWrite = writeIndex.load(std::memory_order_relaxed);
            auto nextWrite = (currentWrite + 1) & (Size - 1);

            if (nextWrite == readIndex.load(std::memory_order_acquire))
                return false; // Buffer full

            buffer[currentWrite] = item;
            writeIndex.store(nextWrite, std::memory_order_release);
            return true;
        }

        bool pop(T& item)
        {
            auto currentRead = readIndex.load(std::memory_order_relaxed);

            if (currentRead == writeIndex.load(std::memory_order_acquire))
                return false; // Buffer empty

            item = buffer[currentRead];
            readIndex.store((currentRead + 1) & (Size - 1), std::memory_order_release);
            return true;
        }

        size_t size() const
        {
            auto write = writeIndex.load(std::memory_order_acquire);
            auto read = readIndex.load(std::memory_order_acquire);
            return (write - read) & (Size - 1);
        }

        bool empty() const
        {
            return readIndex.load(std::memory_order_acquire) ==
                   writeIndex.load(std::memory_order_acquire);
        }

        bool full() const
        {
            auto currentWrite = writeIndex.load(std::memory_order_acquire);
            auto nextWrite = (currentWrite + 1) & (Size - 1);
            return nextWrite == readIndex.load(std::memory_order_acquire);
        }

    private:
        std::array<T, Size> buffer;
        std::atomic<size_t> writeIndex;
        std::atomic<size_t> readIndex;
    };

    // Lock-free parameter update system
    class LockFreeParameterUpdater
    {
    public:
        struct ParameterUpdate
        {
            juce::String parameterID;
            float value;
            uint64_t timestamp;
        };

        LockFreeParameterUpdater() = default;

        void pushUpdate(const juce::String& parameterID, float value)
        {
            ParameterUpdate update;
            update.parameterID = parameterID;
            update.value = value;
            update.timestamp = juce::Time::getMillisecondCounterHiRes();

            updateQueue.push(update);
        }

        void processUpdates(std::function<void(const juce::String&, float)> callback)
        {
            ParameterUpdate update;
            while (updateQueue.pop(update))
            {
                callback(update.parameterID, update.value);
            }
        }

        size_t getQueueSize() const
        {
            return updateQueue.size();
        }

    private:
        LockFreeCircularBuffer<ParameterUpdate, 1024> updateQueue;
    };
}
```

### Multi-Threading for Audio Processing

```cpp
class ThreadedAudioProcessor
{
public:
    ThreadedAudioProcessor(int numThreads = std::thread::hardware_concurrency())
        : threadPool(numThreads)
    {
        // Initialize per-thread data
        threadLocalData.resize(numThreads);
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;

        // Prepare thread-local processors
        for (auto& data : threadLocalData)
        {
            data.processor = std::make_unique<AdvancedDSP::BiquadFilter<float>>();
            data.processor->prepare(sampleRate, maxBlockSize);
        }
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        auto numChannels = buffer.getNumChannels();
        auto numSamples = buffer.getNumSamples();

        if (numChannels <= 1)
        {
            // Single channel - process directly
            processSingleChannel(buffer, 0);
        }
        else
        {
            // Multi-channel - distribute across threads
            std::vector<std::future<void>> futures;

            for (int channel = 0; channel < numChannels; ++channel)
            {
                futures.push_back(
                    threadPool.enqueue([this, &buffer, channel]() {
                        processSingleChannel(buffer, channel);
                    })
                );
            }

            // Wait for all threads to complete
            for (auto& future : futures)
            {
                future.wait();
            }
        }
    }

private:
    struct ThreadLocalData
    {
        std::unique_ptr<AdvancedDSP::BiquadFilter<float>> processor;
        std::vector<float> tempBuffer;
    };

    juce::ThreadPool threadPool;
    std::vector<ThreadLocalData> threadLocalData;
    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    void processSingleChannel(juce::AudioBuffer<float>& buffer, int channel)
    {
        auto threadId = std::hash<std::thread::id>{}(std::this_thread::get_id()) % threadLocalData.size();
        auto& data = threadLocalData[threadId];

        auto* channelData = buffer.getWritePointer(channel);
        auto numSamples = buffer.getNumSamples();

        // Process with thread-local processor
        data.processor->process(channelData, numSamples);
    }
};
```

---

## Professional Audio Features

### Oversampling Implementation

```cpp
class OversamplingProcessor
{
public:
    OversamplingProcessor(int oversamplingFactor = 4)
        : factor(oversamplingFactor)
    {
        // Create upsampling and downsampling filters
        createOversamplingFilters();
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->baseSampleRate = sampleRate;
        this->oversampledSampleRate

 = sampleRate * factor;
        this->baseBlockSize = maxBlockSize;
        this->oversampledBlockSize = maxBlockSize * factor;

        // Prepare upsampling and downsampling filters
        upsamplingFilter.prepare({oversampledSampleRate, static_cast<juce::uint32>(oversampledBlockSize), 2});
        downsamplingFilter.prepare({oversampledSampleRate, static_cast<juce::uint32>(oversampledBlockSize), 2});

        // Allocate oversampled buffer
        oversampledBuffer.setSize(2, oversampledBlockSize);
    }

    void processBlock(juce::AudioBuffer<float>& buffer,
                     std::function<void(juce::AudioBuffer<float>&)> processor)
    {
        // Upsample
        upsample(buffer);

        // Process at higher sample rate
        processor(oversampledBuffer);

        // Downsample back to original rate
        downsample(buffer);
    }

private:
    int factor;
    double baseSampleRate = 44100.0;
    double oversampledSampleRate = 176400.0;
    int baseBlockSize = 512;
    int oversampledBlockSize = 2048;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> upsamplingFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> downsamplingFilter;

    juce::AudioBuffer<float> oversampledBuffer;

    void createOversamplingFilters()
    {
        // Create anti-aliasing filters
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            oversampledSampleRate, baseSampleRate * 0.45f, 0.707f);

        upsamplingFilter.state = coefficients;
        downsamplingFilter.state = coefficients;
    }

    void upsample(const juce::AudioBuffer<float>& input)
    {
        oversampledBuffer.clear();

        // Zero-stuff upsampling
        for (int channel = 0; channel < input.getNumChannels(); ++channel)
        {
            auto* inputData = input.getReadPointer(channel);
            auto* outputData = oversampledBuffer.getWritePointer(channel);

            for (int i = 0; i < input.getNumSamples(); ++i)
            {
                outputData[i * factor] = inputData[i];
                // Zero-stuff intermediate samples
                for (int j = 1; j < factor; ++j)
                {
                    outputData[i * factor + j] = 0.0f;
                }
            }
        }

        // Apply anti-aliasing filter
        juce::dsp::AudioBlock<float> block(oversampledBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        upsamplingFilter.process(context);

        // Apply gain compensation
        oversampledBuffer.applyGain(static_cast<float>(factor));
    }

    void downsample(juce::AudioBuffer<float>& output)
    {
        // Apply anti-aliasing filter
        juce::dsp::AudioBlock<float> block(oversampledBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        downsamplingFilter.process(context);

        // Decimate
        for (int channel = 0; channel < output.getNumChannels(); ++channel)
        {
            auto* inputData = oversampledBuffer.getReadPointer(channel);
            auto* outputData = output.getWritePointer(channel);

            for (int i = 0; i < output.getNumSamples(); ++i)
            {
                outputData[i] = inputData[i * factor];
            }
        }
    }
};
```

### A/B Comparison System

```cpp
class ABComparisonSystem
{
public:
    ABComparisonSystem() = default;

    void prepare(double sampleRate, int maxBlockSize)
    {
        // Prepare delay compensation
        delayBuffer.setSize(2, maxBlockSize * 2);
        delayBuffer.clear();
    }

    void processBlock(juce::AudioBuffer<float>& buffer,
                     std::function<void(juce::AudioBuffer<float>&)> processA,
                     std::function<void(juce::AudioBuffer<float>&)> processB)
    {
        // Store original (dry) signal
        dryBuffer.makeCopyOf(buffer);

        if (currentState == State::A)
        {
            processA(buffer);
            processedABuffer.makeCopyOf(buffer);
        }
        else if (currentState == State::B)
        {
            processB(buffer);
            processedBBuffer.makeCopyOf(buffer);
        }
        else if (currentState == State::Dry)
        {
            // Keep original signal
        }

        // Apply crossfade if transitioning
        if (isTransitioning)
        {
            applyCrossfade(buffer);
        }

        // Apply level matching if enabled
        if (levelMatchingEnabled)
        {
            applyLevelMatching(buffer);
        }
    }

    void setState(State newState)
    {
        if (newState != currentState)
        {
            previousState = currentState;
            targetState = newState;
            startTransition();
        }
    }

    void setLevelMatching(bool enabled) { levelMatchingEnabled = enabled; }
    void setCrossfadeTime(float timeInSeconds) { crossfadeTime = timeInSeconds; }

    enum class State { Dry, A, B };
    State getCurrentState() const { return currentState; }

    // Get RMS levels for level matching
    float getDryLevel() const { return dryLevel.getCurrentRMS(); }
    float getProcessedLevel() const { return processedLevel.getCurrentRMS(); }

private:
    State currentState = State::Dry;
    State previousState = State::Dry;
    State targetState = State::Dry;

    bool isTransitioning = false;
    bool levelMatchingEnabled = true;
    float crossfadeTime = 0.05f; // 50ms default
    float crossfadePosition = 0.0f;

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> processedABuffer;
    juce::AudioBuffer<float> processedBBuffer;
    juce::AudioBuffer<float> delayBuffer;

    AudioAnalysis::RMSAnalyzer<float> dryLevel{1024};
    AudioAnalysis::RMSAnalyzer<float> processedLevel{1024};

    void startTransition()
    {
        isTransitioning = true;
        crossfadePosition = 0.0f;
    }

    void applyCrossfade(juce::AudioBuffer<float>& buffer)
    {
        auto numSamples = buffer.getNumSamples();
        auto samplesPerCrossfade = crossfadeTime * 44100.0f; // Assuming 44.1kHz
        auto fadeIncrement = 1.0f / samplesPerCrossfade;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            if (crossfadePosition >= 1.0f)
            {
                // Transition complete
                currentState = targetState;
                isTransitioning = false;
                break;
            }

            auto fadeIn = crossfadePosition;
            auto fadeOut = 1.0f - crossfadePosition;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto currentSample = buffer.getSample(channel, sample);
                auto previousSample = getPreviousStateSample(channel, sample);

                auto crossfadedSample = previousSample * fadeOut + currentSample * fadeIn;
                buffer.setSample(channel, sample, crossfadedSample);
            }

            crossfadePosition += fadeIncrement;
        }
    }

    void applyLevelMatching(juce::AudioBuffer<float>& buffer)
    {
        // Analyze levels
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                processedLevel.process(channelData[sample]);
            }
        }

        // Calculate and apply level compensation
        auto dryRMS = dryLevel.getCurrentRMS();
        auto processedRMS = processedLevel.getCurrentRMS();

        if (processedRMS > 0.001f && currentState != State::Dry)
        {
            auto compensationGain = dryRMS / processedRMS;
            compensationGain = juce::jlimit(0.1f, 10.0f, compensationGain); // Limit gain range

            buffer.applyGain(compensationGain);
        }
    }

    float getPreviousStateSample(int channel, int sample)
    {
        switch (previousState)
        {
            case State::Dry:
                return dryBuffer.getSample(channel, sample);
            case State::A:
                return processedABuffer.getSample(channel, sample);
            case State::B:
                return processedBBuffer.getSample(channel, sample);
        }
        return 0.0f;
    }
};
```

---

## Advanced C++ Techniques

### CRTP (Curiously Recurring Template Pattern)

```cpp
namespace AdvancedCPP
{
    // CRTP base class for DSP processors
    template<typename Derived>
    class DSPProcessorCRTP
    {
    public:
        void processBlock(juce::AudioBuffer<float>& buffer)
        {
            static_cast<Derived*>(this)->processBlockImpl(buffer);
        }

        void prepare(double sampleRate, int maxBlockSize)
        {
            static_cast<Derived*>(this)->prepareImpl(sampleRate, maxBlockSize);
        }

        void reset()
        {
            static_cast<Derived*>(this)->resetImpl();
        }

        // Common functionality
        void setBypass(bool shouldBypass) { bypassed = shouldBypass; }
        bool isBypassed() const { return bypassed; }

    protected:
        bool bypassed = false;
        double sampleRate = 44100.0;
        int maxBlockSize = 512;
    };

    // Concrete implementation using CRTP
    class DistortionProcessor : public DSPProcessorCRTP<DistortionProcessor>
    {
    public:
        void processBlockImpl(juce::AudioBuffer<float>& buffer)
        {
            if (bypassed) return;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);

                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    auto input = channelData[sample];
                    auto output = std::tanh(input * drive) * outputGain;
                    channelData[sample] = output;
                }
            }
        }

        void prepareImpl(double sr, int maxBlock)
        {
            sampleRate = sr;
            maxBlockSize = maxBlock;
        }

        void resetImpl()
        {
            // Reset any internal state
        }

        void setDrive(float newDrive) { drive = newDrive; }
        void setOutputGain(float newGain) { outputGain = newGain; }

    private:
        float drive = 1.0f;
        float outputGain = 1.0f;
    };

    // Template specialization for different sample types
    template<typename SampleType>
    class TemplateSpecializationExample
    {
    public:
        SampleType process(SampleType input);
    };

    // Specialization for float
    template<>
    class TemplateSpecializationExample<float>
    {
    public:
        float process(float input)
        {
            // Optimized float processing
            return std::sin(input * juce::MathConstants<float>::pi);
        }
    };

    // Specialization for double
    template<>
    class TemplateSpecializationExample<double>
    {
    public:
        double process(double input)
        {
            // High-precision double processing
            return std::sin(input * juce::MathConstants<double>::pi);
        }
    };

    // Constexpr compile-time calculations
    namespace CompileTimeCalculations
    {
        // Constexpr factorial
        constexpr int factorial(int n)
        {
            return n <= 1 ? 1 : n * factorial(n - 1);
        }

        // Constexpr power function
        constexpr double power(double base, int exp)
        {
            return exp == 0 ? 1.0 : base * power(base, exp - 1);
        }

        // Constexpr frequency to MIDI note conversion
        constexpr double frequencyToMidi(double frequency)
        {
            return 69.0 + 12.0 * std::log2(frequency / 440.0);
        }

        // Compile-time filter coefficient calculation
        template<int SampleRate, int CutoffFreq>
        struct LowPassCoefficients
        {
            static constexpr double omega = 2.0 * juce::MathConstants<double>::pi * CutoffFreq / SampleRate;
            static constexpr double alpha = std::sin(omega) / (2.0 * 0.707); // Q = 0.707
            static constexpr double cos_omega = std::cos(omega);

            static constexpr double b0 = (1.0 - cos_omega) * 0.5 / (1.0 + alpha);
            static constexpr double b1 = (1.0 - cos_omega) / (1.0 + alpha);
            static constexpr double b2 = b0;
            static constexpr double a1 = -2.0 * cos_omega / (1.0 + alpha);
            static constexpr double a2 = (1.0 - alpha) / (1.0 + alpha);
        };

        // Usage example
        using Filter1kHz = LowPassCoefficients<44100, 1000>;
        // Coefficients are calculated at compile time
    }

    // Variadic templates for flexible parameter handling
    template<typename... Args>
    class VariadicProcessor
    {
    public:
        template<typename Func>
        void processWithCallback(juce::AudioBuffer<float>& buffer, Func&& callback, Args... args)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);

                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    channelData[sample] = callback(channelData[sample], args...);
                }
            }
        }

        // Fold expression (C++17) for parameter processing
        template<typename... Processors>
        void processChain(juce::AudioBuffer<float>& buffer, Processors&... processors)
        {
            (processors.processBlock(buffer), ...); // Fold expression
        }
    };
}
```

---

## Practical Implementation

Now let's create a comprehensive advanced audio processor that combines all the techniques we've learned.

### Advanced Audio Processor Header

```cpp
// AdvancedAudioProcessor.h
#pragma once

#include <JuceHeader.h>
#include "MultiBandEQ.h"
#include "DynamicCompressor.h"
#include "ModulatedDelay.h"
#include "SpectrumAnalyzer.h"
#include "MIDIProcessor.h"
#include "ModulationMatrix.h"
#include "MacroControlSystem.h"

class AdvancedAudioProcessor : public juce::AudioProcessor
{
public:
    AdvancedAudioProcessor();
    ~AdvancedAudioProcessor() override;

    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Plugin information
    const juce::String getName() const override { return "Advanced Audio Processor"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    // Programs
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
    const juce::String getProgramName(int index) override { juce::ignoreUnused(index); return "Default"; }
    void changeProgramName(int index, const juce::String& newName) override { juce::ignoreUnused(index, newName); }

    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Access to components
    MultiBandEQ& getEQ() { return eq; }
    DynamicCompressor& getCompressor() { return compressor; }
    ModulatedDelay& getDelay() { return delay; }
    SpectrumAnalyzer& getSpectrumAnalyzer() { return spectrumAnalyzer; }
    MIDIProcessor& getMIDIProcessor() { return midiProcessor; }
    ModulationMatrix& getModulationMatrix() { return modulationMatrix; }
    MacroControlSystem& getMacroControls() { return macroControls; }

    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    // Parameter management
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Audio processing components
    MultiBandEQ eq;
    DynamicCompressor compressor;
    ModulatedDelay delay;

    // Analysis and visualization
    SpectrumAnalyzer spectrumAnalyzer;

    // MIDI and modulation
    MIDIProcessor midiProcessor;
    ModulationMatrix modulationMatrix;
    MacroControlSystem macroControls;

    // Performance optimization
    OversamplingProcessor oversamplingProcessor;
    ABComparisonSystem abComparison;

    // Parameter pointers for real-time access
    std::atomic<float>* masterGainParam = nullptr;
    std::atomic<float>* eqEnabledParam = nullptr;
    std::atomic<float>* compressorEnabledParam = nullptr;
    std::atomic<float>* delayEnabledParam = nullptr;

    // Internal state
    bool oversamplingEnabled = false;

    void updateParameters();
    void setupParameterCallbacks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancedAudioProcessor)
};
```

### Advanced Audio Processor Implementation

```cpp
// AdvancedAudioProcessor.cpp
#include "AdvancedAudioProcessor.h"
#include "AdvancedAudioEditor.h"

AdvancedAudioProcessor::AdvancedAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
    , spectrumAnalyzer(11) // 2048 FFT size
    , macroControls(8) // 8 macro controls
{
    // Get parameter pointers
    masterGainParam = parameters.getRawParameterValue("masterGain");
    eqEnabledParam = parameters.getRawParameterValue("eqEnabled");
    compressorEnabledParam = parameters.getRawParameterValue("compressorEnabled");
    delayEnabledParam = parameters.getRawParameterValue("delayEnabled");

    // Setup parameter callbacks
    setupParameterCallbacks();
}

AdvancedAudioProcessor::~AdvancedAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AdvancedAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Master controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterGain", "Master Gain",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; }
    ));

    // Component enables
    params.push_back(std::make_unique<juce::AudioParameterBool>("eqEnabled", "EQ Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("compressorEnabled", "Compressor Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("delayEnabled", "Delay Enabled", false));

    // EQ parameters
    for (int band = 0; band < MultiBandEQ::NumBands; ++band)
    {
        auto bandStr = juce::String(band + 1);

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "eqFreq" + bandStr, "EQ Band " + bandStr + " Frequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "eqGain" + bandStr, "EQ Band " + bandStr + " Gain",
            juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "eqQ" + bandStr, "EQ Band " + bandStr + " Q",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.3f), 0.707f
        ));
    }

    // Compressor parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compThreshold", "Compressor Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -10.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRatio", "Compressor Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.3f), 4.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compAttack", "Compressor Attack",
        juce::NormalisableRange<float>(0.001f, 0.1f, 0.001f, 0.3f), 0.003f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRelease", "Compressor Release",
        juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f, 0.3f), 0.1f
    ));

    // Delay parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.3f), 0.25f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.3f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayWet", "Delay Wet Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f
    ));

    // Macro controls
    for (int macro = 0; macro < 8; ++macro)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "macro" + juce::String(macro + 1), "Macro " + juce::String(macro + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f
        ));
    }

    return { params.begin(), params.end() };
}

void AdvancedAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare all components
    eq.prepare(sampleRate, samplesPerBlock);
    compressor.prepare(sampleRate, samplesPerBlock);
    delay.prepare(sampleRate, samplesPerBlock);
    spectrumAnalyzer.prepare(sampleRate);
    modulationMatrix.prepare(sampleRate, samplesPerBlock);
    oversamplingProcessor.prepare(sampleRate, samplesPerBlock);
    abComparison.prepare(sampleRate, samplesPerBlock);

    // Setup MIDI processor callbacks
    midiProcessor.onParameterChange = [this](const juce::String& paramID, float value)
    {
        if (auto* param = parameters.getParameter(paramID))
        {
            param->setValueNotifyingHost(value);
        }
    };

    // Setup modulation matrix callbacks
    modulationMatrix.onParameterModulation = [this](const juce::String& paramID, float modulation)
    {
        // Apply modulation to parameters
        if (auto* param = parameters.getParameter(paramID))
        {
            auto currentValue = param->getValue();
            auto modulatedValue = juce::jlimit(0.0f, 1.0f, currentValue + modulation);
            param->setValueNotifyingHost(modulatedValue);
        }
    };

    // Setup macro control callbacks
    macroControls.onParameterChange = [this](const juce::String& paramID, float value)
    {
        if (auto* param = parameters.getParameter(paramID))
        {
            param->setValueNotifyingHost(value);
        }
    };
}

void AdvancedAudioProcessor::releaseResources()
{
    eq.reset();
    compressor.reset();
    delay.reset();
}

void AdvancedAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        midiProcessor.handleMidiMessage(nullptr, metadata.getMessage());
    }

    // Update parameters
    updateParameters();

    // Process modulation matrix
    modulationMatrix.processBlock(buffer.getNumSamples());

    // Store dry signal for A/B comparison
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Define processing chain
    auto processChain = [this](juce::AudioBuffer<float>& audioBuffer)
    {
        // EQ
        if (eqEnabledParam && eqEnabledParam->load() > 0.5f)
        {
            eq.processBlock(audioBuffer);
        }

        // Compressor
        if (compressorEnabledParam && compressorEnabledParam->load() > 0.5f)
        {
            compressor.processBlock(audioBuffer);
        }

        // Delay
        if (delayEnabledParam && delayEnabledParam->load() > 0.5f)
        {
            delay.processBlock(audioBuffer);
        }
    };

    // Process with or without oversampling
    if (oversamplingEnabled)
    {
        oversamplingProcessor.processBlock(buffer, processChain);
    }
    else
    {
        processChain(buffer);
    }

    // Apply master gain
    if (masterGainParam)
    {
        auto gainDb = masterGainParam->load();
        auto gainLinear = juce::Decibels::decibelsToGain(gainDb);
        buffer.applyGain(gainLinear);
    }

    // Update spectrum analyzer
    spectrumAnalyzer.processAudioBlock(buffer);

    // A/B comparison processing
    abComparison.processBlock(buffer,
        [&](juce::AudioBuffer<float>& buf) { /* Process A */ },
        [&](juce::AudioBuffer<float>& buf) { /* Process B */ });
}

void AdvancedAudioProcessor::updateParameters()
{
    // Update EQ parameters
    for (int band = 0; band < MultiBandEQ::NumBands; ++band)
    {
        auto bandStr = juce::String(band + 1);

        if (auto* freqParam = parameters.getRawParameterValue("eqFreq" + bandStr))
        {
            if (auto* gainParam = parameters.getRawParameterValue("eqGain" + bandStr))
            {
                if (auto* qParam = parameters.getRawParameterValue("eqQ" + bandStr))
                {
                    eq.setBandParameters(band, freqParam->load(), gainParam->load(), qParam->load());
                }
            }
        }
    }

    // Update compressor parameters
    DynamicCompressor::Parameters compParams;
    if (auto* thresholdParam = parameters.getRaw

ParameterValue("compThreshold"))
    {
        compParams.threshold = thresholdParam->load();
    }
    if (auto* ratioParam = parameters.getRawParameterValue("compRatio"))
    {
        compParams.ratio = ratioParam->load();
    }
    if (auto* attackParam = parameters.getRawParameterValue("compAttack"))
    {
        compParams.attack = attackParam->load();
    }
    if (auto* releaseParam = parameters.getRawParameterValue("compRelease"))
    {
        compParams.release = releaseParam->load();
    }
    compressor.setParameters(compParams);

    // Update delay parameters
    ModulatedDelay::Parameters delayParams;
    if (auto* timeParam = parameters.getRawParameterValue("delayTime"))
    {
        delayParams.delayTime = timeParam->load();
    }
    if (auto* feedbackParam = parameters.getRawParameterValue("delayFeedback"))
    {
        delayParams.feedback = feedbackParam->load();
    }
    if (auto* wetParam = parameters.getRawParameterValue("delayWet"))
    {
        delayParams.wetLevel = wetParam->load();
    }
    delay.setParameters(delayParams);

    // Update macro controls
    for (int macro = 0; macro < 8; ++macro)
    {
        if (auto* macroParam = parameters.getRawParameterValue("macro" + juce::String(macro + 1)))
        {
            macroControls.setMacroValue(macro, macroParam->load());
        }
    }
}

void AdvancedAudioProcessor::setupParameterCallbacks()
{
    // Add parameter listeners for real-time updates
    parameters.addParameterListener("masterGain", this);
    parameters.addParameterListener("eqEnabled", this);
    parameters.addParameterListener("compressorEnabled", this);
    parameters.addParameterListener("delayEnabled", this);
}

void AdvancedAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AdvancedAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessorEditor* AdvancedAudioProcessor::createEditor()
{
    return new AdvancedAudioEditor(*this);
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AdvancedAudioProcessor();
}
```

---

## Practical Exercises

### Exercise 1: Implement and Tune a 3-Band EQ with Real-Time Visualization

**Objective**: Create a professional 3-band EQ with interactive frequency response visualization.

**Steps**:
1. Create a simplified 3-band EQ class based on the [`MultiBandEQ`](tutorials/Module_06_Advanced_Audio_Features.md:300) implementation
2. Implement real-time frequency response calculation
3. Create an interactive visualization component
4. Add mouse interaction for parameter adjustment
5. Implement proper parameter smoothing

**Implementation**:

```cpp
class ThreeBandEQ
{
public:
    struct Band
    {
        float frequency = 1000.0f;
        float gain = 0.0f;
        float Q = 0.707f;
        AdvancedDSP::BiquadFilter<float>::FilterType type;
        bool enabled = true;
    };

    ThreeBandEQ()
    {
        // Initialize bands: Low shelf, Mid peak, High shelf
        bands[0] = {200.0f, 0.0f, 0.707f, AdvancedDSP::BiquadFilter<float>::LowShelf, true};
        bands[1] = {1000.0f, 0.0f, 1.0f, AdvancedDSP::BiquadFilter<float>::Peak, true};
        bands[2] = {5000.0f, 0.0f, 0.707f, AdvancedDSP::BiquadFilter<float>::HighShelf, true};

        for (int i = 0; i < 3; ++i)
        {
            filters[i] = std::make_unique<AdvancedDSP::BiquadFilter<float>>();
        }
    }

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;
        for (int i = 0; i < 3; ++i)
        {
            filters[i]->prepare(sampleRate, maxBlockSize);
            updateBand(i);
        }
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float processedSample = channelData[sample];

                for (int band = 0; band < 3; ++band)
                {
                    if (bands[band].enabled)
                    {
                        processedSample = filters[band]->processSample(processedSample);
                    }
                }

                channelData[sample] = processedSample;
            }
        }
    }

    void setBandParameters(int bandIndex, float frequency, float gain, float Q)
    {
        if (bandIndex >= 0 && bandIndex < 3)
        {
            bands[bandIndex].frequency = frequency;
            bands[bandIndex].gain = gain;
            bands[bandIndex].Q = Q;
            updateBand(bandIndex);
        }
    }

    const Band& getBand(int index) const { return bands[index]; }

private:
    std::array<Band, 3> bands;
    std::array<std::unique_ptr<AdvancedDSP::BiquadFilter<float>>, 3> filters;
    double sampleRate = 44100.0;

    void updateBand(int bandIndex)
    {
        if (sampleRate > 0)
        {
            const auto& band = bands[bandIndex];
            auto gainLinear = juce::Decibels::decibelsToGain(band.gain);
            filters[bandIndex]->setFilter(band.type, band.frequency, band.Q, gainLinear);
        }
    }
};
```

**Expected Result**: A working 3-band EQ with visual feedback and mouse interaction.

**Code Challenge**: Add frequency response curve smoothing and implement band solo/mute functionality.

### Exercise 2: Create a Compressor with Sidechain Input and Visualization

**Objective**: Build a dynamic compressor with sidechain capability and real-time gain reduction visualization.

**Steps**:
1. Extend the [`DynamicCompressor`](tutorials/Module_06_Advanced_Audio_Features.md:600) class to support sidechain input
2. Implement gain reduction metering
3. Create a compression curve visualization
4. Add real-time gain reduction display
5. Implement sidechain filtering options

**Implementation**:

```cpp
class SidechainCompressor : public DynamicCompressor
{
public:
    void processBlockWithSidechain(juce::AudioBuffer<float>& mainBuffer,
                                  const juce::AudioBuffer<float>& sidechainBuffer)
    {
        if (!params.enabled) return;

        for (int sample = 0; sample < mainBuffer.getNumSamples(); ++sample)
        {
            // Use sidechain for detection
            float detectionLevel = 0.0f;
            if (sidechainBuffer.getNumSamples() > sample)
            {
                for (int channel = 0; channel < sidechainBuffer.getNumChannels(); ++channel)
                {
                    auto sampleValue = std::abs(sidechainBuffer.getSample(channel, sample));
                    detectionLevel = std::max(detectionLevel, sampleValue);
                }
            }
            else
            {
                // Fallback to main signal
                for (int channel = 0; channel < mainBuffer.getNumChannels(); ++channel)
                {
                    auto sampleValue = std::abs(mainBuffer.getSample(channel, sample));
                    detectionLevel = std::max(detectionLevel, sampleValue);
                }
            }

            // Apply sidechain filtering
            detectionLevel = sidechainFilter.processSample(detectionLevel);

            // Convert to dB and calculate compression
            auto detectionDb = juce::Decibels::gainToDecibels(detectionLevel);
            auto gainReduction = calculateGainReduction(detectionDb);
            auto targetGain = juce::Decibels::decibelsToGain(gainReduction + params.makeupGain);

            // Smooth gain changes
            gainSmoother.setTargetValue(targetGain);
            auto currentGain = gainSmoother.getNextValue();

            // Apply gain to main signal only
            for (int channel = 0; channel < mainBuffer.getNumChannels(); ++channel)
            {
                auto currentSample = mainBuffer.getSample(channel, sample);
                mainBuffer.setSample(channel, sample, currentSample * currentGain);
            }

            // Update gain reduction meter
            currentGainReduction = gainReduction;
        }
    }

    void setSidechainFilterFrequency(float frequency)
    {
        sidechainFilter.setFilter(AdvancedDSP::BiquadFilter<float>::HighPass, frequency, 0.707f);
    }

private:
    AdvancedDSP::BiquadFilter<float> sidechainFilter;
};
```

**Expected Result**: A compressor that responds to sidechain input with visual gain reduction feedback.

**Code Challenge**: Add lookahead processing and implement different detection modes (peak, RMS, hybrid).

### Exercise 3: Build a Delay Effect with Modulation and MIDI Sync

**Objective**: Create a sophisticated delay effect with tempo synchronization and modulation.

**Steps**:
1. Extend the [`ModulatedDelay`](tutorials/Module_06_Advanced_Audio_Features.md:900) class with MIDI sync
2. Implement tempo-based delay times
3. Add multiple modulation sources
4. Create stereo width and ping-pong effects
5. Implement delay time smoothing for glitch-free changes

**Implementation**:

```cpp
class AdvancedDelay : public ModulatedDelay
{
public:
    enum class SyncMode { Free, Sync };
    enum class NoteValue { Whole, Half, Quarter, Eighth, Sixteenth };

    struct AdvancedParameters : Parameters
    {
        SyncMode syncMode = SyncMode::Free;
        NoteValue noteValue = NoteValue::Quarter;
        float stereoWidth = 0.5f;
        bool pingPong = false;
        float tempoSync = 120.0f; // BPM
    };

    void setAdvancedParameters(const AdvancedParameters& newParams)
    {
        advancedParams = newParams;
        updateDelayTime();
    }

    void setTempo(double bpm)
    {
        advancedParams.tempoSync = bpm;
        updateDelayTime();
    }

    void processBlock(juce::AudioBuffer<float>& buffer) override
    {
        if (!params.enabled) return;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get modulated delay time
            auto lfoValue = lfo.processSample(0.0f);
            auto modulation = lfoValue * params.modDepth * maxModulationSamples;

            // Process left channel
            if (buffer.getNumChannels() > 0)
            {
                auto input = buffer.getSample(0, sample);
                auto delayedSample = leftDelay.process(input + leftFeedback);

                // Apply stereo width
                auto leftOutput = input * (1.0f - params.wetLevel) +
                                delayedSample * params.wetLevel * (1.0f - advancedParams.stereoWidth * 0.5f);

                leftDelay.setDelay(currentDelayTimeSamples + modulation);
                leftFeedback = delayedSample * params.feedback;

                buffer.setSample(0, sample, leftOutput);
            }

            // Process right channel with ping-pong
            if (buffer.getNumChannels() > 1)
            {
                auto input = buffer.getSample(1, sample);
                auto delayedSample = rightDelay.process(input + rightFeedback);

                if (advancedParams.pingPong)
                {
                    // Cross-feedback for ping-pong effect
                    rightFeedback = leftFeedback * params.feedback;
                    leftFeedback = delayedSample * params.feedback;
                }
                else
                {
                    rightFeedback = delayedSample * params.feedback;
                }

                auto rightOutput = input * (1.0f - params.wetLevel) +
                                 delayedSample * params.wetLevel * (1.0f + advancedParams.stereoWidth * 0.5f);

                rightDelay.setDelay(currentDelayTimeSamples - modulation);

                buffer.setSample(1, sample, rightOutput);
            }
        }
    }

private:
    AdvancedParameters advancedParams;
    float currentDelayTimeSamples = 0;

    void updateDelayTime()
    {
        if (advancedParams.syncMode == SyncMode::Sync)
        {
            // Calculate delay time based on tempo and note value
            auto noteValueMultiplier = getNoteValueMultiplier(advancedParams.noteValue);
            auto delayTimeSeconds = (60.0f / advancedParams.tempoSync) * noteValueMultiplier;
            currentDelayTimeSamples = delayTimeSeconds * sampleRate;
        }
        else
        {
            currentDelayTimeSamples = params.delayTime * sampleRate;
        }
    }

    float getNoteValueMultiplier(NoteValue noteValue)
    {
        switch (noteValue)
        {
            case NoteValue::Whole: return 4.0f;
            case NoteValue::Half: return 2.0f;
            case NoteValue::Quarter: return 1.0f;
            case NoteValue::Eighth: return 0.5f;
            case NoteValue::Sixteenth: return 0.25f;
        }
        return 1.0f;
    }
};
```

**Expected Result**: A delay effect that synchronizes to host tempo with advanced modulation options.

**Code Challenge**: Add multi-tap delays and implement ducking (automatic level reduction when input is present).

### Exercise 4: Implement MIDI Learn and Parameter Automation

**Objective**: Create a comprehensive MIDI learn system with parameter automation recording.

**Steps**:
1. Extend the [`MIDIProcessor`](tutorials/Module_06_Advanced_Audio_Features.md:1400) with automation recording
2. Implement parameter gesture recording and playback
3. Create a MIDI learn interface
4. Add automation curve editing
5. Implement automation quantization and smoothing

**Implementation**:

```cpp
class ParameterAutomation
{
public:
    struct AutomationPoint
    {
        double timeStamp;
        float value;
        float curvature = 0.0f; // For bezier curves
    };

    struct AutomationLane
    {
        juce::String parameterID;
        std::vector<AutomationPoint> points;
        bool enabled = true;
        bool recording = false;
    };

    void startRecording(const juce::String& parameterID)
    {
        auto& lane = getOrCreateLane(parameterID);
        lane.recording = true;
        lane.points.clear();
        recordingStartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    }

    void stopRecording(const juce::String& parameterID)
    {
        auto& lane = getOrCreateLane(parameterID);
        lane.recording = false;

        // Quantize points if enabled
        if (quantizationEnabled)
        {
            quantizeAutomationPoints(lane);
        }
    }

    void recordParameterChange(const juce::String& parameterID, float value)
    {
        auto& lane = getOrCreateLane(parameterID);
        if (!lane.recording) return;

        AutomationPoint point;
        point.timeStamp = juce::Time::getMillisecondCounterHiRes() / 1000.0 - recordingStartTime;
        point.value = value;

        lane.points.push_back(point);
    }

    float getAutomatedValue(const juce::String& parameterID, double currentTime)
    {
        auto& lane = getOrCreateLane(parameterID);
        if (!lane.enabled || lane.points.empty()) return 0.0f;

        // Find surrounding points
        auto it = std::lower_bound(lane.points.begin(), lane.points.end(), currentTime,
            [](const AutomationPoint& point, double time) {
                return point.timeStamp < time;
            });

        if (it == lane.points.begin())
            return lane.points.front().value;

        if (it == lane.points.end())
            return lane.points.back().value;

        // Interpolate between points
        auto& nextPoint = *it;
        auto& prevPoint = *(it - 1);

        auto timeDiff = nextPoint.timeStamp - prevPoint.timeStamp;
        auto timeRatio = (currentTime - prevPoint.timeStamp) / timeDiff;

        // Apply curve interpolation
        if (std::abs(prevPoint.curvature) > 0.001f)
        {
            // Bezier curve interpolation
            auto curvedRatio = applyCurve(timeRatio, prevPoint.curvature);
            return prevPoint.value + (nextPoint.value - prevPoint.value) * curvedRatio;
        }
        else
        {
            // Linear interpolation
            return prevPoint.value + (nextPoint.value - prevPoint.value) * timeRatio;
        }
    }

    void setQuantization(bool enabled, double gridSize = 0.25)
    {
        quantizationEnabled = enabled;
        quantizationGrid = gridSize;
    }

    std::function<void(const juce::String&, float)> onParameterAutomation;

private:
    std::map<juce::String, AutomationLane> automationLanes;
    double recordingStartTime = 0.0;
    bool quantizationEnabled = false;
    double quantizationGrid = 0.25; // Quarter note

    AutomationLane& getOrCreateLane(const juce::String& parameterID)
    {
        auto it = automationLanes.find(parameterID);
        if (it == automationLanes.end())
        {
            automationLanes[parameterID] = AutomationLane{parameterID, {}, true, false};
        }
        return automationLanes[parameterID];
    }

    void quantizeAutomationPoints(AutomationLane& lane)
    {
        for (auto& point : lane.points)
        {
            auto quantizedTime = std::round(point.timeStamp / quantizationGrid) * quantizationGrid;
            point.timeStamp = quantizedTime;
        }

        // Remove duplicate time stamps
        std::sort(lane.points.begin(), lane.points.end(),
            [](const AutomationPoint& a, const AutomationPoint& b) {
                return a.timeStamp < b.timeStamp;
            });

        lane.points.erase(
            std::unique(lane.points.begin(), lane.points.end(),
                [](const AutomationPoint& a, const AutomationPoint& b) {
                    return std::abs(a.timeStamp - b.timeStamp) < 0.001;
                }),
            lane.points.end());
    }

    float applyCurve(float t, float curvature)
    {
        if (curvature > 0.0f)
        {
            // Exponential curve
            return std::pow(t, 1.0f + curvature * 3.0f);
        }
        else
        {
            // Logarithmic curve
            return 1.0f - std::pow(1.0f - t, 1.0f - curvature * 3.0f);
        }
    }
};
```

**Expected Result**: A complete MIDI learn and automation system with curve editing capabilities.

**Code Challenge**: Add automation lane grouping and implement automation morphing between different takes.

---

## Code Examples and Best Practices

### Example 1: Professional Audio Buffer Management

```cpp
class AudioBufferManager
{
public:
    AudioBufferManager(int maxChannels = 8, int maxSamples = 4096)
        : maxChannels(maxChannels), maxSamples(maxSamples)
    {
        // Pre-allocate buffers to avoid real-time allocation
        for (int i = 0; i < BufferPoolSize; ++i)
        {
            bufferPool.push_back(std::make_unique<juce::AudioBuffer<float>>(maxChannels, maxSamples));
        }
    }

    // RAII buffer acquisition
    class ScopedBuffer
    {
    public:
        ScopedBuffer(AudioBufferManager& manager, int channels, int samples)
            : bufferManager(manager)
        {
            buffer = bufferManager.acquireBuffer(channels, samples);
        }

        ~ScopedBuffer()
        {
            if (buffer)
                bufferManager.releaseBuffer(buffer);
        }

        juce::AudioBuffer<float>* get() const { return buffer; }
        juce::AudioBuffer<float>* operator->() const { return buffer; }
        juce::AudioBuffer<float>& operator*() const { return *buffer; }

    private:
        AudioBufferManager& bufferManager;
        juce::AudioBuffer<float>* buffer = nullptr;
    };

    ScopedBuffer getScopedBuffer(int channels, int samples)
    {
        return ScopedBuffer(*this, channels, samples);
    }

private:
    static constexpr int BufferPoolSize = 16;

    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> bufferPool;
    std::vector<bool> bufferInUse{BufferPoolSize, false};
    int maxChannels, maxSamples;
    std::mutex bufferMutex;

    juce::AudioBuffer<float>* acquireBuffer(int channels, int samples)
    {
        std::lock_guard<std::mutex> lock(bufferMutex);

        jassert(channels <= maxChannels && samples <= maxSamples);

        for (int i = 0; i < BufferPoolSize; ++i)
        {
            if (!bufferInUse[i])
            {
                bufferInUse[i] = true;
                auto* buffer = bufferPool[i].get();
                buffer->setSize(channels, samples, false, true, true);
                return buffer;
            }
        }

        // Fallback: create temporary buffer (not ideal for real-time)
        jassertfalse; // Should not happen in well-designed system
        return new juce::AudioBuffer<float>(channels, samples);
    }

    void releaseBuffer(juce::AudioBuffer<float>* buffer)
    {
        std::lock_guard<std::mutex> lock(bufferMutex);

        for (int i = 0; i < BufferPoolSize; ++i)
        {
            if (bufferPool[i].get() == buffer)
            {
                bufferInUse[i] = false;
                return;
            }
        }

        // Was a temporary buffer
        delete buffer;
    }
};

// Usage example
void processAudio(juce::AudioBuffer<float>& mainBuffer, AudioBufferManager& bufferManager)
{
    // Acquire temporary buffer for processing
    auto tempBuffer = bufferManager.getScopedBuffer(mainBuffer.getNumChannels(), mainBuffer.getNumSamples());

    // Use the buffer
    tempBuffer->makeCopyOf(mainBuffer);

    // Process...

    // Buffer automatically released when tempBuffer goes out of scope
}
```

### Example 2: Real-Time Safe Parameter Updates

```cpp
class RealTimeSafeParameterManager
{
public:
    struct ParameterUpdate
    {
        juce::String parameterID;
        float value;
        std::atomic<bool> processed{false};
    };

    RealTimeSafeParameterManager()
    {
        // Pre-allocate update objects
        for (int i = 0; i < MaxUpdates; ++i)
        {
            updatePool.push_back(std::make_unique<ParameterUpdate>());
        }
    }

    // Called from GUI thread
    void scheduleParameterUpdate(const juce::String& parameterID, float value)
    {
        // Find available update object
        for (auto& update : updatePool)
        {
            bool expected = true;
            if (update->processed.compare_exchange_weak(expected, false))
            {
                update->parameterID = parameterID;
                update->value = value;

                // Add to queue
                updateQueue.push(update.get());
                return;
            }
        }

        // Queue full - could log warning
        DBG("Parameter update queue full!");
    }

    // Called from audio thread
    void processParameterUpdates()
    {
        ParameterUpdate* update;
        while (updateQueue.pop(update))
        {
            // Apply parameter change
            if (onParameterChange)
                onParameterChange(update->parameterID, update->value);

            // Mark as processed
            update->processed.store(true);
        }
    }

    std::function<void(const juce::String&, float)> onParameterChange;

private:
    static constexpr int MaxUpdates = 256;

    std::vector<std::unique_ptr<ParameterUpdate>> updatePool;
    LockFreeStructures::LockFreeCircularBuffer<ParameterUpdate*, MaxUpdates> updateQueue;
};
```

### Example 3: Advanced Error Handling and Diagnostics

```cpp
class AudioProcessorDiagnostics
{
public:
    struct ProcessingStats
    {
        double averageProcessingTime = 0.0;
        double peakProcessingTime = 0.0;
        double cpuUsagePercent = 0.0;
        int bufferUnderruns = 0;
        int parameterUpdates = 0;
        std::chrono::steady_clock::time_point lastUpdate;
    };

    AudioProcessorDiagnostics()
    {
        stats.lastUpdate = std::chrono::steady_clock::now();
    }

    void startProcessingTimer()
    {
        processingStartTime = std::chrono::high_resolution_clock::now();
    }

    void endProcessingTimer(double sampleRate, int bufferSize)
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto processingTime = std::chrono::duration<double>(endTime - processingStartTime).count();

        // Update statistics
        stats.averageProcessingTime = stats.averageProcessingTime * 0.99 + processingTime * 0.01;
        stats.peakProcessingTime = std::max(stats.peakProcessingTime, processingTime);

        // Calculate CPU usage
        auto bufferDuration = bufferSize / sampleRate;
        stats.cpuUsagePercent = (stats.averageProcessingTime / bufferDuration) * 100.0;

        // Check for buffer underruns
        if (processingTime > bufferDuration * 0.8) // 80% threshold
        {
            stats.bufferUnderruns++;
            logWarning("Potential buffer underrun detected", processingTime, bufferDuration);
        }
    }

    void logParameterUpdate()
    {
        stats.parameterUpdates++;
    }

    const ProcessingStats& getStats() const { return stats; }

    void resetStats()
    {
        stats = ProcessingStats{};
        stats.lastUpdate = std::chrono::steady_clock::now();
    }

    // Performance monitoring
    bool isPerformanceAcceptable() const
    {
        return stats.cpuUsagePercent < 80.0 && // Less than 80% CPU
               stats.averageProcessingTime < 0.01; // Less than 10ms average
    }

    juce::String getPerformanceReport() const
    {
        juce::String report;
        report << "=== Audio Processor Performance Report ===\n";
        report << "Average Processing Time: " << juce::String(stats.averageProcessingTime * 1000.0, 2) << " ms\n";
        report << "Peak Processing Time: " << juce::String(stats.peakProcessingTime * 1000.0, 2) << " ms\n";
        report << "CPU Usage: " << juce::String(stats.cpuUsagePercent, 1) << "%\n";
        report << "Buffer Underruns: " << juce::String(stats.bufferUnderruns) << "\n";
        report << "Parameter Updates: " << juce::String(stats.parameterUpdates) << "\n";
        report << "Performance Status: " << (isPerformanceAcceptable() ? "GOOD" : "WARNING") << "\n";

        return report;
    }

private:
    ProcessingStats stats;
    std::chrono::high_resolution_clock::time_point processingStartTime;

    void logWarning(const juce::String& message, double processingTime, double bufferDuration)
    {
        DBG("PERFORMANCE WARNING: " << message
            << " - Processing: " << (processingTime * 1000.0) << "ms"
            << " - Buffer: " << (bufferDuration * 1000.0) << "ms");
    }
};
```

---

## Summary

In this comprehensive Module 6, you've learned to implement advanced audio features that transform a basic audio application into a professional-grade audio processing system:

### Key Concepts Mastered

1. **Advanced DSP Algorithms**: Implemented multi-band EQ, dynamic compression, and modulated delay effects with professional-quality processing
2. **Real-Time Audio Visualization**: Created spectrum analyzers, waveform displays,

and level meters with real-time FFT processing and interactive visualization
3. **MIDI Integration**: Built comprehensive MIDI processing with MPE support, MIDI learn functionality, and parameter automation
4. **Advanced Parameter Modulation**: Created modulation matrices, macro controls, and sophisticated parameter routing systems
5. **Performance Optimization**: Implemented SIMD optimizations, lock-free data structures, and multi-threading for real-time audio
6. **Professional Audio Features**: Added oversampling, A/B comparison, zero-latency monitoring, and plugin state versioning
7. **Advanced C++ Techniques**: Mastered CRTP, template specialization, constexpr calculations, and variadic templates in audio context

### Advanced Features Implemented

- **Multi-Band EQ**: Professional 5-band equalizer with interactive frequency response visualization
- **Dynamic Compressor**: Full-featured compressor with sidechain support and gain reduction metering
- **Modulated Delay**: Advanced delay with tempo sync, modulation, and stereo effects
- **Spectrum Analyzer**: Real-time FFT-based frequency analysis with peak detection
- **MIDI Processing**: Complete MIDI input handling with MPE support and parameter mapping
- **Modulation System**: Flexible modulation matrix with multiple sources and targets
- **Macro Controls**: 8-macro system with parameter morphing and preset management
- **Performance Monitoring**: Real-time CPU usage tracking and optimization tools

### Professional Development Practices

- **Real-Time Safety**: Lock-free programming and atomic operations for audio thread safety
- **Memory Management**: Efficient buffer pooling and RAII patterns for real-time audio
- **Error Handling**: Comprehensive diagnostics and performance monitoring
- **Code Organization**: Template-based design patterns and modular architecture
- **Optimization**: SIMD processing and multi-threading for maximum performance
- **Testing**: Validation tools and performance benchmarking

### Industry-Standard Techniques

- **Oversampling**: Anti-aliasing for high-quality nonlinear processing
- **Parameter Smoothing**: Zipper-noise elimination with proper interpolation
- **MIDI Learn**: Professional parameter mapping and automation recording
- **A/B Comparison**: Level-matched comparison with crossfading
- **State Management**: Robust preset system with version compatibility
- **Plugin Validation**: Host compatibility and performance optimization

### Next Steps

This module represents the culmination of professional audio development techniques. You now have the knowledge to:

- Create commercial-quality audio plugins and applications
- Implement sophisticated DSP algorithms with optimal performance
- Build professional user interfaces with real-time visualization
- Handle complex MIDI processing and parameter automation
- Optimize code for real-time audio processing constraints
- Apply advanced C++ techniques in audio development contexts

### Recommended Further Study

- **Advanced DSP Theory**: Study filter design, spectral processing, and psychoacoustics
- **Plugin Standards**: Explore VST3, AAX, and AUv3 specifications in depth
- **Performance Optimization**: Learn advanced SIMD techniques and GPU processing
- **Machine Learning**: Investigate AI-based audio processing and analysis
- **Spatial Audio**: Study 3D audio, ambisonics, and immersive audio formats
- **Real-Time Systems**: Explore hard real-time programming and embedded audio

---

## Additional Resources

### Essential Reading
- [JUCE Advanced Audio Programming Guide](https://docs.juce.com/master/tutorial_advanced_audio.html)
- [Digital Signal Processing: A Practical Approach](https://www.dspguide.com/)
- [The Audio Programming Book](https://mitpress.mit.edu/books/audio-programming-book)
- [Real-Time Audio Programming](https://www.kadenze.com/courses/real-time-audio-programming-in-c)

### Professional Tools and Libraries
- [Intel IPP (Integrated Performance Primitives)](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/ipp.html)
- [FFTW (Fastest Fourier Transform in the West)](http://www.fftw.org/)
- [Eigen (C++ Template Library for Linear Algebra)](http://eigen.tuxfamily.org/)
- [Google Benchmark](https://github.com/google/benchmark)

### Industry Standards and Specifications
- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [Audio Unit Programming Guide](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/)
- [AAX SDK Documentation](https://developer.avid.com/aax/)
- [MIDI 2.0 Specification](https://www.midi.org/midi-articles/details-about-midi-2-0-midi-ci-profiles-and-property-exchange)

### Performance and Optimization
- [Intel Optimization Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-64-and-ia-32-architectures-optimization-reference-manual.html)
- [ARM NEON Programming Guide](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon)
- [Real-Time Systems Design Patterns](https://www.embedded.com/real-time-systems-design-patterns/)

---

## Troubleshooting Common Issues

### Performance Problems

**Problem**: High CPU usage during audio processing
**Solution**:
- Profile code to identify bottlenecks
- Implement SIMD optimizations for DSP loops
- Use lock-free data structures for thread communication
- Consider oversampling only when necessary
- Optimize memory access patterns

**Problem**: Audio dropouts or glitches
**Solution**:
- Ensure all audio thread code is real-time safe
- Avoid memory allocation in audio callbacks
- Use atomic variables for parameter updates
- Implement proper parameter smoothing
- Check for blocking operations in audio thread

### MIDI Integration Issues

**Problem**: MIDI events not being processed correctly
**Solution**:
- Verify MIDI input device is properly opened and started
- Check MIDI message parsing and filtering
- Ensure proper handling of MIDI channel messages
- Implement proper note-on/note-off pairing
- Add MIDI event timestamping for accurate timing

**Problem**: MIDI learn functionality not working
**Solution**:
- Verify MIDI input callback is receiving messages
- Check parameter ID mapping and validation
- Ensure proper thread safety for MIDI learn state
- Implement timeout for MIDI learn mode
- Add visual feedback for MIDI learn status

### Visualization Problems

**Problem**: Spectrum analyzer showing incorrect data
**Solution**:
- Verify FFT window function implementation
- Check sample rate and buffer size configuration
- Ensure proper magnitude scaling and dB conversion
- Implement proper frequency bin mapping
- Add input signal validation and range checking

**Problem**: Real-time visualization causing performance issues
**Solution**:
- Reduce visualization update rate (30 FPS max)
- Use separate thread for visualization processing
- Implement efficient drawing algorithms
- Cache expensive calculations
- Use appropriate data decimation for display

### Memory and Threading Issues

**Problem**: Memory leaks in audio processing
**Solution**:
- Use RAII patterns for resource management
- Implement proper buffer pooling
- Avoid dynamic allocation in audio thread
- Use smart pointers for automatic cleanup
- Regular memory profiling and leak detection

**Problem**: Thread synchronization issues
**Solution**:
- Use atomic variables for simple data exchange
- Implement lock-free circular buffers for queues
- Avoid mutexes in audio thread
- Use memory barriers for proper ordering
- Design wait-free algorithms where possible

### Plugin Integration Problems

**Problem**: Plugin not loading in DAW
**Solution**:
- Verify plugin format compatibility (VST3, AU, AAX)
- Check plugin validation with host tools
- Ensure proper plugin metadata and categorization
- Validate plugin binary architecture (32/64-bit)
- Test with multiple DAW applications

**Problem**: Parameter automation not working
**Solution**:
- Verify AudioProcessorValueTreeState setup
- Check parameter range and scaling
- Ensure proper parameter attachment in editor
- Implement parameter smoothing for automation
- Test automation recording and playback

---

## Performance Benchmarks

### Typical Performance Targets

| Component | CPU Usage (%) | Latency (ms) | Memory (MB) |
|-----------|---------------|--------------|-------------|
| Multi-Band EQ | 5-10 | <1 | 2-5 |
| Compressor | 3-8 | <1 | 1-3 |
| Delay Effect | 8-15 | <2 | 5-20 |
| Spectrum Analyzer | 10-20 | 10-50 | 5-10 |
| Complete Plugin | 25-50 | <5 | 20-50 |

### Optimization Checklist

- [ ] Profile code with performance tools
- [ ] Implement SIMD optimizations for DSP loops
- [ ] Use efficient memory access patterns
- [ ] Minimize dynamic memory allocation
- [ ] Implement proper parameter smoothing
- [ ] Use lock-free data structures
- [ ] Optimize visualization update rates
- [ ] Cache expensive calculations
- [ ] Use appropriate compiler optimizations
- [ ] Test on target hardware configurations

---

## Conclusion

Module 6 represents the pinnacle of JUCE audio development, combining advanced DSP algorithms, real-time visualization, MIDI processing, and professional optimization techniques. You've learned to create sophisticated audio applications that meet industry standards for performance, functionality, and user experience.

The techniques covered in this module form the foundation for developing commercial audio software, from simple effects plugins to complex digital audio workstations. The emphasis on real-time safety, performance optimization, and professional development practices ensures your code will perform reliably in demanding production environments.

As you continue your audio development journey, remember that mastery comes through practice and experimentation. The concepts and implementations provided here serve as a solid foundation, but the field of audio technology continues to evolve with new algorithms, hardware capabilities, and user expectations.

The combination of theoretical knowledge, practical implementation, and professional best practices covered in this module prepares you to tackle any audio development challenge and contribute meaningfully to the audio technology industry.

---

*This tutorial completes the comprehensive JUCE learning series, taking you from basic concepts to professional-grade audio application development. Each module builds upon previous knowledge while introducing increasingly sophisticated techniques and industry-standard practices.*

**Final Project Suggestion**: Combine all techniques learned across modules to create a complete audio workstation plugin with multi-track processing, advanced effects, MIDI sequencing, and professional visualization. This capstone project will demonstrate mastery of all concepts covered in the tutorial series.

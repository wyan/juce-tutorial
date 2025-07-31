#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>
#include <memory>

/**
 * Real-time spectrum analyzer with FFT processing and peak detection.
 * Provides frequency domain analysis with configurable FFT size and windowing.
 */
class SpectrumAnalyzer : public juce::Component,
                        public juce::Timer
{
public:
    SpectrumAnalyzer(int fftOrder = 11) // 2048 samples default
        : fftSize(1 << fftOrder)
        , fft(fftOrder)
        , inputBufferIndex(0)
        , samplesUntilNextFFT(0)
        , sampleRate(44100.0)
        , smoothing(0.8f)
        , peakHoldEnabled(true)
        , minFrequency(20.0f)
        , maxFrequency(20000.0f)
        , minMagnitude(-80.0f)
        , maxMagnitude(0.0f)
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

        // Initialize input buffer
        inputBuffer.resize(fftSize);
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);

        setSize(400, 200);
        startTimer(30); // 30 FPS
    }

    ~SpectrumAnalyzer() override
    {
        stopTimer();
    }

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;

        // Calculate frequency bin centers
        binFrequencies.clear();
        for (int i = 0; i < fftSize / 2; ++i)
        {
            binFrequencies.push_back(static_cast<float>(i * sampleRate / fftSize));
        }

        // Reset analysis
        reset();
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

        // Draw frequency labels
        drawFrequencyLabels(g, bounds);
    }

    void timerCallback() override
    {
        repaint();
    }

    // Configuration methods
    void setFFTSize(int newFFTOrder)
    {
        stopTimer();

        fftSize = 1 << newFFTOrder;
        fft = juce::dsp::FFT(newFFTOrder);

        // Resize buffers
        fftBuffer.resize(fftSize * 2);
        frequencyBins.resize(fftSize / 2);
        smoothedBins.resize(fftSize / 2);
        inputBuffer.resize(fftSize);
        window.resize(fftSize);

        // Reinitialize window
        for (int i = 0; i < fftSize; ++i)
        {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
        }

        reset();
        startTimer(30);
    }

    void setSmoothing(float smoothingFactor)
    {
        smoothing = juce::jlimit(0.0f, 0.99f, smoothingFactor);
    }

    void setPeakHold(bool enabled)
    {
        peakHoldEnabled = enabled;
        if (!enabled)
        {
            peakHoldBins.clear();
            peakHoldBins.resize(fftSize / 2, -100.0f);
        }
    }

    // Get current spectrum data
    const std::vector<float>& getFrequencyBins() const { return smoothedBins; }
    const std::vector<float>& getBinFrequencies() const { return binFrequencies; }

    // Get peaks
    std::vector<std::pair<float, float>> getPeaks() const { return peaks; }

private:
    int fftSize;
    juce::dsp::FFT fft;
    std::vector<float> fftBuffer;
    std::vector<float> frequencyBins;
    std::vector<float> smoothedBins;
    std::vector<float> peakHoldBins;
    std::vector<float> window;
    std::vector<float> binFrequencies;

    std::vector<float> inputBuffer;
    int inputBufferIndex;
    int samplesUntilNextFFT;

    double sampleRate;
    float smoothing;
    bool peakHoldEnabled;

    // Peak detection
    std::vector<std::pair<float, float>> peaks; // frequency, magnitude

    // Display parameters
    float minFrequency;
    float maxFrequency;
    float minMagnitude;
    float maxMagnitude;

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
            smoothedBins[i] = smoothedBins[i] * smoothing + magnitudeDb * (1.0f - smoothing);

            // Peak hold
            if (peakHoldEnabled)
            {
                if (peakHoldBins.size() <= static_cast<size_t>(i))
                    peakHoldBins.resize(i + 1, -100.0f);

                peakHoldBins[i] = std::max(peakHoldBins[i] * 0.999f, magnitudeDb);
            }
        }

        // Detect peaks
        detectPeaks();
    }

    void detectPeaks()
    {
        peaks.clear();

        // Simple peak detection with minimum separation
        const float minPeakSeparation = 100.0f; // Hz
        const float minPeakMagnitude = -40.0f; // dB

        for (int i = 2; i < static_cast<int>(smoothedBins.size()) - 2; ++i)
        {
            if (smoothedBins[i] > smoothedBins[i-1] &&
                smoothedBins[i] > smoothedBins[i+1] &&
                smoothedBins[i] > smoothedBins[i-2] &&
                smoothedBins[i] > smoothedBins[i+2] &&
                smoothedBins[i] > minPeakMagnitude &&
                binFrequencies[i] >= minFrequency &&
                binFrequencies[i] <= maxFrequency)
            {
                // Check minimum separation from existing peaks
                bool tooClose = false;
                for (const auto& existingPeak : peaks)
                {
                    if (std::abs(binFrequencies[i] - existingPeak.first) < minPeakSeparation)
                    {
                        tooClose = true;
                        break;
                    }
                }

                if (!tooClose)
                {
                    peaks.emplace_back(binFrequencies[i], smoothedBins[i]);
                }
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
        }

        // Draw magnitude lines
        for (int db = static_cast<int>(minMagnitude); db <= static_cast<int>(maxMagnitude); db += 20)
        {
            auto y = magnitudeToY(static_cast<float>(db), bounds);
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
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
            if (frequency < minFrequency || frequency > maxFrequency) continue;

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

        // Draw peak hold if enabled
        if (peakHoldEnabled)
        {
            g.setColour(juce::Colours::yellow.withAlpha(0.7f));
            juce::Path peakPath;
            pathStarted = false;

            for (int i = 1; i < static_cast<int>(peakHoldBins.size()); ++i)
            {
                auto frequency = binFrequencies[i];
                if (frequency < minFrequency || frequency > maxFrequency) continue;

                auto x = frequencyToX(frequency, bounds);
                auto y = magnitudeToY(peakHoldBins[i], bounds);

                if (!pathStarted)
                {
                    peakPath.startNewSubPath(x, y);
                    pathStarted = true;
                }
                else
                {
                    peakPath.lineTo(x, y);
                }
            }

            g.strokePath(peakPath, juce::PathStrokeType(1.0f));
        }
    }

    void drawPeakMarkers(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::yellow);
        g.setFont(10.0f);

        for (const auto& peak : peaks)
        {
            auto x = frequencyToX(peak.first, bounds);
            auto y = magnitudeToY(peak.second, bounds);

            // Draw peak marker
            g.fillEllipse(x - 2, y - 2, 4, 4);

            // Draw frequency label
            juce::String freqText;
            if (peak.first < 1000.0f)
                freqText = juce::String(peak.first, 0) + "Hz";
            else
                freqText = juce::String(peak.first / 1000.0f, 1) + "kHz";

            g.drawText(freqText, x - 25, y - 20, 50, 15, juce::Justification::centred);
        }
    }

    void drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(10.0f);

        // Frequency labels
        std::vector<std::pair<float, juce::String>> freqLabels = {
            {100.0f, "100Hz"}, {1000.0f, "1kHz"}, {10000.0f, "10kHz"}
        };

        for (const auto& label : freqLabels)
        {
            auto x = frequencyToX(label.first, bounds);
            g.drawText(label.second, x - 20, bounds.getBottom() - 20, 40, 15, juce::Justification::centred);
        }

        // Magnitude labels
        for (int db = static_cast<int>(minMagnitude); db <= static_cast<int>(maxMagnitude); db += 20)
        {
            auto y = magnitudeToY(static_cast<float>(db), bounds);
            g.drawText(juce::String(db) + "dB", bounds.getX() + 5, y - 7, 40, 15, juce::Justification::centredLeft);
        }
    }

    float frequencyToX(float frequency, juce::Rectangle<float> bounds) const
    {
        auto logFreq = std::log10(frequency / minFrequency) / std::log10(maxFrequency / minFrequency);
        return bounds.getX() + logFreq * bounds.getWidth();
    }

    float magnitudeToY(float magnitudeDb, juce::Rectangle<float> bounds) const
    {
        auto normalizedMag = (magnitudeDb - minMagnitude) / (maxMagnitude - minMagnitude);
        return bounds.getBottom() - normalizedMag * bounds.getHeight();
    }

    void reset()
    {
        std::fill(smoothedBins.begin(), smoothedBins.end(), minMagnitude);
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
        peakHoldBins.clear();
        peakHoldBins.resize(fftSize / 2, minMagnitude);
        inputBufferIndex = 0;
        samplesUntilNextFFT = 0;
        peaks.clear();
    }
};

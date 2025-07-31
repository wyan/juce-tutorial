#pragma once

#include <JuceHeader.h>
#include "BiquadFilter.h"
#include <array>
#include <memory>
#include <vector>
#include <complex>

/**
 * Professional multi-band equalizer with configurable bands and real-time frequency response calculation.
 * Supports up to 5 bands with different filter types per band.
 */
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
        // Initialize default bands with typical frequencies
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
            bands[bandIndex].frequency = juce::jlimit(20.0f, 20000.0f, frequency);
            bands[bandIndex].gain = juce::jlimit(-24.0f, 24.0f, gain);
            bands[bandIndex].Q = juce::jlimit(0.1f, 10.0f, Q);
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

    void setBandType(int bandIndex, AdvancedDSP::BiquadFilter<float>::FilterType type)
    {
        if (bandIndex >= 0 && bandIndex < NumBands)
        {
            bands[bandIndex].type = type;
            updateBand(bandIndex);
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
                    auto bandResponse = filters[band]->getFrequencyResponse(frequencies[i]);
                    totalResponse *= bandResponse;
                }
            }

            response[i] = std::abs(totalResponse);
        }

        return response;
    }

    // Get magnitude response in dB
    std::vector<float> getMagnitudeResponseDB(const std::vector<float>& frequencies) const
    {
        auto response = getFrequencyResponse(frequencies);
        std::vector<float> responseDB(response.size());

        for (size_t i = 0; i < response.size(); ++i)
        {
            responseDB[i] = juce::Decibels::gainToDecibels(response[i]);
        }

        return responseDB;
    }

    // Reset all filters
    void reset()
    {
        for (int i = 0; i < NumBands; ++i)
        {
            filters[i]->reset();
        }
    }

    // Bypass all processing
    void setBypass(bool shouldBypass)
    {
        bypassed = shouldBypass;
    }

    bool isBypassed() const { return bypassed; }

    // Get the current sample rate
    double getSampleRate() const { return sampleRate; }

private:
    std::array<Band, NumBands> bands;
    std::array<std::unique_ptr<AdvancedDSP::BiquadFilter<float>>, NumBands> filters;
    double sampleRate = 44100.0;
    bool bypassed = false;

    void updateBand(int bandIndex)
    {
        if (bandIndex >= 0 && bandIndex < NumBands && sampleRate > 0)
        {
            const auto& band = bands[bandIndex];
            auto gainLinear = juce::Decibels::decibelsToGain(band.gain);

            filters[bandIndex]->setFilter(band.type, band.frequency, band.Q, gainLinear);
        }
    }
};

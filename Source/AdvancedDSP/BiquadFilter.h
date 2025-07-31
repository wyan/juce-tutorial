#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <type_traits>

namespace AdvancedDSP
{
    /**
     * High-performance biquad filter implementation with multiple filter types.
     * Supports both float and double precision processing.
     */
    template<typename SampleType>
    class BiquadFilter
    {
    public:
        static_assert(std::is_floating_point_v<SampleType>, "Sample type must be floating point");

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

        BiquadFilter() = default;

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
            currentQ = Q; // Store Q for shelf filters
            auto omega = juce::MathConstants<SampleType>::twoPi * frequency / currentSampleRate;
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
                case LowShelf:
                    setLowShelfCoefficients(cos_omega, alpha, A);
                    break;
                case HighShelf:
                    setHighShelfCoefficients(cos_omega, alpha, A);
                    break;
                case Notch:
                    setNotchCoefficients(cos_omega, alpha);
                    break;
                case AllPass:
                    setAllPassCoefficients(cos_omega, alpha);
                    break;
            }
        }

        void prepare(double sampleRate, int maxBlockSize)
        {
            currentSampleRate = sampleRate;
            maxBufferSize = maxBlockSize;
            reset();
        }

        void process(SampleType* samples, int numSamples)
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

        void reset()
        {
            state = {};
        }

        // Get frequency response at given frequency
        std::complex<SampleType> getFrequencyResponse(SampleType frequency) const
        {
            auto omega = juce::MathConstants<SampleType>::twoPi * frequency / currentSampleRate;
            auto z = std::complex<SampleType>(std::cos(omega), std::sin(omega));
            auto z2 = z * z;

            auto numerator = coeffs.b0 + coeffs.b1 * z + coeffs.b2 * z2;
            auto denominator = SampleType(1.0) + coeffs.a1 * z + coeffs.a2 * z2;

            return numerator / denominator;
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

        double currentSampleRate = 44100.0;
        int maxBufferSize = 512;

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
            coeffs.a1 = -2.0 * std::cos(std::asin(sin_omega)) * norm;
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

        void setLowShelfCoefficients(SampleType cos_omega, SampleType alpha, SampleType A)
        {
            auto S = 1.0;
            auto beta = std::sqrt(A) / currentQ;

            auto norm = 1.0 / ((A + 1.0) + (A - 1.0) * cos_omega + beta);
            coeffs.b0 = A * ((A + 1.0) - (A - 1.0) * cos_omega + beta) * norm;
            coeffs.b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos_omega) * norm;
            coeffs.b2 = A * ((A + 1.0) - (A - 1.0) * cos_omega - beta) * norm;
            coeffs.a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos_omega) * norm;
            coeffs.a2 = ((A + 1.0) + (A - 1.0) * cos_omega - beta) * norm;
        }

        void setHighShelfCoefficients(SampleType cos_omega, SampleType alpha, SampleType A)
        {
            auto S = 1.0;
            auto beta = std::sqrt(A) / currentQ;

            auto norm = 1.0 / ((A + 1.0) - (A - 1.0) * cos_omega + beta);
            coeffs.b0 = A * ((A + 1.0) + (A - 1.0) * cos_omega + beta) * norm;
            coeffs.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cos_omega) * norm;
            coeffs.b2 = A * ((A + 1.0) + (A - 1.0) * cos_omega - beta) * norm;
            coeffs.a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cos_omega) * norm;
            coeffs.a2 = ((A + 1.0) - (A - 1.0) * cos_omega - beta) * norm;
        }

        void setNotchCoefficients(SampleType cos_omega, SampleType alpha)
        {
            auto norm = 1.0 / (1.0 + alpha);
            coeffs.b0 = norm;
            coeffs.b1 = -2.0 * cos_omega * norm;
            coeffs.b2 = norm;
            coeffs.a1 = coeffs.b1;
            coeffs.a2 = (1.0 - alpha) * norm;
        }

        void setAllPassCoefficients(SampleType cos_omega, SampleType alpha)
        {
            auto norm = 1.0 / (1.0 + alpha);
            coeffs.b0 = (1.0 - alpha) * norm;
            coeffs.b1 = -2.0 * cos_omega * norm;
            coeffs.b2 = (1.0 + alpha) * norm;
            coeffs.a1 = coeffs.b1;
            coeffs.a2 = coeffs.b0;
        }

        SampleType currentQ = 0.707; // Store Q for shelf filters
    };
}

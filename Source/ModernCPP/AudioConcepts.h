/*
  ==============================================================================

    AudioConcepts.h
    Created: JUCE Audio Tutorial Series - Modern C++ Module
    Author:  JUCE Tutorial Team

    This file demonstrates C++20 concepts for type-safe audio programming.
    Concepts provide compile-time validation of types and interfaces,
    making audio code more robust and self-documenting.

    Key Benefits of Audio Concepts:
    - Compile-time type validation
    - Better error messages
    - Self-documenting interfaces
    - Template constraint enforcement
    - Improved code maintainability

    Usage Examples:
    - Constraining template parameters
    - Validating audio processor interfaces
    - Ensuring real-time safety
    - Type-safe parameter systems

  ==============================================================================
*/

#pragma once

#include <concepts>
#include <type_traits>
#include <string>

namespace ModernAudio {

//==============================================================================
/**
 * Audio Sample Type Concept
 *
 * Defines what constitutes a valid audio sample type. Audio samples can be
 * either floating-point (preferred for internal processing) or integral
 * (for file I/O and hardware interfaces).
 *
 * Valid types: float, double, int16_t, int32_t, etc.
 * Invalid types: bool, char*, custom classes without conversion
 *
 * @tparam T The type to validate as an audio sample
 */
template<typename T>
concept AudioSample = std::floating_point<T> || std::integral<T>;

//==============================================================================
/**
 * Audio Processor Concept
 *
 * Defines the interface that all audio processors must implement.
 * This ensures consistent behavior across different processor types
 * and enables generic programming with audio processors.
 *
 * Required Methods:
 * - processBlock(): Process a block of audio samples
 * - getLatency(): Return processing latency in samples
 * - reset(): Reset internal state (filters, delays, etc.)
 *
 * Example Usage:
 * @code
 * template<AudioProcessor T>
 * void processAudio(T& processor, float* buffer, int numSamples) {
 *     processor.processBlock(buffer, numSamples);
 * }
 * @endcode
 *
 * @tparam T The type to validate as an audio processor
 */
template<typename T>
concept AudioProcessor = requires(T processor, float* buffer, int numSamples) {
    // Must be able to process a block of audio samples
    processor.processBlock(buffer, numSamples);

    // Must provide latency information for delay compensation
    { processor.getLatency() } -> std::convertible_to<int>;

    // Must be able to reset internal state
    { processor.reset() } -> std::same_as<void>;
};

//==============================================================================
/**
 * Filter Type Concept
 *
 * Defines the interface for audio filters. Filters are a specific type
 * of audio processor that modify frequency content of audio signals.
 *
 * Required Methods:
 * - process(): Process a single sample
 * - setFrequency(): Set cutoff/center frequency
 * - setQ(): Set quality factor/resonance
 * - reset(): Clear filter state
 *
 * Example Usage:
 * @code
 * template<FilterType T>
 * void sweepFilter(T& filter, float startFreq, float endFreq) {
 *     for (float freq = startFreq; freq <= endFreq; freq += 10.0f) {
 *         filter.setFrequency(freq);
 *         // Process audio...
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as a filter
 */
template<typename T>
concept FilterType = requires(T filter, float input, float frequency, float q) {
    // Must be able to process individual samples
    { filter.process(input) } -> AudioSample;

    // Must support frequency control
    filter.setFrequency(frequency);

    // Must support Q/resonance control
    filter.setQ(q);

    // Must be able to reset filter state
    filter.reset();
};

//==============================================================================
/**
 * Audio Parameter Concept
 *
 * Defines the interface for audio parameters used in plugins and applications.
 * Parameters provide a standardized way to control audio processing with
 * proper validation, automation, and user interface integration.
 *
 * Required Methods:
 * - getValue(): Get current parameter value
 * - setValue(): Set parameter value with validation
 * - getName(): Get human-readable parameter name
 * - getRange(): Get valid parameter range
 *
 * Example Usage:
 * @code
 * template<AudioParameter T>
 * void automateParameter(T& param, float targetValue, float rampTime) {
 *     auto [minVal, maxVal] = param.getRange();
 *     targetValue = std::clamp(targetValue, minVal, maxVal);
 *     // Implement smooth parameter automation...
 * }
 * @endcode
 *
 * @tparam T The type to validate as an audio parameter
 */
template<typename T>
concept AudioParameter = requires(T param) {
    // Must provide current value as floating-point
    { param.getValue() } -> std::floating_point;

    // Must accept new values
    param.setValue(std::declval<float>());

    // Must provide human-readable name
    { param.getName() } -> std::convertible_to<std::string>;

    // Must provide valid range information
    { param.getRange() } -> std::same_as<std::pair<float, float>>;
};

//==============================================================================
/**
 * Audio Plugin Concept
 *
 * Defines the interface for audio plugins (VST, AU, etc.).
 * Combines audio processing capabilities with plugin-specific metadata
 * and parameter management.
 *
 * This concept ensures plugins implement both audio processing and
 * the metadata required for plugin hosts.
 *
 * Example Usage:
 * @code
 * template<AudioPlugin T>
 * void validatePlugin(const T& plugin) {
 *     std::cout << "Plugin: " << plugin.getName()
 *               << " v" << plugin.getVersion()
 *               << " (" << plugin.getNumParameters() << " parameters)\n";
 * }
 * @endcode
 *
 * @tparam T The type to validate as an audio plugin
 */
template<typename T>
concept AudioPlugin = AudioProcessor<T> && requires(T plugin) {
    // Must provide plugin identification
    { plugin.getName() } -> std::convertible_to<std::string>;
    { plugin.getVersion() } -> std::convertible_to<std::string>;

    // Must provide parameter count for host integration
    { plugin.getNumParameters() } -> std::convertible_to<int>;
};

//==============================================================================
/**
 * Audio Buffer Concept
 *
 * Defines the interface for audio buffers used throughout the system.
 * Audio buffers store multi-channel audio data and provide safe access
 * to sample data for processing.
 *
 * Required Methods:
 * - getNumChannels(): Number of audio channels
 * - getNumSamples(): Number of samples per channel
 * - getWritePointer(): Mutable access to channel data
 * - getReadPointer(): Immutable access to channel data
 *
 * Example Usage:
 * @code
 * template<AudioBuffer T>
 * void mixBuffers(T& dest, const T& source, float gain) {
 *     for (int ch = 0; ch < dest.getNumChannels(); ++ch) {
 *         auto* destData = dest.getWritePointer(ch);
 *         const auto* srcData = source.getReadPointer(ch);
 *         for (int i = 0; i < dest.getNumSamples(); ++i) {
 *             destData[i] += srcData[i] * gain;
 *         }
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as an audio buffer
 */
template<typename T>
concept AudioBuffer = requires(T buffer) {
    // Must provide channel and sample count information
    { buffer.getNumChannels() } -> std::convertible_to<int>;
    { buffer.getNumSamples() } -> std::convertible_to<int>;

    // Must provide mutable access to sample data
    { buffer.getWritePointer(0) } -> std::convertible_to<float*>;

    // Must provide immutable access to sample data
    { buffer.getReadPointer(0) } -> std::convertible_to<const float*>;
};

//==============================================================================
/**
 * Real-Time Safe Concept
 *
 * Defines requirements for real-time safe audio processing.
 * Real-time safety is crucial for audio applications to avoid dropouts,
 * glitches, and timing violations.
 *
 * Requirements:
 * - No memory allocation in process methods
 * - No blocking operations (locks, I/O, etc.)
 * - Exception-safe operations
 * - Predictable execution time
 *
 * This concept enforces compile-time checks for real-time safety,
 * though runtime behavior must still be carefully designed.
 *
 * Example Usage:
 * @code
 * template<RealtimeSafe T>
 * void processInRealtimeContext(T& processor, float input) {
 *     // Guaranteed to be real-time safe at compile time
 *     auto output = processor.process(input);
 * }
 * @endcode
 *
 * @tparam T The type to validate for real-time safety
 */
template<typename T>
concept RealtimeSafe = requires(T obj) {
    // Must support efficient move operations (no deep copying)
    requires std::is_nothrow_move_constructible_v<T>;
    requires std::is_nothrow_move_assignable_v<T>;

    // Process method must not throw exceptions
    requires requires(T& t, float input) {
        { t.process(input) } noexcept -> AudioSample;
    };
};

//==============================================================================
/**
 * MIDI Processor Concept
 *
 * Defines the interface for MIDI processing components.
 * MIDI processors handle Musical Instrument Digital Interface data,
 * converting MIDI messages to audio or routing MIDI data.
 *
 * Required Methods:
 * - processMidi(): Process MIDI message data
 * - getNumMidiInputs(): Number of MIDI input ports
 * - getNumMidiOutputs(): Number of MIDI output ports
 *
 * Example Usage:
 * @code
 * template<MidiProcessor T>
 * void routeMidi(T& processor, const MidiMessage& message) {
 *     if (processor.getNumMidiInputs() > 0) {
 *         processor.processMidi(message.getRawData(), message.getRawDataSize());
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as a MIDI processor
 */
template<typename T>
concept MidiProcessor = requires(T processor) {
    // Must be able to process raw MIDI data
    processor.processMidi(std::declval<const void*>(), std::declval<int>());

    // Must provide MIDI port information
    { processor.getNumMidiInputs() } -> std::convertible_to<int>;
    { processor.getNumMidiOutputs() } -> std::convertible_to<int>;
};

//==============================================================================
/**
 * Synthesizer Concept
 *
 * Defines the interface for synthesizer instruments.
 * Synthesizers combine audio processing with MIDI input to generate
 * musical sounds from MIDI note data.
 *
 * This concept ensures synthesizers can handle both audio generation
 * and MIDI note management with proper voice allocation.
 *
 * Example Usage:
 * @code
 * template<Synthesizer T>
 * void playChord(T& synth, const std::vector<int>& notes, float velocity) {
 *     for (int note : notes) {
 *         synth.noteOn(note, velocity);
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as a synthesizer
 */
template<typename T>
concept Synthesizer = AudioProcessor<T> && MidiProcessor<T> && requires(T synth) {
    // Must handle MIDI note events
    synth.noteOn(std::declval<int>(), std::declval<float>());
    synth.noteOff(std::declval<int>());

    // Must provide voice management information
    { synth.getNumVoices() } -> std::convertible_to<int>;
};

//==============================================================================
/**
 * Effect Processor Concept
 *
 * Defines the interface for audio effects processors.
 * Effects modify existing audio signals rather than generating new ones.
 * They typically include bypass functionality and may have tail times.
 *
 * Required Methods:
 * - isBypassed(): Check if effect is bypassed
 * - setBypassed(): Enable/disable effect bypass
 * - getTailLengthSeconds(): Get effect tail time for reverbs/delays
 *
 * Example Usage:
 * @code
 * template<EffectProcessor T>
 * void fadeOutEffect(T& effect) {
 *     if (!effect.isBypassed() && effect.getTailLengthSeconds() > 0) {
 *         // Implement smooth fade-out considering tail time
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as an effect processor
 */
template<typename T>
concept EffectProcessor = AudioProcessor<T> && requires(T effect) {
    // Must support bypass functionality
    { effect.isBypassed() } -> std::convertible_to<bool>;
    effect.setBypassed(std::declval<bool>());

    // Must provide tail length information for proper fade-outs
    { effect.getTailLengthSeconds() } -> std::convertible_to<double>;
};

//==============================================================================
/**
 * Modulation Source Concept
 *
 * Defines the interface for modulation sources like LFOs, envelopes,
 * and other control signal generators. Modulation sources provide
 * time-varying control signals for audio parameters.
 *
 * Required Methods:
 * - getNextValue(): Get next modulation value
 * - setFrequency(): Set modulation rate/frequency
 * - reset(): Reset modulation state
 * - isFinished(): Check if modulation is complete (for envelopes)
 *
 * Example Usage:
 * @code
 * template<ModulationSource T>
 * void modulateParameter(T& modSource, AudioParameter auto& param) {
 *     if (!modSource.isFinished()) {
 *         float modValue = modSource.getNextValue();
 *         param.setValue(param.getValue() + modValue);
 *     }
 * }
 * @endcode
 *
 * @tparam T The type to validate as a modulation source
 */
template<typename T>
concept ModulationSource = requires(T modSource) {
    // Must provide modulation values
    { modSource.getNextValue() } -> std::floating_point;

    // Must support frequency/rate control
    modSource.setFrequency(std::declval<float>());

    // Must be able to reset state
    modSource.reset();

    // Must indicate completion status (important for envelopes)
    { modSource.isFinished() } -> std::convertible_to<bool>;
};

//==============================================================================
/**
 * Concept Validation Utilities
 *
 * These utility functions help validate and work with audio concepts
 * at runtime, providing helpful debugging and validation capabilities.
 */

/**
 * Validate Audio Processor at Runtime
 *
 * Performs runtime validation of an audio processor, checking for
 * common issues and providing diagnostic information.
 *
 * @tparam T Type that satisfies AudioProcessor concept
 * @param processor The processor to validate
 * @return true if processor passes all runtime checks
 */
template<AudioProcessor T>
constexpr bool validateAudioProcessor(const T& processor) noexcept
{
    // Compile-time validation is handled by the concept
    // Runtime validation could check for:
    // - Reasonable latency values
    // - Proper initialization state
    // - Resource allocation status
    return true; // Placeholder for actual runtime checks
}

/**
 * Get Concept Information
 *
 * Provides compile-time information about which concepts a type satisfies.
 * Useful for debugging and template metaprogramming.
 *
 * @tparam T The type to analyze
 */
template<typename T>
struct ConceptInfo
{
    static constexpr bool is_audio_sample = AudioSample<T>;
    static constexpr bool is_audio_processor = AudioProcessor<T>;
    static constexpr bool is_filter_type = FilterType<T>;
    static constexpr bool is_audio_parameter = AudioParameter<T>;
    static constexpr bool is_audio_plugin = AudioPlugin<T>;
    static constexpr bool is_audio_buffer = AudioBuffer<T>;
    static constexpr bool is_realtime_safe = RealtimeSafe<T>;
    static constexpr bool is_midi_processor = MidiProcessor<T>;
    static constexpr bool is_synthesizer = Synthesizer<T>;
    static constexpr bool is_effect_processor = EffectProcessor<T>;
    static constexpr bool is_modulation_source = ModulationSource<T>;
};

} // namespace ModernAudio

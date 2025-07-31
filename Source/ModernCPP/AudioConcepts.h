#pragma once
#include <concepts>
#include <type_traits>
#include <string>

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

// Buffer concept for type-safe buffer operations
template<typename T>
concept AudioBuffer = requires(T buffer) {
    { buffer.getNumChannels() } -> std::convertible_to<int>;
    { buffer.getNumSamples() } -> std::convertible_to<int>;
    { buffer.getWritePointer(0) } -> std::convertible_to<float*>;
    { buffer.getReadPointer(0) } -> std::convertible_to<const float*>;
};

// Real-time safe concept
template<typename T>
concept RealtimeSafe = requires(T obj) {
    // Must not allocate memory in process methods
    requires std::is_nothrow_move_constructible_v<T>;
    requires std::is_nothrow_move_assignable_v<T>;
    // Should have a process method that doesn't throw
    requires requires(T& t, float input) {
        { t.process(input) } noexcept -> AudioSample;
    };
};

// MIDI processor concept
template<typename T>
concept MidiProcessor = requires(T processor) {
    processor.processMidi(std::declval<const void*>(), std::declval<int>());
    { processor.getNumMidiInputs() } -> std::convertible_to<int>;
    { processor.getNumMidiOutputs() } -> std::convertible_to<int>;
};

// Synthesizer concept
template<typename T>
concept Synthesizer = AudioProcessor<T> && MidiProcessor<T> && requires(T synth) {
    synth.noteOn(std::declval<int>(), std::declval<float>());
    synth.noteOff(std::declval<int>());
    { synth.getNumVoices() } -> std::convertible_to<int>;
};

// Effect processor concept
template<typename T>
concept EffectProcessor = AudioProcessor<T> && requires(T effect) {
    { effect.isBypassed() } -> std::convertible_to<bool>;
    effect.setBypassed(std::declval<bool>());
    { effect.getTailLengthSeconds() } -> std::convertible_to<double>;
};

// Modulation source concept
template<typename T>
concept ModulationSource = requires(T modSource) {
    { modSource.getNextValue() } -> std::floating_point;
    modSource.setFrequency(std::declval<float>());
    modSource.reset();
    { modSource.isFinished() } -> std::convertible_to<bool>;
};

} // namespace ModernAudio

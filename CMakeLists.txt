cmake_minimum_required(VERSION 3.15)

project(JUCEAudioGenerator VERSION 0.1.0)

# Add JUCE module
add_subdirectory(JUCE)

# Plugin configuration
set(PLUGIN_NAME "JUCEAudioGenerator")
set(PLUGIN_MANUFACTURER "finitud-labs")
set(PLUGIN_DESCRIPTION "Educational sine wave generator plugin")
set(PLUGIN_CATEGORY "Generator")

# Create plugin target
juce_add_plugin(${PLUGIN_NAME}
    VERSION 0.1.0
    COMPANY_NAME "${PLUGIN_MANUFACTURER}"
    PLUGIN_MANUFACTURER_CODE "Flab"
    PLUGIN_CODE "JAGn"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "${PLUGIN_NAME}"
    DESCRIPTION "${PLUGIN_DESCRIPTION}"
    PLUGIN_CATEGORY_ID "kPluginCategGenerator"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Source files
target_sources(${PLUGIN_NAME} PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/AudioEngine.cpp
)

# Generate JUCE header
juce_generate_juce_header(${PLUGIN_NAME})

# Link JUCE modules
target_link_libraries(${PLUGIN_NAME} PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
)

# Compiler definitions
target_compile_definitions(${PLUGIN_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Keep the original standalone app for development/testing
juce_add_gui_app(TestJuce
    VERSION 0.1.0
    COMPANY_NAME "finitud-labs"
    PRODUCT_NAME "test-juce"
)

# Source files for standalone app
target_sources(TestJuce PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/AudioEngine.cpp
)

juce_generate_juce_header(TestJuce)

# JUCE modules to link for standalone app
target_link_libraries(TestJuce PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
)

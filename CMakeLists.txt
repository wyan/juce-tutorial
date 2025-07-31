cmake_minimum_required(VERSION 3.15)

project(TestJuce VERSION 0.1.0)

# Add JUCE module
add_subdirectory(JUCE)

# Create a GUI app
juce_add_gui_app(TestJuce
    VERSION 0.1.0
    COMPANY_NAME "finitud-labs"
    PRODUCT_NAME "test-juce"
)

# Source files
target_sources(TestJuce PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/AudioEngine.cpp
)

juce_generate_juce_header(TestJuce)

# JUCE modules to link
target_link_libraries(TestJuce PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
)

# ==============================================================================
# JUCE Tutorial Examples - Master CMake Configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.15)

# Project configuration
project(JUCETutorialExamples VERSION 1.0.0)

# Set C++ standard for all examples
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Global Configuration
# ==============================================================================

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Enable folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ==============================================================================
# Find JUCE Framework
# ==============================================================================

# Try to find JUCE in common locations
set(JUCE_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE"
    "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
)

set(JUCE_FOUND FALSE)
foreach(JUCE_PATH ${JUCE_PATHS})
    if(EXISTS "${JUCE_PATH}/CMakeLists.txt")
        message(STATUS "Found JUCE at: ${JUCE_PATH}")
        add_subdirectory(${JUCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
        set(JUCE_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT JUCE_FOUND)
    message(FATAL_ERROR
        "Could not find JUCE framework. Please ensure JUCE is available in one of these locations:\n"
        "  - ${CMAKE_CURRENT_SOURCE_DIR}/../JUCE\n"
        "  - ${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE\n"
        "  - ${CMAKE_CURRENT_SOURCE_DIR}/JUCE\n"
        "\nYou can download JUCE from: https://github.com/juce-framework/JUCE"
    )
endif()

# ==============================================================================
# Example Projects Configuration
# ==============================================================================

# Option to build specific examples
option(BUILD_ALL_EXAMPLES "Build all examples" ON)
option(BUILD_HELLO_JUCE "Build Hello JUCE example" ON)
option(BUILD_AUDIO_PLAYER "Build Audio Player example" ON)
option(BUILD_BASIC_SYNTHESIZER "Build Basic Synthesizer example" ON)
option(BUILD_GUI_COMPONENTS "Build GUI Components example" ON)
option(BUILD_AUDIO_EFFECTS "Build Audio Effects example" ON)
option(BUILD_MIDI_PROCESSOR "Build MIDI Processor example" ON)
option(BUILD_PLUGIN_TEMPLATE "Build Plugin Template example" ON)
option(BUILD_SPECTRUM_ANALYZER "Build Spectrum Analyzer example" ON)
option(BUILD_MODERN_CPP_AUDIO "Build Modern C++ Audio example" ON)
option(BUILD_PERFORMANCE_OPTIMIZATION "Build Performance Optimization example" ON)

# ==============================================================================
# Add Example Subdirectories
# ==============================================================================

# Function to conditionally add example subdirectories
function(add_example_if_enabled EXAMPLE_NAME OPTION_NAME DIRECTORY_NAME)
    if(BUILD_ALL_EXAMPLES OR ${OPTION_NAME})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY_NAME}/CMakeLists.txt")
            message(STATUS "Adding example: ${EXAMPLE_NAME}")
            add_subdirectory(${DIRECTORY_NAME})

            # Set folder for IDE organization
            if(TARGET ${EXAMPLE_NAME})
                set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Examples")
            endif()
        else()
            message(WARNING "Example directory not found: ${DIRECTORY_NAME}")
        endif()
    else()
        message(STATUS "Skipping example: ${EXAMPLE_NAME} (disabled)")
    endif()
endfunction()

# Add examples based on options
add_example_if_enabled("HelloJUCE" BUILD_HELLO_JUCE "01-hello-juce")
add_example_if_enabled("AudioPlayer" BUILD_AUDIO_PLAYER "02-audio-player")
add_example_if_enabled("BasicSynthesizer" BUILD_BASIC_SYNTHESIZER "03-basic-synthesizer")
add_example_if_enabled("GUIComponents" BUILD_GUI_COMPONENTS "04-gui-components")
add_example_if_enabled("AudioEffects" BUILD_AUDIO_EFFECTS "05-audio-effects")
add_example_if_enabled("MIDIProcessor" BUILD_MIDI_PROCESSOR "06-midi-processor")
add_example_if_enabled("PluginTemplate" BUILD_PLUGIN_TEMPLATE "07-plugin-template")
add_example_if_enabled("SpectrumAnalyzer" BUILD_SPECTRUM_ANALYZER "08-spectrum-analyzer")
add_example_if_enabled("ModernCppAudio" BUILD_MODERN_CPP_AUDIO "09-modern-cpp-audio")
add_example_if_enabled("PerformanceOptimization" BUILD_PERFORMANCE_OPTIMIZATION "10-performance-optimization")

# ==============================================================================
# Custom Targets for Development
# ==============================================================================

# Create a target to build all examples
if(BUILD_ALL_EXAMPLES)
    add_custom_target(build_all_examples
        COMMENT "Building all JUCE tutorial examples"
    )

    # Add dependencies for all example targets that exist
    set(EXAMPLE_TARGETS
        HelloJUCE AudioPlayer BasicSynthesizer GUIComponents AudioEffects
        MIDIProcessor PluginTemplate SpectrumAnalyzer ModernCppAudio PerformanceOptimization
    )

    foreach(TARGET_NAME ${EXAMPLE_TARGETS})
        if(TARGET ${TARGET_NAME})
            add_dependencies(build_all_examples ${TARGET_NAME})
        endif()
    endforeach()
endif()

# Create a target to clean all examples
add_custom_target(clean_all_examples
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning all JUCE tutorial examples"
)

# ==============================================================================
# Installation Configuration (Optional)
# ==============================================================================

# Install examples (uncomment if needed)
# include(GNUInstallDirs)
#
# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
#     DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
#     FILES_MATCHING
#     PATTERN "*.md"
#     PATTERN "*.cpp"
#     PATTERN "*.h"
#     PATTERN "CMakeLists.txt"
#     PATTERN "build" EXCLUDE
#     PATTERN ".git" EXCLUDE
# )

# ==============================================================================
# Development Information and Validation
# ==============================================================================

# Print configuration summary
message(STATUS "")
message(STATUS "JUCE Tutorial Examples Configuration Summary:")
message(STATUS "============================================")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "")

# Print enabled examples
message(STATUS "Enabled Examples:")
if(BUILD_ALL_EXAMPLES)
    message(STATUS "  All examples enabled")
else()
    if(BUILD_HELLO_JUCE) message(STATUS "  ✓ Hello JUCE") endif()
    if(BUILD_AUDIO_PLAYER) message(STATUS "  ✓ Audio Player") endif()
    if(BUILD_BASIC_SYNTHESIZER) message(STATUS "  ✓ Basic Synthesizer") endif()
    if(BUILD_GUI_COMPONENTS) message(STATUS "  ✓ GUI Components") endif()
    if(BUILD_AUDIO_EFFECTS) message(STATUS "  ✓ Audio Effects") endif()
    if(BUILD_MIDI_PROCESSOR) message(STATUS "  ✓ MIDI Processor") endif()
    if(BUILD_PLUGIN_TEMPLATE) message(STATUS "  ✓ Plugin Template") endif()
    if(BUILD_SPECTRUM_ANALYZER) message(STATUS "  ✓ Spectrum Analyzer") endif()
    if(BUILD_MODERN_CPP_AUDIO) message(STATUS "  ✓ Modern C++ Audio") endif()
    if(BUILD_PERFORMANCE_OPTIMIZATION) message(STATUS "  ✓ Performance Optimization") endif()
endif()
message(STATUS "")

# Validate JUCE version (if available)
if(DEFINED JUCE_VERSION)
    message(STATUS "JUCE Version: ${JUCE_VERSION}")

    # Check for minimum JUCE version
    if(JUCE_VERSION VERSION_LESS "6.0.0")
        message(WARNING "JUCE version ${JUCE_VERSION} detected. Some examples may require JUCE 6.0.0 or later.")
    endif()
endif()

# Platform-specific information
if(WIN32)
    message(STATUS "Platform: Windows")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
elseif(APPLE)
    message(STATUS "Platform: macOS")
    message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif(UNIX)
    message(STATUS "Platform: Linux")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  Build all: cmake --build . --config ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build specific: cmake --build . --target <example_name> --config ${CMAKE_BUILD_TYPE}")
message(STATUS "  Clean all: cmake --build . --target clean")
message(STATUS "")

# ==============================================================================
# Advanced Configuration for Developers
# ==============================================================================

# Enable additional warnings for development builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/W4)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
endif()

# Enable static analysis (optional)
option(ENABLE_STATIC_ANALYSIS "Enable static analysis tools" OFF)
if(ENABLE_STATIC_ANALYSIS)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
        message(STATUS "Static analysis enabled with clang-tidy")
    endif()
endif()

# Enable code coverage (optional)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled")
    endif()
endif()

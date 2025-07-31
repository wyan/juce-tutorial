cmake_minimum_required(VERSION 3.22)

project(JUCEAudioGenerator VERSION 1.0.0)

# C++20 requirement for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration options
option(BUILD_STANDALONE "Build standalone application" ON)
option(BUILD_VST3 "Build VST3 plugin" ON)
option(BUILD_AU "Build Audio Unit plugin" ON)
option(BUILD_AUV3 "Build AUv3 plugin for iOS/macOS" OFF)
option(BUILD_AAX "Build AAX plugin" OFF)
option(ENABLE_CODESIGNING "Enable code signing" OFF)
option(ENABLE_TESTING "Enable unit tests" OFF)

# Platform detection
if(APPLE)
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(PLATFORM_IOS TRUE)
        set(BUILD_AUV3 ON)  # Enable AUv3 for iOS builds
        set(BUILD_STANDALONE OFF)
        set(BUILD_VST3 OFF)
        set(BUILD_AU OFF)
    else()
        set(PLATFORM_MACOS TRUE)
    endif()
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    set(BUILD_AU OFF) # AU not available on Windows
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    set(BUILD_AU OFF) # AU not available on Linux
endif()

# JUCE setup
add_subdirectory(JUCE)

# Shared configuration
set(PLUGIN_NAME "JUCEAudioGenerator")
set(PLUGIN_MANUFACTURER "finitud-labs")
set(PLUGIN_DESCRIPTION "Professional dual-target audio generator with AUv3 support")
set(PLUGIN_CATEGORY "Generator")

# Shared source files
set(SHARED_SOURCES
    Source/AudioEngine.cpp
    Source/SharedAudioComponent.cpp
)

# Platform-specific sources
set(PLATFORM_SOURCES)
if(PLATFORM_IOS)
    list(APPEND PLATFORM_SOURCES
        Source/iOS/TouchInterface.cpp
        Source/iOS/MobileOptimizations.cpp
    )
elseif(PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        Source/macOS/NativeIntegration.cpp
    )
elseif(PLATFORM_WINDOWS)
    list(APPEND PLATFORM_SOURCES
        Source/Windows/WindowsOptimizations.cpp
    )
elseif(PLATFORM_LINUX)
    list(APPEND PLATFORM_SOURCES
        Source/Linux/LinuxOptimizations.cpp
    )
endif()

# Function to configure common target properties
function(configure_target target_name)
    target_sources(${target_name} PRIVATE ${SHARED_SOURCES} ${PLATFORM_SOURCES})

    target_link_libraries(${target_name} PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
    )

    target_compile_definitions(${target_name} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=1
    )

    # Platform-specific definitions
    if(PLATFORM_IOS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_IOS=1
            JUCE_MOBILE=1
            TARGET_AUV3=1
        )
    elseif(PLATFORM_MACOS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_MAC=1
            PLATFORM_DESKTOP=1
        )
    elseif(PLATFORM_WINDOWS)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_WINDOWS=1
            PLATFORM_DESKTOP=1
        )
    elseif(PLATFORM_LINUX)
        target_compile_definitions(${target_name} PRIVATE
            JUCE_LINUX=1
            PLATFORM_DESKTOP=1
        )
    endif()

    # C++20 features
    target_compile_features(${target_name} PRIVATE cxx_std_20)

    # Optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target_name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -flto>
        )

        target_link_options(${target_name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/LTCG>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-flto>
        )
    endif()

    # Generate JUCE header
    juce_generate_juce_header(${target_name})
endfunction()

# Plugin target (VST3, AU, AAX)
if(BUILD_VST3 OR BUILD_AU OR BUILD_AAX)
    set(PLUGIN_FORMATS)
    if(BUILD_VST3)
        list(APPEND PLUGIN_FORMATS VST3)
    endif()
    if(BUILD_AU AND APPLE)
        list(APPEND PLUGIN_FORMATS AU)
    endif()
    if(BUILD_AAX)
        list(APPEND PLUGIN_FORMATS AAX)
    endif()

    juce_add_plugin(${PLUGIN_NAME}
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JAG5"
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME "${PLUGIN_NAME}"
        DESCRIPTION "${PLUGIN_DESCRIPTION}"
        PLUGIN_CATEGORY_ID "kPluginCategGenerator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES "Generator"
        AU_MAIN_TYPE "kAudioUnitType_Generator"
        AU_SANDBOX_SAFE TRUE
    )

    target_sources(${PLUGIN_NAME} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
    )

    configure_target(${PLUGIN_NAME})

    # Plugin-specific modules
    target_link_libraries(${PLUGIN_NAME} PRIVATE
        juce::juce_audio_plugin_client
    )

    target_compile_definitions(${PLUGIN_NAME} PRIVATE
        TARGET_PLUGIN=1
    )
endif()

# AUv3 target for iOS/macOS
if(BUILD_AUV3 AND APPLE)
    juce_add_plugin(${PLUGIN_NAME}_AUv3
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PLUGIN_MANUFACTURER_CODE "Flab"
        PLUGIN_CODE "JA3V"
        FORMATS AUv3
        PRODUCT_NAME "${PLUGIN_NAME} AUv3"
        DESCRIPTION "${PLUGIN_DESCRIPTION} for iOS and macOS"
        PLUGIN_CATEGORY_ID "kAudioUnitType_Generator"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        AUV3_CATEGORY "Generator"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator.auv3"
        AU_SANDBOX_SAFE TRUE
    )

    target_sources(${PLUGIN_NAME}_AUv3 PRIVATE
        Source/PluginProcessor.cpp
        Source/AUv3/AUv3Editor.cpp
    )

    configure_target(${PLUGIN_NAME}_AUv3)

    # AUv3-specific modules
    target_link_libraries(${PLUGIN_NAME}_AUv3 PRIVATE
        juce::juce_audio_plugin_client
    )

    target_compile_definitions(${PLUGIN_NAME}_AUv3 PRIVATE
        TARGET_AUV3=1
        TARGET_MOBILE_PLUGIN=1
    )

    # AUv3-specific configuration
    if(PLATFORM_IOS)
        set_target_properties(${PLUGIN_NAME}_AUv3 PROPERTIES
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
            XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
        )
    endif()
endif()

# Standalone application
if(BUILD_STANDALONE)
    juce_add_gui_app(${PLUGIN_NAME}_Standalone
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "${PLUGIN_MANUFACTURER}"
        PRODUCT_NAME "${PLUGIN_NAME}"
        BUNDLE_ID "com.finitud-labs.juceaudiogenerator"
    )

    target_sources(${PLUGIN_NAME}_Standalone PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Standalone/StandaloneWrapper.cpp
    )

    configure_target(${PLUGIN_NAME}_Standalone)

    target_compile_definitions(${PLUGIN_NAME}_Standalone PRIVATE
        TARGET_STANDALONE=1
    )
endif()

# Keep the original test app for development
juce_add_gui_app(TestJuce
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "${PLUGIN_MANUFACTURER}"
    PRODUCT_NAME "test-juce"
)

target_sources(TestJuce PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/AudioEngine.cpp
)

configure_target(TestJuce)

target_compile_definitions(TestJuce PRIVATE
    TARGET_TEST=1
)

# Code signing configuration
if(ENABLE_CODESIGNING AND APPLE)
    include(cmake/CodeSigning.cmake)
    setup_code_signing()
endif()

# Custom build targets
add_custom_target(build_all_targets
    DEPENDS
        $<$<BOOL:${BUILD_STANDALONE}>:${PLUGIN_NAME}_Standalone>
        $<$<BOOL:${BUILD_VST3}>:${PLUGIN_NAME}>
        $<$<BOOL:${BUILD_AUV3}>:${PLUGIN_NAME}_AUv3>
        TestJuce
)

# Testing
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(Tests)
endif()

# Installation and packaging
include(GNUInstallDirs)

# Print build configuration
message(STATUS "=== JUCE Audio Generator Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Standalone: ${BUILD_STANDALONE}")
message(STATUS "VST3: ${BUILD_VST3}")
message(STATUS "AU: ${BUILD_AU}")
message(STATUS "AUv3: ${BUILD_AUV3}")
message(STATUS "AAX: ${BUILD_AAX}")
message(STATUS "Code Signing: ${ENABLE_CODESIGNING}")
message(STATUS "Testing: ${ENABLE_TESTING}")
message(STATUS "===============================================")

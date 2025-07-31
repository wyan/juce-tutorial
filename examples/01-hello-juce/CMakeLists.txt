# ==============================================================================
# Hello JUCE - Basic Application Example
# CMake Configuration File
# ==============================================================================

cmake_minimum_required(VERSION 3.15)

# Project configuration
project(HelloJUCE VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Find JUCE Framework
# ==============================================================================

# Add JUCE subdirectory (assumes JUCE is in the parent directory structure)
# Adjust this path based on your JUCE installation
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../JUCE/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
else()
    message(FATAL_ERROR "Could not find JUCE framework. Please ensure JUCE is available in the expected location.")
endif()

# ==============================================================================
# Source Files
# ==============================================================================

set(SOURCES
    Source/Main.cpp
    Source/MainComponent.h
)

# ==============================================================================
# Create the Application Target
# ==============================================================================

# Create the GUI application target
juce_add_gui_app(HelloJUCE
    # Basic app properties
    PRODUCT_NAME "Hello JUCE"
    COMPANY_NAME "JUCE Tutorial Series"
    VERSION ${PROJECT_VERSION}

    # Bundle identifier (important for macOS/iOS)
    BUNDLE_ID com.jucetutorial.hellojuce

    # Icon file (optional - you can add an icon later)
    # ICON_BIG "Resources/icon.png"
    # ICON_SMALL "Resources/icon_small.png"

    # Microphone/camera permissions (not needed for this example)
    MICROPHONE_PERMISSION_ENABLED FALSE
    CAMERA_PERMISSION_ENABLED FALSE

    # File sharing (not needed for this example)
    FILE_SHARING_ENABLED FALSE

    # Document browser (not needed for this example)
    DOCUMENT_BROWSER_ENABLED FALSE
)

# ==============================================================================
# Add Source Files to Target
# ==============================================================================

target_sources(HelloJUCE PRIVATE ${SOURCES})

# ==============================================================================
# JUCE Module Configuration
# ==============================================================================

# Link required JUCE modules
target_link_libraries(HelloJUCE
    PRIVATE
        # Core JUCE modules for GUI applications
        juce::juce_core                 # Core utilities, containers, threading
        juce::juce_data_structures      # Data structures and algorithms
        juce::juce_events               # Event handling and messaging
        juce::juce_graphics             # 2D graphics and drawing
        juce::juce_gui_basics           # Basic GUI components and windows
        juce::juce_gui_extra            # Additional GUI components

    PUBLIC
        # Recommended flags for JUCE applications
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ==============================================================================
# Compiler Definitions
# ==============================================================================

target_compile_definitions(HelloJUCE
    PRIVATE
        # JUCE application configuration
        JUCE_WEB_BROWSER=0                    # Disable web browser component
        JUCE_USE_CURL=0                       # Disable CURL for HTTP requests
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:HelloJUCE,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:HelloJUCE,JUCE_VERSION>"

        # Debug/Release specific settings
        $<$<CONFIG:Debug>:JUCE_DEBUG=1>
        $<$<CONFIG:Release>:JUCE_DEBUG=0>
)

# ==============================================================================
# Platform-Specific Configuration
# ==============================================================================

# Windows specific settings
if(WIN32)
    target_compile_definitions(HelloJUCE PRIVATE
        JUCE_WIN32=1
        JUCE_WINDOWS=1
    )
endif()

# macOS specific settings
if(APPLE)
    target_compile_definitions(HelloJUCE PRIVATE
        JUCE_MAC=1
        JUCE_MACOS=1
    )

    # Set macOS deployment target
    set_target_properties(HelloJUCE PROPERTIES
        MACOSX_DEPLOYMENT_TARGET "10.13"
    )
endif()

# Linux specific settings
if(UNIX AND NOT APPLE)
    target_compile_definitions(HelloJUCE PRIVATE
        JUCE_LINUX=1
    )

    # Link required system libraries on Linux
    target_link_libraries(HelloJUCE PRIVATE
        pthread
        dl
    )
endif()

# ==============================================================================
# IDE Configuration
# ==============================================================================

# Set up source groups for better IDE organization
source_group("Source Files" FILES ${SOURCES})

# Set the startup project for Visual Studio
if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HelloJUCE)
endif()

# ==============================================================================
# Installation (Optional)
# ==============================================================================

# Install the application (optional - uncomment if needed)
# install(TARGETS HelloJUCE
#     BUNDLE DESTINATION .
#     RUNTIME DESTINATION bin
# )

# ==============================================================================
# Custom Targets for Development
# ==============================================================================

# Add a custom target to run the application after building
if(NOT CMAKE_CROSSCOMPILING)
    add_custom_target(run_hello_juce
        COMMAND $<TARGET_FILE:HelloJUCE>
        DEPENDS HelloJUCE
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running Hello JUCE application"
        USES_TERMINAL
    )
endif()

# Add a custom target to clean and rebuild
add_custom_target(rebuild_hello_juce
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target HelloJUCE
    COMMENT "Clean and rebuild Hello JUCE"
)

# ==============================================================================
# Development Information
# ==============================================================================

# Print configuration information
message(STATUS "Hello JUCE Configuration:")
message(STATUS "  Project Name: ${PROJECT_NAME}")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  Debug Mode: Enabled")
else()
    message(STATUS "  Debug Mode: Disabled")
endif()

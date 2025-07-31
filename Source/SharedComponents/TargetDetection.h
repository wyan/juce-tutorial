#pragma once

// Platform and target detection for dual-target setup
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define PLATFORM_IOS 1
        #define PLATFORM_MOBILE 1
    #elif TARGET_OS_MAC
        #define PLATFORM_MACOS 1
        #define PLATFORM_DESKTOP 1
    #endif
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_DESKTOP 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
    #define PLATFORM_DESKTOP 1
#endif

// Target type detection
#if JucePlugin_Build_Standalone
    #define TARGET_STANDALONE 1
#elif JucePlugin_Build_VST3 || JucePlugin_Build_AU || JucePlugin_Build_AAX
    #define TARGET_PLUGIN 1
#elif JucePlugin_Build_AUv3
    #define TARGET_AUV3 1
    #define TARGET_MOBILE_PLUGIN 1
#endif

namespace TargetFeatures {
    constexpr bool hasDirectAudioAccess =
        #ifdef TARGET_STANDALONE
        true;
        #else
        false;
        #endif

    constexpr bool hasHostIntegration =
        #if defined(TARGET_PLUGIN) || defined(TARGET_AUV3)
        true;
        #else
        false;
        #endif

    constexpr bool supportsTouchInterface =
        #ifdef PLATFORM_MOBILE
        true;
        #else
        false;
        #endif

    constexpr bool supportsFileSystem =
        #ifdef TARGET_AUV3
        false; // AUv3 has limited file access
        #else
        true;
        #endif

    constexpr bool supportsMultiWindow =
        #if defined(TARGET_STANDALONE) && defined(PLATFORM_DESKTOP)
        true;
        #else
        false;
        #endif
}

// Conditional compilation helpers
#define IF_STANDALONE(code) do { if constexpr (TargetFeatures::hasDirectAudioAccess) { code } } while(0)
#define IF_PLUGIN(code) do { if constexpr (TargetFeatures::hasHostIntegration) { code } } while(0)
#define IF_MOBILE(code) do { if constexpr (TargetFeatures::supportsTouchInterface) { code } } while(0)
#define IF_DESKTOP(code) do { if constexpr (!TargetFeatures::supportsTouchInterface) { code } } while(0)

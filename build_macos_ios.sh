#!/bin/bash
# build_macos_ios.sh - Comprehensive build script for Apple platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# Configuration
BUILD_TYPE="Release"
ENABLE_CODESIGNING=false
BUILD_IOS=false
BUILD_MACOS=true
XCODE_VERSION=$(xcodebuild -version | head -n1 | awk '{print $2}')

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --codesign)
            ENABLE_CODESIGNING=true
            shift
            ;;
        --ios)
            BUILD_IOS=true
            BUILD_MACOS=false
            shift
            ;;
        --all)
            BUILD_IOS=true
            BUILD_MACOS=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug      Build in Debug mode (default: Release)"
            echo "  --codesign   Enable code signing"
            echo "  --ios        Build for iOS only"
            echo "  --all        Build for both macOS and iOS"
            echo "  --help       Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "=== JUCE Audio Generator - Apple Platform Build ==="
echo "Build Type: $BUILD_TYPE"
echo "Xcode Version: $XCODE_VERSION"
echo "Code Signing: $ENABLE_CODESIGNING"
echo "Build macOS: $BUILD_MACOS"
echo "Build iOS: $BUILD_IOS"
echo ""

# Check prerequisites
check_prerequisites() {
    echo "Checking prerequisites..."

    # Check Xcode
    if ! command -v xcodebuild &> /dev/null; then
        echo "ERROR: Xcode command line tools not found"
        echo "Install with: xcode-select --install"
        exit 1
    fi

    # Check CMake
    if ! command -v cmake &> /dev/null; then
        echo "ERROR: CMake not found"
        echo "Install with: brew install cmake"
        exit 1
    fi

    # Check JUCE submodule
    if [ ! -d "$PROJECT_ROOT/JUCE" ]; then
        echo "ERROR: JUCE submodule not found"
        echo "Initialize with: git submodule update --init --recursive"
        exit 1
    fi

    echo "  ✓ Prerequisites check passed"
    echo ""
}

# Function to build for specific platform
build_platform() {
    local platform=$1
    local build_suffix=$2
    local cmake_args=$3

    echo "Building for $platform..."

    local platform_build_dir="$BUILD_DIR/$build_suffix"
    mkdir -p "$platform_build_dir"
    cd "$platform_build_dir"

    # Configure
    echo "  Configuring CMake..."
    cmake "$PROJECT_ROOT" \
        -G "Xcode" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DENABLE_CODESIGNING="$ENABLE_CODESIGNING" \
        $cmake_args

    if [ $? -ne 0 ]; then
        echo "ERROR: CMake configuration failed for $platform"
        exit 1
    fi

    # Build
    echo "  Building..."
    cmake --build . --config "$BUILD_TYPE" --parallel $(sysctl -n hw.ncpu)

    if [ $? -ne 0 ]; then
        echo "ERROR: Build failed for $platform"
        exit 1
    fi

    echo "  ✓ $platform build completed successfully!"
    echo ""
}

# Run prerequisite checks
check_prerequisites

# Build macOS targets
if [ "$BUILD_MACOS" = true ]; then
    build_platform "macOS" "macos" "-DBUILD_AUV3=ON"
fi

# Build iOS targets
if [ "$BUILD_IOS" = true ]; then
    build_platform "iOS" "ios" "\
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphoneos \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DBUILD_STANDALONE=OFF \
        -DBUILD_VST3=OFF \
        -DBUILD_AU=OFF \
        -DBUILD_AUV3=ON"
fi

echo "=== Build Summary ==="
if [ "$BUILD_MACOS" = true ]; then
    echo "macOS builds:"
    echo "  - Standalone: $BUILD_DIR/macos/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/"
    echo "  - VST3: $BUILD_DIR/macos/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/"
    echo "  - AU: $BUILD_DIR/macos/JUCEAudioGenerator_artefacts/$BUILD_TYPE/AU/"
    echo "  - AUv3: $BUILD_DIR/macos/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/"
fi

if [ "$BUILD_IOS" = true ]; then
    echo "iOS builds:"
    echo "  - AUv3: $BUILD_DIR/ios/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/"
fi

echo ""
echo "Build completed successfully!"
echo ""
echo "Next steps:"
echo "1. Test the built applications/plugins"
echo "2. Run validation tools if available"
echo "3. Deploy using the deployment scripts"

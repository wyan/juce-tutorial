#!/bin/bash
# build_linux.sh - Linux build script for JUCE Audio Generator

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# Configuration
BUILD_TYPE="Release"
GENERATOR="Unix Makefiles"
USE_NINJA=false
PARALLEL_JOBS=$(nproc)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --ninja)
            GENERATOR="Ninja"
            USE_NINJA=true
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug      Build in Debug mode (default: Release)"
            echo "  --ninja      Use Ninja generator instead of Make"
            echo "  --jobs N     Use N parallel jobs (default: $(nproc))"
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

echo "=== JUCE Audio Generator - Linux Build ==="
echo "Build Type: $BUILD_TYPE"
echo "Generator: $GENERATOR"
echo "Parallel Jobs: $PARALLEL_JOBS"
echo ""

# Check prerequisites
check_prerequisites() {
    echo "Checking prerequisites..."

    # Check CMake
    if ! command -v cmake &> /dev/null; then
        echo "ERROR: CMake not found"
        echo "Install with: sudo apt-get install cmake (Ubuntu/Debian)"
        echo "            or: sudo dnf install cmake (Fedora)"
        echo "            or: sudo pacman -S cmake (Arch)"
        exit 1
    fi

    # Check compiler
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        echo "ERROR: No C++ compiler found"
        echo "Install with: sudo apt-get install build-essential (Ubuntu/Debian)"
        echo "            or: sudo dnf groupinstall 'Development Tools' (Fedora)"
        echo "            or: sudo pacman -S base-devel (Arch)"
        exit 1
    fi

    # Check Ninja if requested
    if [ "$USE_NINJA" = true ] && ! command -v ninja &> /dev/null; then
        echo "ERROR: Ninja not found"
        echo "Install with: sudo apt-get install ninja-build (Ubuntu/Debian)"
        echo "            or: sudo dnf install ninja-build (Fedora)"
        echo "            or: sudo pacman -S ninja (Arch)"
        exit 1
    fi

    # Check required development libraries
    echo "Checking required development libraries..."

    # Check for ALSA development headers
    if ! pkg-config --exists alsa; then
        echo "WARNING: ALSA development headers not found"
        echo "Install with: sudo apt-get install libasound2-dev (Ubuntu/Debian)"
        echo "            or: sudo dnf install alsa-lib-devel (Fedora)"
        echo "            or: sudo pacman -S alsa-lib (Arch)"
    fi

    # Check for X11 development headers
    if ! pkg-config --exists x11; then
        echo "WARNING: X11 development headers not found"
        echo "Install with: sudo apt-get install libx11-dev (Ubuntu/Debian)"
        echo "            or: sudo dnf install libX11-devel (Fedora)"
        echo "            or: sudo pacman -S libx11 (Arch)"
    fi

    # Check for additional X11 libraries
    for lib in xext xinerama xrandr xcursor; do
        if ! pkg-config --exists $lib; then
            echo "WARNING: $lib development headers not found"
        fi
    done

    # Check for FreeType
    if ! pkg-config --exists freetype2; then
        echo "WARNING: FreeType development headers not found"
        echo "Install with: sudo apt-get install libfreetype6-dev (Ubuntu/Debian)"
        echo "            or: sudo dnf install freetype-devel (Fedora)"
        echo "            or: sudo pacman -S freetype2 (Arch)"
    fi

    # Check JUCE submodule
    if [ ! -d "$PROJECT_ROOT/JUCE" ]; then
        echo "ERROR: JUCE submodule not found"
        echo "Initialize with: git submodule update --init --recursive"
        exit 1
    fi

    echo "  ✓ Prerequisites check completed"
    echo ""
}

# Install missing dependencies (Ubuntu/Debian)
install_dependencies_ubuntu() {
    echo "Installing missing dependencies for Ubuntu/Debian..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        libasound2-dev \
        libx11-dev \
        libxext-dev \
        libxinerama-dev \
        libxrandr-dev \
        libxcursor-dev \
        libfreetype6-dev \
        libfontconfig1-dev \
        libgtk-3-dev \
        libwebkit2gtk-4.0-dev

    if [ "$USE_NINJA" = true ]; then
        sudo apt-get install -y ninja-build
    fi
}

# Check if we're on Ubuntu/Debian and offer to install dependencies
if command -v apt-get &> /dev/null; then
    echo "Detected Ubuntu/Debian system."
    read -p "Install missing dependencies automatically? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies_ubuntu
    fi
fi

# Run prerequisite checks
check_prerequisites

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring CMake..."
cmake "$PROJECT_ROOT" \
    -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_VST3=ON \
    -DBUILD_AU=OFF \
    -DBUILD_AUV3=OFF \
    -DBUILD_STANDALONE=ON \
    -DENABLE_TESTING=OFF

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

# Build
echo ""
echo "Building..."
if [ "$USE_NINJA" = true ]; then
    ninja -j "$PARALLEL_JOBS"
else
    make -j "$PARALLEL_JOBS"
fi

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "=== Build Summary ==="
echo "Standalone: $BUILD_DIR/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/"
echo "VST3: $BUILD_DIR/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/"
echo "Test App: $BUILD_DIR/TestJuce_artefacts/$BUILD_TYPE/"
echo ""
echo "Build completed successfully!"
echo ""
echo "Next steps:"
echo "1. Test the built applications/plugins"
echo "2. Install plugins to system directories if needed"
echo "3. Run validation tools if available"
echo ""
echo "Plugin installation directories:"
echo "  VST3: ~/.vst3/ (user) or /usr/lib/vst3/ (system)"
echo "  Standalone: Can be run from build directory or installed to /usr/local/bin/"

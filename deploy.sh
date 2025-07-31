#!/bin/bash
# deploy.sh - Comprehensive deployment script for JUCE Audio Generator

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
DEPLOY_DIR="$PROJECT_ROOT/deploy"

# Configuration
VERSION="1.0.0"
BUILD_TYPE="Release"
PLATFORMS=("macos" "windows" "ios")
ENABLE_CODESIGNING=false
ENABLE_NOTARIZATION=false
CREATE_INSTALLERS=false
RUN_VALIDATION=true

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --platforms)
            IFS=',' read -ra PLATFORMS <<< "$2"
            shift 2
            ;;
        --codesign)
            ENABLE_CODESIGNING=true
            shift
            ;;
        --notarize)
            ENABLE_NOTARIZATION=true
            ENABLE_CODESIGNING=true  # Notarization requires code signing
            shift
            ;;
        --installers)
            CREATE_INSTALLERS=true
            shift
            ;;
        --no-validation)
            RUN_VALIDATION=false
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --version VERSION    Set version number (default: 1.0.0)"
            echo "  --platforms LIST     Comma-separated list of platforms (macos,windows,ios)"
            echo "  --codesign           Enable code signing"
            echo "  --notarize           Enable notarization (implies --codesign)"
            echo "  --installers         Create installer packages"
            echo "  --no-validation      Skip plugin validation"
            echo "  --help               Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}=== JUCE Audio Generator Deployment ===${NC}"
echo "Version: $VERSION"
echo "Platforms: ${PLATFORMS[*]}"
echo "Code Signing: $ENABLE_CODESIGNING"
echo "Notarization: $ENABLE_NOTARIZATION"
echo "Create Installers: $CREATE_INSTALLERS"
echo "Run Validation: $RUN_VALIDATION"
echo ""

# Create deployment directory
mkdir -p "$DEPLOY_DIR"

# Function to deploy for specific platform
deploy_platform() {
    local platform=$1
    echo -e "${YELLOW}Deploying for $platform...${NC}"

    local platform_build_dir="$BUILD_DIR/$platform"
    local platform_deploy_dir="$DEPLOY_DIR/$platform"

    if [ ! -d "$platform_build_dir" ]; then
        echo -e "${RED}ERROR: Build directory not found: $platform_build_dir${NC}"
        echo "Run the appropriate build script first:"
        case $platform in
            "macos"|"ios") echo "  ./build_macos_ios.sh" ;;
            "windows") echo "  ./build_windows.bat" ;;
            "linux") echo "  ./build_linux.sh" ;;
        esac
        return 1
    fi

    mkdir -p "$platform_deploy_dir"

    case $platform in
        "macos")
            deploy_macos "$platform_build_dir" "$platform_deploy_dir"
            ;;
        "windows")
            deploy_windows "$platform_build_dir" "$platform_deploy_dir"
            ;;
        "ios")
            deploy_ios "$platform_build_dir" "$platform_deploy_dir"
            ;;
        "linux")
            deploy_linux "$platform_build_dir" "$platform_deploy_dir"
            ;;
        *)
            echo -e "${RED}ERROR: Unknown platform: $platform${NC}"
            return 1
            ;;
    esac
}

deploy_macos() {
    local build_dir=$1
    local deploy_dir=$2

    echo "  Deploying macOS targets..."

    # Copy standalone app
    local standalone_path="$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.app"
    if [ -d "$standalone_path" ]; then
        cp -R "$standalone_path" "$deploy_dir/"
        echo -e "${GREEN}    ✓ Standalone app copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ Standalone app not found${NC}"
    fi

    # Copy VST3
    local vst3_path="$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3"
    if [ -d "$vst3_path" ]; then
        mkdir -p "$deploy_dir/Plugins/VST3"
        cp -R "$vst3_path" "$deploy_dir/Plugins/VST3/"
        echo -e "${GREEN}    ✓ VST3 plugin copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ VST3 plugin not found${NC}"
    fi

    # Copy AU
    local au_path="$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/AU/JUCEAudioGenerator.component"
    if [ -d "$au_path" ]; then
        mkdir -p "$deploy_dir/Plugins/AU"
        cp -R "$au_path" "$deploy_dir/Plugins/AU/"
        echo -e "${GREEN}    ✓ AU plugin copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ AU plugin not found${NC}"
    fi

    # Copy AUv3
    local auv3_path="$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE/AUv3/JUCEAudioGenerator AUv3.appex"
    if [ -d "$auv3_path" ]; then
        mkdir -p "$deploy_dir/Plugins/AUv3"
        cp -R "$auv3_path" "$deploy_dir/Plugins/AUv3/"
        echo -e "${GREEN}    ✓ AUv3 plugin copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ AUv3 plugin not found${NC}"
    fi

    # Copy documentation
    copy_documentation "$deploy_dir"

    # Create installer if requested
    if [ "$CREATE_INSTALLERS" = true ]; then
        create_macos_installer "$deploy_dir"
    fi

    # Create DMG
    create_macos_dmg "$deploy_dir"
}

deploy_windows() {
    local build_dir=$1
    local deploy_dir=$2

    echo "  Deploying Windows targets..."

    # Copy standalone app
    local standalone_path="$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator.exe"
    if [ -f "$standalone_path" ]; then
        cp "$standalone_path" "$deploy_dir/"
        echo -e "${GREEN}    ✓ Standalone app copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ Standalone app not found${NC}"
    fi

    # Copy VST3
    local vst3_path="$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3"
    if [ -d "$vst3_path" ]; then
        mkdir -p "$deploy_dir/Plugins/VST3"
        cp -R "$vst3_path" "$deploy_dir/Plugins/VST3/"
        echo -e "${GREEN}    ✓ VST3 plugin copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ VST3 plugin not found${NC}"
    fi

    # Copy documentation
    copy_documentation "$deploy_dir"

    # Create ZIP archive
    create_windows_zip "$deploy_dir"
}

deploy_ios() {
    local build_dir=$1
    local deploy_dir=$2

    echo "  Deploying iOS targets..."

    # Copy AUv3 app
    local auv3_path="$build_dir/JUCEAudioGenerator_AUv3_artefacts/$BUILD_TYPE-iphoneos/JUCEAudioGenerator AUv3.app"
    if [ -d "$auv3_path" ]; then
        cp -R "$auv3_path" "$deploy_dir/"
        echo -e "${GREEN}    ✓ AUv3 iOS app copied${NC}"

        # Create IPA
        create_ios_ipa "$deploy_dir"
    else
        echo -e "${YELLOW}    ⚠ AUv3 iOS app not found${NC}"
    fi
}

deploy_linux() {
    local build_dir=$1
    local deploy_dir=$2

    echo "  Deploying Linux targets..."

    # Copy standalone app
    local standalone_path="$build_dir/JUCEAudioGenerator_Standalone_artefacts/$BUILD_TYPE/JUCEAudioGenerator"
    if [ -f "$standalone_path" ]; then
        cp "$standalone_path" "$deploy_dir/"
        chmod +x "$deploy_dir/JUCEAudioGenerator"
        echo -e "${GREEN}    ✓ Standalone app copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ Standalone app not found${NC}"
    fi

    # Copy VST3
    local vst3_path="$build_dir/JUCEAudioGenerator_artefacts/$BUILD_TYPE/VST3/JUCEAudioGenerator.vst3"
    if [ -d "$vst3_path" ]; then
        mkdir -p "$deploy_dir/Plugins/VST3"
        cp -R "$vst3_path" "$deploy_dir/Plugins/VST3/"
        echo -e "${GREEN}    ✓ VST3 plugin copied${NC}"
    else
        echo -e "${YELLOW}    ⚠ VST3 plugin not found${NC}"
    fi

    # Copy documentation
    copy_documentation "$deploy_dir"

    # Create tar.gz archive
    create_linux_archive "$deploy_dir"
}

copy_documentation() {
    local deploy_dir=$1

    echo "    Copying documentation..."

    # Copy main documentation files
    [ -f "$PROJECT_ROOT/README.md" ] && cp "$PROJECT_ROOT/README.md" "$deploy_dir/"
    [ -f "$PROJECT_ROOT/LICENSE" ] && cp "$PROJECT_ROOT/LICENSE" "$deploy_dir/"
    [ -f "$PROJECT_ROOT/CHANGELOG.md" ] && cp "$PROJECT_ROOT/CHANGELOG.md" "$deploy_dir/"

    # Copy tutorial documentation
    if [ -d "$PROJECT_ROOT/tutorials" ]; then
        mkdir -p "$deploy_dir/Documentation"
        cp -R "$PROJECT_ROOT/tutorials" "$deploy_dir/Documentation/"
    fi

    # Create installation instructions
    create_installation_instructions "$deploy_dir"
}

create_installation_instructions() {
    local deploy_dir=$1

    cat > "$deploy_dir/INSTALLATION.md" << 'EOF'
# JUCE Audio Generator Installation Instructions

## Plugin Installation

### macOS
- **VST3**: Copy `JUCEAudioGenerator.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
- **AU**: Copy `JUCEAudioGenerator.component` to `~/Library/Audio/Plug-Ins/Components/`
- **AUv3**: Install via App Store or copy to `~/Library/Audio/Plug-Ins/Components/`

### Windows
- **VST3**: Copy `JUCEAudioGenerator.vst3` to `C:\Program Files\Common Files\VST3\`

### Linux
- **VST3**: Copy `JUCEAudioGenerator.vst3` to `~/.vst3/` or `/usr/lib/vst3/`

## Standalone Application

The standalone application can be run directly without installation.

### macOS
Double-click `JUCEAudioGenerator.app`

### Windows
Double-click `JUCEAudioGenerator.exe`

### Linux
Run `./JUCEAudioGenerator` from terminal

## System Requirements

- **macOS**: 10.13 or later
- **Windows**: Windows 10 or later
- **Linux**: Ubuntu 18.04 or equivalent
- **iOS**: iOS 12.0 or later (AUv3 only)

## Troubleshooting

If plugins don't appear in your DAW:
1. Ensure they're copied to the correct directory
2. Restart your DAW
3. Rescan plugins in your DAW's preferences
4. Check that your DAW supports the plugin format

For support, visit: https://github.com/your-repo/issues
EOF
}

create_macos_installer() {
    local deploy_dir=$1

    echo "    Creating macOS installer package..."

    # Create installer using pkgbuild
    pkgbuild --root "$deploy_dir" \
             --identifier "com.finitud-labs.juceaudiogenerator" \
             --version "$VERSION" \
             --install-location "/tmp/JUCEAudioGenerator" \
             "$deploy_dir/JUCEAudioGenerator-${VERSION}-macOS.pkg"

    echo -e "${GREEN}    ✓ macOS installer created${NC}"
}

create_macos_dmg() {
    local deploy_dir=$1

    echo "    Creating macOS DMG..."

    # Create DMG using hdiutil
    hdiutil create -volname "JUCE Audio Generator ${VERSION}" \
                   -srcfolder "$deploy_dir" \
                   -ov -format UDZO \
                   "$DEPLOY_DIR/JUCEAudioGenerator-${VERSION}-macOS.dmg"

    echo -e "${GREEN}    ✓ macOS DMG created${NC}"
}

create_windows_zip() {
    local deploy_dir=$1

    echo "    Creating Windows ZIP archive..."

    cd "$deploy_dir"
    zip -r "../JUCEAudioGenerator-${VERSION}-Windows.zip" .
    cd - > /dev/null

    echo -e "${GREEN}    ✓ Windows ZIP created${NC}"
}

create_ios_ipa() {
    local deploy_dir=$1

    echo "    Creating iOS IPA..."

    local app_path="$deploy_dir/JUCEAudioGenerator AUv3.app"
    if [ -d "$app_path" ]; then
        mkdir -p "$deploy_dir/Payload"
        cp -R "$app_path" "$deploy_dir/Payload/"
        cd "$deploy_dir"
        zip -r "JUCEAudioGenerator-${VERSION}-iOS.ipa" Payload/
        rm -rf Payload/
        cd - > /dev/null
        echo -e "${GREEN}    ✓ iOS IPA created${NC}"
    fi
}

create_linux_archive() {
    local deploy_dir=$1

    echo "    Creating Linux archive..."

    cd "$deploy_dir"
    tar -czf "../JUCEAudioGenerator-${VERSION}-Linux.tar.gz" .
    cd - > /dev/null

    echo -e "${GREEN}    ✓ Linux archive created${NC}"
}

# Run validation if requested
run_validation() {
    if [ "$RUN_VALIDATION" = true ] && [ -x "$PROJECT_ROOT/scripts/validate_plugins.sh" ]; then
        echo -e "${BLUE}Running plugin validation...${NC}"
        "$PROJECT_ROOT/scripts/validate_plugins.sh" --verbose

        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Plugin validation passed${NC}"
        else
            echo -e "${RED}✗ Plugin validation failed${NC}"
            echo "Continuing with deployment..."
        fi
        echo ""
    fi
}

# Main deployment process
echo -e "${BLUE}Starting deployment process...${NC}"
echo ""

# Run validation first
run_validation

# Deploy for each platform
for platform in "${PLATFORMS[@]}"; do
    if deploy_platform "$platform"; then
        echo -e "${GREEN}✓ $platform deployment completed${NC}"
    else
        echo -e "${RED}✗ $platform deployment failed${NC}"
    fi
    echo ""
done

# Create distribution archive
echo -e "${BLUE}Creating distribution archive...${NC}"
cd "$DEPLOY_DIR"
tar -czf "JUCEAudioGenerator-${VERSION}-All-Platforms.tar.gz" */
cd - > /dev/null

echo ""
echo -e "${BLUE}=== Deployment Summary ===${NC}"
echo "Deployment directory: $DEPLOY_DIR"
echo "Distribution archive: JUCEAudioGenerator-${VERSION}-All-Platforms.tar.gz"
echo ""

# List created files
echo "Created files:"
find "$DEPLOY_DIR" -name "*.dmg" -o -name "*.zip" -o -name "*.ipa" -o -name "*.tar.gz" -o -name "*.pkg" | while read file; do
    echo "  $(basename "$file")"
done

echo ""
echo -e "${GREEN}Deployment completed successfully!${NC}"

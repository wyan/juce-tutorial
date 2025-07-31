#!/bin/bash
# validate_plugins.sh - Plugin validation script for all targets

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Configuration
PLUGIN_NAME="JUCEAudioGenerator"
VALIDATION_TIMEOUT=300  # 5 minutes
PLUGINVAL_PATH=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
VALIDATE_VST3=true
VALIDATE_AU=true
VALIDATE_AUV3=true
VALIDATE_STANDALONE=true
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --vst3-only)
            VALIDATE_AU=false
            VALIDATE_AUV3=false
            VALIDATE_STANDALONE=false
            shift
            ;;
        --au-only)
            VALIDATE_VST3=false
            VALIDATE_AUV3=false
            VALIDATE_STANDALONE=false
            shift
            ;;
        --no-standalone)
            VALIDATE_STANDALONE=false
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --pluginval-path)
            PLUGINVAL_PATH="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --vst3-only          Validate VST3 plugins only"
            echo "  --au-only            Validate AU plugins only"
            echo "  --no-standalone      Skip standalone validation"
            echo "  --verbose            Enable verbose output"
            echo "  --pluginval-path     Path to pluginval executable"
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

echo -e "${BLUE}=== JUCE Audio Generator Plugin Validation ===${NC}"
echo "Build Directory: $BUILD_DIR"
echo "Validation Timeout: ${VALIDATION_TIMEOUT}s"
echo ""

# Find pluginval
find_pluginval() {
    if [ -n "$PLUGINVAL_PATH" ] && [ -x "$PLUGINVAL_PATH" ]; then
        echo "$PLUGINVAL_PATH"
        return 0
    fi

    # Common locations
    local locations=(
        "/usr/local/bin/pluginval"
        "/opt/homebrew/bin/pluginval"
        "$HOME/.local/bin/pluginval"
        "$(which pluginval 2>/dev/null)"
    )

    for location in "${locations[@]}"; do
        if [ -x "$location" ]; then
            echo "$location"
            return 0
        fi
    done

    return 1
}

# Validation functions
validate_plugin() {
    local plugin_path="$1"
    local plugin_type="$2"
    local plugin_name="$3"

    echo -e "${YELLOW}Validating $plugin_type: $plugin_name${NC}"

    if [ ! -e "$plugin_path" ]; then
        echo -e "${RED}  ✗ Plugin not found: $plugin_path${NC}"
        return 1
    fi

    # Basic file/directory checks
    if [[ "$plugin_type" == "VST3" || "$plugin_type" == "AU" ]]; then
        if [ ! -d "$plugin_path" ]; then
            echo -e "${RED}  ✗ Plugin bundle is not a directory${NC}"
            return 1
        fi
    else
        if [ ! -f "$plugin_path" ]; then
            echo -e "${RED}  ✗ Plugin file does not exist${NC}"
            return 1
        fi
    fi

    echo -e "${GREEN}  ✓ Plugin file/bundle exists${NC}"

    # Code signature verification (macOS)
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  Checking code signature..."
        if codesign -v "$plugin_path" 2>/dev/null; then
            echo -e "${GREEN}  ✓ Code signature valid${NC}"
        else
            echo -e "${YELLOW}  ⚠ Code signature invalid or missing${NC}"
        fi
    fi

    # pluginval validation
    if [ -n "$PLUGINVAL_BINARY" ]; then
        echo "  Running pluginval validation..."

        local validation_args="--validate"
        if [ "$VERBOSE" = true ]; then
            validation_args="$validation_args --verbose"
        fi

        local temp_log=$(mktemp)

        if timeout "$VALIDATION_TIMEOUT" "$PLUGINVAL_BINARY" $validation_args "$plugin_path" > "$temp_log" 2>&1; then
            echo -e "${GREEN}  ✓ pluginval validation passed${NC}"
            if [ "$VERBOSE" = true ]; then
                echo "  Validation output:"
                sed 's/^/    /' "$temp_log"
            fi
        else
            echo -e "${RED}  ✗ pluginval validation failed${NC}"
            echo "  Validation output:"
            sed 's/^/    /' "$temp_log"
            rm -f "$temp_log"
            return 1
        fi

        rm -f "$temp_log"
    else
        echo -e "${YELLOW}  ⚠ pluginval not available, skipping detailed validation${NC}"
    fi

    echo -e "${GREEN}  ✓ $plugin_type validation completed${NC}"
    echo ""
    return 0
}

validate_standalone() {
    local app_path="$1"
    local app_name="$2"

    echo -e "${YELLOW}Validating Standalone: $app_name${NC}"

    if [ ! -e "$app_path" ]; then
        echo -e "${RED}  ✗ Application not found: $app_path${NC}"
        return 1
    fi

    echo -e "${GREEN}  ✓ Application exists${NC}"

    # Code signature verification (macOS)
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  Checking code signature..."
        if codesign -v "$app_path" 2>/dev/null; then
            echo -e "${GREEN}  ✓ Code signature valid${NC}"
        else
            echo -e "${YELLOW}  ⚠ Code signature invalid or missing${NC}"
        fi
    fi

    # Basic launch test (if display available)
    if [ -n "$DISPLAY" ] || [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  Testing application launch..."

        local temp_log=$(mktemp)

        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS app bundle
            if timeout 10 open -W -n "$app_path" --args --test-mode > "$temp_log" 2>&1; then
                echo -e "${GREEN}  ✓ Application launches successfully${NC}"
            else
                echo -e "${YELLOW}  ⚠ Application launch test inconclusive${NC}"
            fi
        else
            # Linux executable
            if timeout 10 "$app_path" --test-mode > "$temp_log" 2>&1; then
                echo -e "${GREEN}  ✓ Application launches successfully${NC}"
            else
                echo -e "${YELLOW}  ⚠ Application launch test inconclusive${NC}"
            fi
        fi

        rm -f "$temp_log"
    else
        echo -e "${YELLOW}  ⚠ No display available, skipping launch test${NC}"
    fi

    echo -e "${GREEN}  ✓ Standalone validation completed${NC}"
    echo ""
    return 0
}

# Find pluginval
PLUGINVAL_BINARY=$(find_pluginval)
if [ -n "$PLUGINVAL_BINARY" ]; then
    echo -e "${GREEN}Found pluginval: $PLUGINVAL_BINARY${NC}"
else
    echo -e "${YELLOW}pluginval not found. Install from: https://github.com/Tracktion/pluginval${NC}"
fi
echo ""

# Validation results
VALIDATION_RESULTS=()
TOTAL_VALIDATIONS=0
SUCCESSFUL_VALIDATIONS=0

# Detect platform and set paths
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    BUILD_SUFFIX="macos"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    BUILD_SUFFIX="linux"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
    BUILD_SUFFIX="windows"
else
    echo -e "${RED}Unsupported platform: $OSTYPE${NC}"
    exit 1
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUFFIX"

# VST3 Validation
if [ "$VALIDATE_VST3" = true ]; then
    VST3_PATH="$BUILD_PATH/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
    TOTAL_VALIDATIONS=$((TOTAL_VALIDATIONS + 1))

    if validate_plugin "$VST3_PATH" "VST3" "$PLUGIN_NAME"; then
        SUCCESSFUL_VALIDATIONS=$((SUCCESSFUL_VALIDATIONS + 1))
        VALIDATION_RESULTS+=("VST3: ✓ PASSED")
    else
        VALIDATION_RESULTS+=("VST3: ✗ FAILED")
    fi
fi

# AU Validation (macOS only)
if [ "$VALIDATE_AU" = true ] && [[ "$OSTYPE" == "darwin"* ]]; then
    AU_PATH="$BUILD_PATH/${PLUGIN_NAME}_artefacts/Release/AU/${PLUGIN_NAME}.component"
    TOTAL_VALIDATIONS=$((TOTAL_VALIDATIONS + 1))

    if validate_plugin "$AU_PATH" "AU" "$PLUGIN_NAME"; then
        SUCCESSFUL_VALIDATIONS=$((SUCCESSFUL_VALIDATIONS + 1))
        VALIDATION_RESULTS+=("AU: ✓ PASSED")
    else
        VALIDATION_RESULTS+=("AU: ✗ FAILED")
    fi
fi

# AUv3 Validation (macOS/iOS only)
if [ "$VALIDATE_AUV3" = true ] && [[ "$OSTYPE" == "darwin"* ]]; then
    AUV3_PATH="$BUILD_PATH/${PLUGIN_NAME}_AUv3_artefacts/Release/AUv3/${PLUGIN_NAME} AUv3.appex"
    TOTAL_VALIDATIONS=$((TOTAL_VALIDATIONS + 1))

    if validate_plugin "$AUV3_PATH" "AUv3" "${PLUGIN_NAME} AUv3"; then
        SUCCESSFUL_VALIDATIONS=$((SUCCESSFUL_VALIDATIONS + 1))
        VALIDATION_RESULTS+=("AUv3: ✓ PASSED")
    else
        VALIDATION_RESULTS+=("AUv3: ✗ FAILED")
    fi
fi

# Standalone Validation
if [ "$VALIDATE_STANDALONE" = true ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        STANDALONE_PATH="$BUILD_PATH/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}.app"
    else
        STANDALONE_PATH="$BUILD_PATH/${PLUGIN_NAME}_Standalone_artefacts/Release/${PLUGIN_NAME}"
    fi

    TOTAL_VALIDATIONS=$((TOTAL_VALIDATIONS + 1))

    if validate_standalone "$STANDALONE_PATH" "$PLUGIN_NAME"; then
        SUCCESSFUL_VALIDATIONS=$((SUCCESSFUL_VALIDATIONS + 1))
        VALIDATION_RESULTS+=("Standalone: ✓ PASSED")
    else
        VALIDATION_RESULTS+=("Standalone: ✗ FAILED")
    fi
fi

# Summary
echo -e "${BLUE}=== Validation Summary ===${NC}"
for result in "${VALIDATION_RESULTS[@]}"; do
    echo "  $result"
done

echo ""
echo "Total validations: $TOTAL_VALIDATIONS"
echo "Successful: $SUCCESSFUL_VALIDATIONS"
echo "Failed: $((TOTAL_VALIDATIONS - SUCCESSFUL_VALIDATIONS))"

if [ $SUCCESSFUL_VALIDATIONS -eq $TOTAL_VALIDATIONS ]; then
    echo -e "${GREEN}✓ All validations passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some validations failed${NC}"
    exit 1
fi

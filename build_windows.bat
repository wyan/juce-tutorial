@echo off
REM build_windows.bat - Windows build script for JUCE Audio Generator

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%
set BUILD_DIR=%PROJECT_ROOT%build

REM Configuration
set BUILD_TYPE=Release
set ENABLE_CODESIGNING=OFF
set GENERATOR="Visual Studio 17 2022"
set ARCHITECTURE=x64

REM Parse command line arguments
:parse_args
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="--vs2019" (
    set GENERATOR="Visual Studio 16 2019"
    shift
    goto parse_args
)
if "%1"=="--vs2022" (
    set GENERATOR="Visual Studio 17 2022"
    shift
    goto parse_args
)
if "%1"=="--x86" (
    set ARCHITECTURE=Win32
    shift
    goto parse_args
)
if "%1"=="--codesign" (
    set ENABLE_CODESIGNING=ON
    shift
    goto parse_args
)
if "%1"=="--help" (
    echo Usage: %0 [OPTIONS]
    echo Options:
    echo   --debug      Build in Debug mode (default: Release)
    echo   --vs2019     Use Visual Studio 2019 generator
    echo   --vs2022     Use Visual Studio 2022 generator (default)
    echo   --x86        Build for x86 architecture (default: x64)
    echo   --codesign   Enable code signing
    echo   --help       Show this help message
    exit /b 0
)
if "%1"=="" goto start_build
echo Unknown option: %1
echo Use --help for usage information
exit /b 1

:start_build
echo === JUCE Audio Generator - Windows Build ===
echo Build Type: %BUILD_TYPE%
echo Generator: %GENERATOR%
echo Architecture: %ARCHITECTURE%
echo Code Signing: %ENABLE_CODESIGNING%
echo.

REM Check prerequisites
echo Checking prerequisites...

REM Check Visual Studio
where cl >nul 2>nul
if errorlevel 1 (
    echo ERROR: Visual Studio compiler not found
    echo Make sure Visual Studio is installed and run this from Developer Command Prompt
    exit /b 1
)

REM Check CMake
where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake not found
    echo Install CMake from https://cmake.org/download/
    exit /b 1
)

REM Check JUCE submodule
if not exist "%PROJECT_ROOT%JUCE" (
    echo ERROR: JUCE submodule not found
    echo Initialize with: git submodule update --init --recursive
    exit /b 1
)

echo   ✓ Prerequisites check passed
echo.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure
echo Configuring CMake...
cmake "%PROJECT_ROOT%" ^
    -G %GENERATOR% ^
    -A %ARCHITECTURE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_VST3=ON ^
    -DBUILD_AU=OFF ^
    -DBUILD_AUV3=OFF ^
    -DBUILD_STANDALONE=ON ^
    -DENABLE_CODESIGNING=%ENABLE_CODESIGNING%

if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo ERROR: Build failed!
    exit /b 1
)

echo.
echo === Build Summary ===
echo Standalone: %BUILD_DIR%\JUCEAudioGenerator_Standalone_artefacts\%BUILD_TYPE%\
echo VST3: %BUILD_DIR%\JUCEAudioGenerator_artefacts\%BUILD_TYPE%\VST3\
echo Test App: %BUILD_DIR%\TestJuce_artefacts\%BUILD_TYPE%\
echo.
echo Build completed successfully!
echo.
echo Next steps:
echo 1. Test the built applications/plugins
echo 2. Run validation tools if available
echo 3. Deploy using the deployment scripts
echo.

pause

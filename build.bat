@echo off
setlocal

echo Starting LED Project Build Process...

rem Check if Pico-DMX directory exists
if not exist Pico-DMX (
    echo Pico-DMX not found! Please install the Pico-DMX library first:
    echo 1. Download the Pico-DMX library
    echo 2. Extract it to the Pico-DMX directory
    exit /b 1
)

rem Check if pico-sdk is initialized
if not exist pico-sdk\pico_sdk_init.cmake (
    echo Error: pico-sdk not found! Please ensure it's properly initialized.
    exit /b 1
)

rem Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
)
cd build

rem Run cmake
echo Running CMake...
cmake -G "NMake Makefiles" .. || (
    echo CMake configuration failed!
    exit /b 1
)

rem Run nmake
echo Building project...
nmake || (
    echo Build failed!
    exit /b 1
)

rem Create output directory if it doesn't exist
if not exist ..\outputs (
    echo Creating output directory...
    mkdir ..\outputs
)

rem Copy the generated files to the output directory
echo Copying output files...
copy *.uf2 ..\outputs\ > nul 2>&1

rem Check if UF2 file exists
if exist ..\outputs\PicoLED_ProtocolBridge.uf2 (
    echo Build successful!
    echo Output files location: outputs\
    
    rem List the generated files
    echo Generated files:
    dir /b ..\outputs\
    
    echo To flash the Pico:
    echo 1. Hold the BOOTSEL button
    echo 2. Connect the Pico to USB
    echo 3. Copy PicoLED_ProtocolBridge.uf2 to the RPI-RP2 drive
) else (
    echo Build failed to generate UF2 file!
    exit /b 1
)

cd ..
endlocal 
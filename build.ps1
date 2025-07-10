# PowerShell Build Script for PicoLED Protocol Bridge
# This script builds both DMX Sender and DMX Receiver for Raspberry Pi Pico

Write-Host "=== PicoLED Protocol Bridge Build Script ===" -ForegroundColor Cyan
Write-Host ""

# Function to check if a command exists
function Test-Command($cmdname) {
    return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
}

# Check for required tools
Write-Host "Checking required tools..." -ForegroundColor Yellow

if (-not (Test-Command "cmake")) {
    Write-Host "ERROR: CMake not found. Please install CMake and add it to PATH." -ForegroundColor Red
    exit 1
}

if (-not (Test-Command "ninja") -and -not (Test-Command "make")) {
    Write-Host "WARNING: Neither Ninja nor Make found. Build may fail." -ForegroundColor Yellow
}

# Check for Pico SDK
if (-not $env:PICO_SDK_PATH) {
    Write-Host "ERROR: PICO_SDK_PATH environment variable not set." -ForegroundColor Red
    Write-Host "Please set PICO_SDK_PATH to your Pico SDK installation directory." -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $env:PICO_SDK_PATH)) {
    Write-Host "ERROR: Pico SDK path does not exist: $env:PICO_SDK_PATH" -ForegroundColor Red
    exit 1
}

Write-Host "✓ Pico SDK found at: $env:PICO_SDK_PATH" -ForegroundColor Green

# Check for Pico-DMX
if (-not (Test-Path "Pico-DMX")) {
    Write-Host "ERROR: Pico-DMX directory not found!" -ForegroundColor Red
    Write-Host "Please ensure the Pico-DMX library is in the project directory." -ForegroundColor Red
    exit 1
}

Write-Host "✓ Pico-DMX library found" -ForegroundColor Green

# Check source files
if (-not (Test-Path "src\dmx-sender.cpp")) {
    Write-Host "ERROR: dmx-sender.cpp not found in src directory!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path "src\dmx-reciver.cpp")) {
    Write-Host "ERROR: dmx-reciver.cpp not found in src directory!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path "src\PicoLED.cpp")) {
    Write-Host "ERROR: PicoLED.cpp not found in src directory!" -ForegroundColor Red
    exit 1
}

Write-Host "✓ All source files found" -ForegroundColor Green

# Create build directory
Write-Host ""
Write-Host "Creating build directory..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}
New-Item -ItemType Directory -Name "build" | Out-Null

# Change to build directory
Set-Location "build"

try {
    # Configure CMake
    Write-Host ""
    Write-Host "Configuring CMake..." -ForegroundColor Yellow
    
    if (Test-Command "ninja") {
        cmake -G "Ninja" ..
    } else {
        cmake -G "MinGW Makefiles" ..
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "✓ CMake configuration successful" -ForegroundColor Green
    
    # Build both targets
    Write-Host ""
    Write-Host "Building DMX Sender..." -ForegroundColor Yellow
    cmake --build . --target dmx_sender
    
    if ($LASTEXITCODE -ne 0) {
        throw "DMX Sender build failed"
    }
    
    Write-Host "✓ DMX Sender build successful" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "Building DMX Receiver..." -ForegroundColor Yellow
    cmake --build . --target dmx_receiver
    
    if ($LASTEXITCODE -ne 0) {
        throw "DMX Receiver build failed"
    }
    
    Write-Host "✓ DMX Receiver build successful" -ForegroundColor Green
    
    # List output files
    Write-Host ""
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output files:" -ForegroundColor Cyan
    
    if (Test-Path "dmx_sender.uf2") {
        Write-Host "  - dmx_sender.uf2 (for DMX Sender Pico)" -ForegroundColor White
    }
    
    if (Test-Path "dmx_receiver.uf2") {
        Write-Host "  - dmx_receiver.uf2 (for DMX Receiver Pico)" -ForegroundColor White
    }
    
    if (Test-Path "../outputs") {
        Write-Host ""
        Write-Host "Files also copied to outputs/ directory:" -ForegroundColor Cyan
        Get-ChildItem "../outputs" | ForEach-Object {
            Write-Host "  - $($_.Name)" -ForegroundColor White
        }
    }
    
    Write-Host ""
    Write-Host "To flash to Pico:" -ForegroundColor Yellow
    Write-Host "1. Hold BOOTSEL button while connecting Pico to USB" -ForegroundColor White
    Write-Host "2. Copy the appropriate .uf2 file to the Pico drive" -ForegroundColor White
    Write-Host "   - dmx_sender.uf2 for the DMX sending Pico" -ForegroundColor White
    Write-Host "   - dmx_receiver.uf2 for the DMX receiving Pico" -ForegroundColor White
    
} catch {
    Write-Host ""
    Write-Host "Build failed: $($_.Exception.Message)" -ForegroundColor Red
    Set-Location ".."
    exit 1
} finally {
    # Return to original directory
    Set-Location ".."
}

Write-Host ""
Write-Host "Build process completed!" -ForegroundColor Green

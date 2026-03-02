# Build Release Configuration
# This script builds the project in Release mode (optimized, no debug symbols)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building BeyondConvex - Release Mode" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (!(Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Configure CMake
Write-Host "Configuring CMake..." -ForegroundColor Yellow
Push-Location build
cmake .. -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host ""
Write-Host "Building Release..." -ForegroundColor Yellow
cmake --build . --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable: build\bin\Release\geometry_viewer.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Cyan
Write-Host "  .\build\bin\Release\geometry_viewer.exe" -ForegroundColor White
Write-Host ""

# Build Release with Debug Info Configuration
# This script builds the project in RelWithDebInfo mode (optimized with debug symbols)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building BeyondConvex - Release with Debug Info" -ForegroundColor Cyan
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
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host ""
Write-Host "Building Release with Debug Info..." -ForegroundColor Yellow
cmake --build . --config RelWithDebInfo --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable: build\bin\RelWithDebInfo\geometry_viewer.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Cyan
Write-Host "  .\build\bin\RelWithDebInfo\geometry_viewer.exe" -ForegroundColor White
Write-Host ""
Write-Host "To debug in Visual Studio:" -ForegroundColor Cyan
Write-Host "  1. Open build\BeyondConvex.sln" -ForegroundColor White
Write-Host "  2. Set geometry_viewer as startup project" -ForegroundColor White
Write-Host "  3. Select 'RelWithDebInfo' configuration" -ForegroundColor White
Write-Host "  4. Press F5 to debug" -ForegroundColor White
Write-Host ""

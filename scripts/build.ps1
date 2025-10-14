# Script pour compiler le projet rogue-publisher sur Windows

# --- Configuration
$buildDir = "build"
$buildConfig = "Release"

# --- Logique du script
Write-Host "--- Lancement du build pour Windows ---" -ForegroundColor Green

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir
}

Write-Host "--- Etape 1: Configuration du projet avec CMake... ---" -ForegroundColor Yellow
cmake -S . -B $buildDir
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERREUR: La configuration CMake a �chou�." -ForegroundColor Red
    exit 1
}

Write-Host "--- Etape 2: Compilation du projet en mode $buildConfig... ---" -ForegroundColor Yellow
cmake --build $buildDir --config $buildConfig
if ($LASTEXITCODE -eq 0) {
    Write-Host "--- Build termin� avec succ�s ! ---" -ForegroundColor Green
    Write-Host "L'ex�cutable se trouve dans: $buildDir\$buildConfig\rogue-publisher.exe"
} else {
    Write-Host "ERREUR: La compilation a �chou�." -ForegroundColor Red
    exit 1
}
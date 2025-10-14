#!/bin/bash

# Script pour compiler le projet rogue-publisher sur Linux

# --- Configuration
BUILD_DIR="build"
GENERATOR="Ninja"

# --- Logique du script
echo "--- Lancement du build pour Linux ---"
mkdir -p ${BUILD_DIR}

echo "--- Etape 1: Configuration du projet avec CMake... ---"
cmake -S . -B ${BUILD_DIR} -G "${GENERATOR}"
if [ $? -ne 0 ]; then
    echo "ERREUR: La configuration CMake a échoué."
    exit 1
fi

echo "--- Etape 2: Compilation du projet... ---"
cmake --build ${BUILD_DIR}
if [ $? -eq 0 ]; then
    echo "--- Build terminé avec succès ! ---"
    echo "L'exécutable se trouve dans: ${BUILD_DIR}/rogue-publisher"
else
    echo "ERREUR: La compilation a échoué."
    exit 1
fi
# Étape 1: Environnement de build
FROM ubuntu:22.04 AS build

# Installation des dépendances
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    libgl1-mesa-dev \
    libxkbcommon-x11-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev

# Copie sélective uniquement des fichiers nécessaires
WORKDIR /app

# Copier les fichiers de configuration CMake
COPY CMakeLists.txt ./

# Copier les répertoires source
COPY src/ ./src/
COPY include/ ./include/
COPY ui/ ./ui/
COPY resources/ ./resources/

# Compilation
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j $(nproc)

# Étape 2: Environnement d'exécution minimal
FROM ubuntu:22.04

# Créer un utilisateur non-root pour plus de sécurité
RUN useradd -m -u 1000 appuser

# Installation des dépendances d'exécution Qt
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6network6 \
    libxkbcommon-x11-0 \
    libxcb-xinerama0 && \
    rm -rf /var/lib/apt/lists/*

# Copie du binaire compilé
WORKDIR /app
COPY --from=build /app/build/rogue-publisher .

# Changer la propriété et passer à l'utilisateur non-root
RUN chown -R appuser:appuser /app
USER appuser

# Point d'entrée pour lancer l'application
ENTRYPOINT ["./rogue-publisher"]
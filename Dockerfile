# Étape 1: Environnement de build
FROM ubuntu:22.04 AS build

# Installation des dépendances
RUN apt-get update && apt-get install -y --no-install-recommends \
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
    && rm -rf /var/lib/apt/lists/*

# Créer un répertoire de travail
WORKDIR /app

# Copie sélective uniquement des fichiers nécessaires
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY include/ ./include/
COPY ui/ ./ui/
COPY resources/ ./resources/
COPY tests/ ./tests/

# Compilation
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j $(nproc)

# Étape 2: Environnement d'exécution minimal
FROM ubuntu:22.04

# Créer un utilisateur non-root
RUN groupadd -r appuser && useradd -r -g appuser -u 1000 appuser

# Installation des dépendances d'exécution Qt
RUN apt-get update && apt-get install -y --no-install-recommends \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6network6 \
    libxkbcommon-x11-0 \
    libxcb-xinerama0 \
    && rm -rf /var/lib/apt/lists/*

# Copie du binaire compilé
WORKDIR /app
COPY --from=build /app/build/rogue-publisher .

# Changer la propriété et passer à l'utilisateur non-root
RUN chown -R appuser:appuser /app
USER appuser

# Point d'entrée pour lancer l'application
ENTRYPOINT ["./rogue-publisher"]
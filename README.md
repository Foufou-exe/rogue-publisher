# Rogue Publisher

Un outil graphique simple et cross-platform (Windows/Linux) pour faciliter l'ajout de fichiers de workshop à un dépôt GitHub. Ce projet a été développé en C++ avec Qt et CMake.

!

## 🎯 Objectif de l'outil

L'objectif principal est de fournir une interface graphique simple pour :
1.  Sélectionner des fichiers et documents depuis le système de fichiers.
2.  Visualiser la liste des fichiers à téléverser.
3.  Lancer une simulation des commandes `git add`, `git commit` et `git push` pour mettre à jour un dépôt distant.

## 📂 Structure du Projet

```
rogue-publisher/
├── build/
├── include/
├── resources/
├── scripts/
├── src/
├── ui/
├── .gitignore
├── CMakeLists.txt
└── README.md
```

## 🛠️ Prérequis et Installation

Avant de compiler, assurez-vous d'avoir les outils suivants installés :

1.  **CMake** (version 3.16 ou supérieure)
2.  **Un compilateur C++17** ou plus:
    * **Windows**: Visual Studio (avec le module "Développement desktop en C++").
    * **Linux**: GCC ou Clang (`sudo apt install build-essential`).
3.  **Git**
4.  **Qt 6** (bibliothèques de développement) :
    * **Windows**: Installez via le [Qt Online Installer](https://www.qt.io/download-qt-installer).
    * **Linux (Debian/Ubuntu)**: `sudo apt install qt6-base-dev qt6-base-private-dev`

## 🚀 Compilation et Exécution

### Sous Linux

1.  Ouvrez un terminal dans le répertoire racine du projet.
2.  Rendez le script de build exécutable : `chmod +x scripts/build.sh`
3.  Lancez le script : `./scripts/build.sh`
4.  Exécutez l'application : `./build/rogue-publisher`

### Sous Windows (avec Visual Studio)

L'intégration de CMake est native et c'est la méthode la plus simple.

1.  Lancez Visual Studio.
2.  Choisissez **"Ouvrir un dossier local"** et sélectionnez le dossier `rogue-publisher`.
3.  Visual Studio configure CMake automatiquement.
4.  Sélectionnez **`rogue-publisher.exe`** comme cible de démarrage.
5.  Cliquez sur la flèche verte "Play" pour compiler et lancer.



### Sous Windows (en ligne de commande)

1.  Ouvrez **PowerShell**.
2.  Lancez le script : `.\scripts\build.ps1`
3.  Exécutez l'application : `.\build\Release\rogue-publisher.exe`

## ✍️ Comment publier sur GitHub

1.  Créez un nouveau dépôt sur GitHub.
2.  Dans le terminal, à la racine de votre projet :
    ```bash
    git init -b main
    git add .
    git commit -m "Initial commit of the rogue-publisher project"
    git remote add origin [https://github.com/VOTRE_NOM/VOTRE_REPO.git](https://github.com/VOTRE_NOM/VOTRE_REPO.git)
    git push -u origin main
    ```
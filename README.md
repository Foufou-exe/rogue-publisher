# 🎮 Rogue Publisher

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![C++](https://img.shields.io/badge/C++-17-00599C.svg?logo=c%2B%2B)
![Qt](https://img.shields.io/badge/Qt-6.9-41CD52.svg?logo=qt)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C.svg?logo=cmake)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)

**Gestionnaire Git avec Interface Graphique**

Un outil graphique moderne, simple et cross-platform pour faciliter la publication de vos fichiers sur GitHub.

[Fonctionnalités](#-fonctionnalités) •
[Installation](#-installation) •
[Utilisation](#-utilisation) •
[Compilation](#-compilation) •
[Contribution](#-contribution)

</div>

---

## 📋 Table des matières

- [À propos](#-à-propos)
- [Fonctionnalités](#-fonctionnalités)
- [Captures d'écran](#-captures-décran)
- [Prérequis](#-prérequis)
- [Installation](#-installation)
- [Compilation](#-compilation)
- [Utilisation](#-utilisation)
- [Architecture](#-architecture)
- [Contribution](#-contribution)
- [Licence](#-licence)

---

## 🎯 À propos

**Rogue Publisher** est une application desktop développée en C++ avec Qt6 qui simplifie la gestion et la publication de fichiers sur GitHub. Conçu pour être intuitif, il propose une interface graphique conviviale qui automatise les opérations Git courantes.

### Pourquoi Rogue Publisher ?

- ✅ **Simplicité** : Interface graphique intuitive, pas besoin de ligne de commande
- ✅ **Sécurité** : Gestion sécurisée des tokens GitHub avec authentification HTTPS
- ✅ **Robustesse** : Système de retry automatique et vérification de connexion
- ✅ **Cross-platform** : Fonctionne sur Windows et Linux
- ✅ **Moderne** : Développé avec les dernières versions de C++17 et Qt6

---

## ✨ Fonctionnalités

### 🚀 Fonctionnalités Principales

- **Sélection de fichiers** : Interface intuitive pour choisir les fichiers à publier
- **Gestion Git automatisée** : 
  - Initialisation de dépôt
  - Ajout de fichiers (`git add`)
  - Création de commits (`git commit`)
  - Synchronisation automatique (`git pull`)
  - Publication (`git push`)
- **Configuration persistante** : Sauvegarde de vos paramètres (dépôt, branche, utilisateur)
- **Authentification sécurisée** : Support des Personal Access Tokens GitHub
- **Logs détaillés** : Suivi en temps réel des opérations Git

### 🛡️ Fonctionnalités Avancées

- **Retry automatique** : Nouvelle tentative en cas d'échec réseau (max 3 fois)
- **Vérification de connexion** : Détection automatique de la connectivité internet et GitHub
- **Gestion des conflits** : Option de force push avec avertissement de sécurité
- **Interface responsive** : Dialogue de progression avec possibilité d'annulation
- **Messages d'erreur détaillés** : Identification précise des problèmes avec suggestions de solution

---


## 📋 Prérequis

### Outils Requis

| Outil | Version Minimale | Description |
|-------|------------------|-------------|
| **CMake** | 3.16+ | Système de build |
| **C++ Compiler** | C++17 | MSVC 2022, GCC 9+, ou Clang 10+ |
| **Qt** | 6.9+ | Framework d'interface graphique |
| **Git** | 2.x | Système de contrôle de version |

### Installation des Prérequis

#### 🪟 Windows

1. **Visual Studio 2022**
   - Téléchargez depuis [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - Cochez "Développement Desktop en C++"

2. **Qt 6**
   - Téléchargez [Qt Online Installer](https://www.qt.io/download-qt-installer)
   - Sélectionnez Qt 6.9+ avec MSVC 2022 64-bit

3. **Git**
   - Téléchargez depuis [git-scm.com](https://git-scm.com/)
   - Utilisez les options par défaut

4. **CMake** (optionnel si vous utilisez Visual Studio)
   - Téléchargez depuis [cmake.org](https://cmake.org/download/)

#### 🐧 Linux (Ubuntu/Debian)

Installer les outils de build
sudo apt update sudo apt install build-essential cmake git
Installer Qt 6
sudo apt install qt6-base-dev qt6-base-private-dev qt6-tools-dev
Vérifier les installations
cmake --version g++ --version qmake6 --version git --version



---

## 🚀 Installation

### Option 1 : Télécharger les Binaires (Recommandé)

Téléchargez la dernière version depuis la [page Releases](https://github.com/Foufou-exe/rogue-publisher/releases).

### Option 2 : Compiler depuis les Sources

Cloner le dépôt
git clone https://github.com/Foufou-exe/rogue-publisher.git cd rogue-publisher
Suivez les instructions de compilation ci-dessous

---

## 🔨 Compilation

### 🪟 Windows

#### Méthode 1 : Visual Studio (Recommandé)

1. Lancez **Visual Studio 2022**
2. `Fichier` → `Ouvrir` → `Dossier...`
3. Sélectionnez le dossier `rogue-publisher`
4. Visual Studio configure automatiquement CMake
5. Sélectionnez la configuration : `x64-Release`
6. `Générer` → `Tout générer` (ou F7)
7. L'exécutable se trouve dans `out/build/x64-Release/`

#### Méthode 2 : PowerShell

Créer le dossier de build
mkdir build cd build
Configurer avec CMake
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.9.3/msvc2022_64"
Compiler
cmake --build . --config Release
L'exécutable est dans build/Release/rogue-publisher.exe


#### Méthode 3 : Script Automatisé

.\scripts\build.ps1


### 🐧 Linux

#### Méthode 1 : Script Automatisé (Recommandé)

chmod +x scripts/build.sh ./scripts/build.sh


#### Méthode 2 : Manuelle


Créer le dossier de build
mkdir build && cd build
Configurer
cmake .. -DCMAKE_BUILD_TYPE=Release
Compiler
cmake --build . -j$(nproc)
Installer (optionnel)
sudo cmake --install .
L'exécutable est dans build/rogue-publisher
./rogue-publisher



---

## 📖 Utilisation

### Premier Lancement

1. **Lancez l'application**
   - Windows : Double-cliquez sur `rogue-publisher.exe`
   - Linux : `./rogue-publisher`

2. **Configuration initiale**
   - Cliquez sur `Actions` → `Configurer Git`
   - Renseignez :
     - 📁 **Dépôt local** : Chemin où stocker vos fichiers
     - 🌐 **URL GitHub** : `https://github.com/votre-nom/votre-depot.git`
     - 🌿 **Branche** : `main` (par défaut)
     - 👤 **Nom d'utilisateur** : Votre username GitHub

### Workflow Standard

#### 1️⃣ Ajouter des Fichiers

Cliquez sur "Ajouter des Fichiers..." → Sélectionnez vos fichiers


Les fichiers sont automatiquement copiés dans votre dépôt local.

#### 2️⃣ Publier sur GitHub

Cliquez sur "Pousser vers GitHub"


L'application va :
1. ✅ Vérifier la connexion internet et GitHub
2. ✅ Synchroniser avec le dépôt distant (pull)
3. ✅ Ajouter vos fichiers (add)
4. ✅ Créer un commit
5. ✅ Publier sur GitHub (push)

#### 3️⃣ Authentification

Lors du push, vous devrez fournir :
- **Token GitHub** : Créez-en un sur [github.com/settings/tokens](https://github.com/settings/tokens)
  - Permissions requises : `repo` (accès complet)

---

## 🏗️ Architecture

### Structure du Projet

rogue-publisher/ ├── 📂 include/           # Headers (.h) │   ├── mainwindow.h │   └── gitmanager.h ├── 📂 src/               # Sources (.cpp) │   ├── main.cpp │   ├── mainwindow.cpp │   └── gitmanager.cpp ├── 📂 ui/                # Interfaces Qt (.ui) │   └── mainwindow.ui ├── 📂 resources/         # Ressources (icônes, etc.) │   ├── resources.qrc │   ├── app.rc │   └── icon.ico ├── 📂 scripts/           # Scripts de build │   ├── build.sh │   └── build.ps1 ├── 📄 CMakeLists.txt     # Configuration CMake ├── 📄 README.md └── 📄 LICENSE.txt


### Classes Principales

#### `MainWindow`
- Interface graphique principale
- Gestion des événements utilisateur
- Affichage des logs et des dialogues

#### `GitManager`
- Exécution des commandes Git
- Gestion des erreurs et des retries
- Vérification de la connectivité réseau

---

## 🤝 Contribution

Les contributions sont les bienvenues ! Voici comment contribuer :

1. **Fork** le projet
2. **Créez** une branche : `git checkout -b feature/AmazingFeature`
3. **Committez** : `git commit -m 'Add: Amazing Feature'`
4. **Pushez** : `git push origin feature/AmazingFeature`
5. **Ouvrez** une Pull Request

### Guidelines

- Suivez le style de code existant (C++17, Qt conventions)
- Ajoutez des tests si nécessaire
- Documentez les nouvelles fonctionnalités
- Mettez à jour le README si besoin

---

## 📝 Roadmap

- [ ] Support de GitLab et Bitbucket
- [ ] Mode sombre
- [ ] Historique des commits
- [ ] Gestion des branches
- [ ] Support de SSH
- [ ] Traductions (EN, FR, ES)
- [ ] Mode CLI (ligne de commande)

---

## 🐛 Problèmes Connus

### Windows

- **Erreur "Git non disponible"** : Ajoutez Git au PATH système
- **windeployqt introuvable** : Vérifiez que Qt bin est dans le PATH

### Linux

- **Erreur de compilation Qt6** : Installez `qt6-base-private-dev`

---

## 📜 Licence

Ce projet est sous licence **MIT** - voir le fichier [LICENSE.txt](LICENSE.txt) pour plus de détails.

MIT License Copyright (c) 2025 Foufou-exe


---

## 👤 Auteur

**Foufou-exe**

- GitHub : [@Foufou-exe](https://github.com/Foufou-exe)
- Projet : [rogue-publisher](https://github.com/Foufou-exe/rogue-publisher)

---

## 🙏 Remerciements

- [Qt Framework](https://www.qt.io/) - Framework d'interface graphique
- [CMake](https://cmake.org/) - Système de build cross-platform
- [Git](https://git-scm.com/) - Système de contrôle de version


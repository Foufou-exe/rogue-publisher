# CMakeList.txt : projet CMake pour rogue-publisher, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)
project (rogue-publisher VERSION 1.0 LANGUAGES CXX)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


#if (CMAKE_VERSION VERSION_GREATER 3.12)
#  set_property(TARGET rogue-publisher PROPERTY CXX_STANDARD 20)
#endif()

# --- Configuration Standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Activer les fonctionnalités magiques de Qt avec CMake pour gérer MOC, UIC et RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Détection de Qt
# Définir le chemin d'installation de Qt6 (ajustez selon votre installation)
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/mingw_64" ${CMAKE_PREFIX_PATH})
# Cherche Qt6 et ses composants requis. CMake gère la détection sur Windows et Linux.
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# --- Définition des sources du projet
add_executable(GitHubUploader
    # Pour Windows, crée une application graphique sans console en arrière-plan
    WIN32
    # Fichiers source
    src/main.cpp
    src/mainwindow.cpp
    # Fichiers d'en-tête (inclus via CMAKE_AUTOMOC)
    include/mainwindow.h
    # Fichiers d'interface (.ui) et de ressources (.qrc)
    ui/mainwindow.ui
    resources/resources.qrc
)

# --- Liaison avec les bibliothèques Qt
target_link_libraries(GitHubUploader PRIVATE Qt6::Widgets)

# --- Instructions d'installation (Bonne pratique)
install(TARGETS GitHubUploader
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
# CMakeList.txt : projet CMake pour rogue-publisher
cmake_minimum_required(VERSION 3.16...3.31)

# Definition du projet avec metadonnees
project(rogue-publisher 
    VERSION 1.0.0
    DESCRIPTION "Application de gestion et publication Git avec interface Qt"
    LANGUAGES CXX
)

# Activez Rechargement a chaud pour les compilateurs MSVC
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# --- Configuration Standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Organisation des cibles dans l'IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Configuration Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Recherche automatique de Qt
if(NOT DEFINED CMAKE_PREFIX_PATH)
    if(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/msvc2022_64")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
)

# --- Definition des sources
set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/gitmanager.cpp
)

set(PROJECT_HEADERS
    include/mainwindow.h
    include/gitmanager.h 
)

set(PROJECT_UI
    ui/mainwindow.ui
)

set(PROJECT_RESOURCES
    resources/resources.qrc
)

# --- Creation de l'executable
add_executable(${PROJECT_NAME}
    WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    ${PROJECT_RESOURCES}
)

# --- Configuration des proprietes de la cible
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui"
    WIN32_EXECUTABLE ON
    OUTPUT_NAME "rogue-publisher"
)

# --- Repertoires d'include
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# --- Options de compilation specifiques
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W4
        /Zc:__cplusplus
        /permissive-
        /utf-8
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        /wd4127
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -pedantic
    )
endif()


# Ajouter le module Network de Qt
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Network  # Ajoutez cette ligne
)

# --- Liaison avec Qt
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# --- Deploiement automatique des DLL Qt (Windows uniquement)
if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "windeployqt trouve: ${WINDEPLOYQT_EXECUTABLE}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --verbose 1
                --no-compiler-runtime
                --no-translations
                $<$<CONFIG:Debug>:--debug>
                $<$<CONFIG:Release>:--release>
                "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Deploiement des dependances Qt avec windeployqt..."
        )
    else()
        message(WARNING "windeployqt introuvable.")
    endif()
endif()

# --- Configuration pour differentes builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RELEASE_BUILD)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()

# --- Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# --- Informations de build
message(STATUS "")
message(STATUS "Configuration de ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Systeme d'exploitation: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compilateur: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Standard C++: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Type de build: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "")
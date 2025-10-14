#include "include/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

/**
 * @brief Fonction principale, point d'entrée de l'application.
 */
int main(int argc, char* argv[]) {
    // Initialise l'application Qt
    QApplication a(argc, argv);

    // Appliquer un style moderne pour un look plus propre
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Crée et affiche la fenêtre principale
    MainWindow w;
    w.show();

    // Lance la boucle d'événements de l'application
    return a.exec();
}
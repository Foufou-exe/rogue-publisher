#include "include/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

/**
 * @brief Fonction principale, point d'entr�e de l'application.
 */
int main(int argc, char* argv[]) {
    // Initialise l'application Qt
    QApplication a(argc, argv);

    // Appliquer un style moderne pour un look plus propre
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Cr�e et affiche la fen�tre principale
    MainWindow w;
    w.show();

    // Lance la boucle d'�v�nements de l'application
    return a.exec();
}
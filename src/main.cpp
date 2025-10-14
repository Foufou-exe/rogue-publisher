#include "include/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QCommandLineParser>
#include <QDebug>

/**
 * @brief Fonction principale, point d'entree de l'application.
 * @param argc Nombre d'arguments de ligne de commande
 * @param argv Tableau des arguments de ligne de commande
 * @return Code de retour de l'application (0 = succes)
 */
int main(int argc, char* argv[]) {
    // Configure les attributs Qt avant la creation de QApplication
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Initialise l'application Qt
    QApplication app(argc, argv);

    // Configure les metadonnees de l'application
    QApplication::setApplicationName("Rogue Publisher");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Foufou-exe");
    QApplication::setOrganizationDomain("github.com/Foufou-exe");

    // Parser les arguments de ligne de commande
    QCommandLineParser parser;
    parser.setApplicationDescription("Application de gestion et publication Git");
    parser.addHelpOption();
    parser.addVersionOption();

    // Options personnalisees
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
        "Active le mode debug avec logs detailles");
    parser.addOption(debugOption);

    QCommandLineOption noStyleOption("no-style",
        "Desactive le theme Fusion");
    parser.addOption(noStyleOption);

    parser.process(app);

    // Appliquer le style moderne (sauf si desactive)
    if (!parser.isSet(noStyleOption)) {
        QApplication::setStyle(QStyleFactory::create("Fusion"));
    }

    // Mode debug
    if (parser.isSet(debugOption)) {
        qDebug() << "Mode debug active";
        qDebug() << "Version Qt:" << qVersion();
        qDebug() << "Styles disponibles:" << QStyleFactory::keys();
    }

    // Cree et affiche la fenetre principale
    MainWindow mainWindow;
    mainWindow.show();

    // Lance la boucle d'evenements de l'application
    return app.exec();
}
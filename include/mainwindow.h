#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

// Forward declaration de la classe UI générée par Qt Designer
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Fenêtre principale de l'application.
 * Gère les interactions utilisateur pour sélectionner des fichiers et simuler un push GitHub.
 */
    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        /**
         * @brief Constructeur de la fenêtre principale.
         * @param parent Widget parent (généralement nullptr).
         */
        MainWindow(QWidget* parent = nullptr);

        /**
         * @brief Destructeur.
         */
        ~MainWindow();

    private slots:
        /**
         * @brief Slot déclenché par le clic sur le bouton "Ajouter des Fichiers".
         * Ouvre une boîte de dialogue pour sélectionner des fichiers à ajouter à la liste.
         */
        void on_addFilesButton_clicked();

        /**
         * @brief Slot déclenché par le clic sur le bouton "Simuler le Commit & Push".
         * Exécute une simulation des commandes Git et affiche le résultat dans les logs.
         */
        void on_pushToGitHubButton_clicked();

        /**
         * @brief Slot déclenché par l'action "Quitter" du menu.
         */
        void on_actionQuitter_triggered();

        /**
         * @brief Slot déclenché par l'action "Ouvrir..." du menu.
         */
        void on_actionOuvrir_triggered();

    private:
        /**
         * @brief Ajoute un message de log dans la zone de texte dédiée.
         * @param message Le message à afficher.
         */
        void logMessage(const QString& message);

        Ui::MainWindow* ui; // Pointeur vers l'objet de l'interface utilisateur
};

#endif // MAINWINDOW_H
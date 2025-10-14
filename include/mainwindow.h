#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

// Forward declaration de la classe UI g�n�r�e par Qt Designer
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Fen�tre principale de l'application.
 * G�re les interactions utilisateur pour s�lectionner des fichiers et simuler un push GitHub.
 */
    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        /**
         * @brief Constructeur de la fen�tre principale.
         * @param parent Widget parent (g�n�ralement nullptr).
         */
        MainWindow(QWidget* parent = nullptr);

        /**
         * @brief Destructeur.
         */
        ~MainWindow();

    private slots:
        /**
         * @brief Slot d�clench� par le clic sur le bouton "Ajouter des Fichiers".
         * Ouvre une bo�te de dialogue pour s�lectionner des fichiers � ajouter � la liste.
         */
        void on_addFilesButton_clicked();

        /**
         * @brief Slot d�clench� par le clic sur le bouton "Simuler le Commit & Push".
         * Ex�cute une simulation des commandes Git et affiche le r�sultat dans les logs.
         */
        void on_pushToGitHubButton_clicked();

        /**
         * @brief Slot d�clench� par l'action "Quitter" du menu.
         */
        void on_actionQuitter_triggered();

        /**
         * @brief Slot d�clench� par l'action "Ouvrir..." du menu.
         */
        void on_actionOuvrir_triggered();

    private:
        /**
         * @brief Ajoute un message de log dans la zone de texte d�di�e.
         * @param message Le message � afficher.
         */
        void logMessage(const QString& message);

        Ui::MainWindow* ui; // Pointeur vers l'objet de l'interface utilisateur
};

#endif // MAINWINDOW_H
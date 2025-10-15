#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QProgressDialog>
#include "gitmanager.h"

// Forward declaration de la classe UI generee par Qt Designer
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Fenetre principale de l'application.
 * Gere les interactions utilisateur pour selectionner des fichiers et simuler un push GitHub.
 */
    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        /**
         * @brief Constructeur de la fenetre principale.
         * @param parent Widget parent (generalement nullptr).
         */
        MainWindow(QWidget* parent = nullptr);

        /**
         * @brief Destructeur.
         */
        ~MainWindow();

    protected:
        /**
         * @brief Gestion de l'evenement de fermeture de la fenetre.
         * @param event L'evenement de fermeture.
         */
        void closeEvent(QCloseEvent* event) override;

    private slots:
        /**
         * @brief Slot declenche par le clic sur le bouton "Ajouter des Fichiers".
         * Ouvre une boite de dialogue pour selectionner des fichiers a ajouter a la liste.
         */
        void on_addFilesButton_clicked();

        /**
         * @brief Slot declenche par le clic sur le bouton "Simuler le Commit & Push".
         * Execute une simulation des commandes Git et affiche le resultat dans les logs.
         */
        void on_pushToGitHubButton_clicked();

        /**
         * @brief Slot declenche par l'action "Quitter" du menu.
         */
        void on_actionQuitter_triggered();

        /**
         * @brief Slot declenche par l'action "Ouvrir..." du menu.
         */
        void on_actionOuvrir_triggered();

        /**
         * @brief Slot declenche par l'action "Configurer" du menu.
         * Ouvre une boite de dialogue pour configurer les options Git.
         */
        void on_actionConfigurer_triggered();  // Nouveau: configurer Git

        // Slots pour les signaux de GitManager
        /**
         * @brief Slot declenche par le debut d'une operation Git.
         * @param message Message d'information.
         */
        void onGitOperationStarted(const QString& message);

        /**
         * @brief Slot declenche lors d'une operation Git reussie.
         * @param message Message deSucces.
         */
        void onGitOperationSuccess(const QString& message);

        /**
         * @brief Slot declenche en cas d'echec d'une operation Git.
         * @param error Message d'erreur.
         */
        void onGitOperationFailed(const QString& error, GitError errorCode);

        /**
         * @brief Slot declenche quand une operation Git est annulee.
         */
        void onGitOperationCancelled();

        // Nouveaux slots pour gestion reseau
        void onRetryAttempt(int attempt, int maxAttempts);
        void onConnectionCheckStarted();
        void onConnectionCheckCompleted(bool success);

    private:
        /**
         * @brief Ajoute un message de log dans la zone de texte dediee.
         * @param message Le message a afficher.
         */
        void logMessage(const QString& message);

        /**
         * @brief Ajoute un message d'erreur dans la zone de texte dediee.
         * @param message Le message d'erreur a afficher.
         */
        void logError(const QString& message);

        /**
         * @brief Ajoute un message de succes dans la zone de texte dediee.
         * @param message Le message de succes a afficher.
         */
        void logSuccess(const QString& message);

        /**
         * @brief Charge les parametres de configuration depuis un fichier.
         */
        void loadSettings();

        /**
         * @brief Sauvegarde les parametres de configuration dans un fichier.
         */
        void saveSettings();

        /**
         * @brief Valide la configuration Git actuelle.
         * @return true si la configuration est valide, false sinon.
         */
        bool validateGitConfig();

        /**
         * @brief Confirme une action critique par une boite de dialogue.
         * @param title Le titre de la boite de dialogue.
         * @param message Le message contenant les details de l'action.
         * @return true si l'utilisateur confirme, false sinon.
         */
        bool confirmAction(const QString& title, const QString& message);

        /**
         * @brief Affiche une boite de dialogue de progression.
         * @param message Le message a afficher dans la boite de dialogue.
         */
        void showProgressDialog(const QString& message);

        /**
         * @brief Cache la boite de dialogue de progression.
         */
        void hideProgressDialog();

        /**
         * @brief Obtient un message d'erreur detaille en fonction du code d'erreur.
         * @param errorCode Le code de l'erreur Git.
         * @param details Informations supplementaires sur l'erreur.
         * @return Le message d'erreur formate.
         */
        QString getErrorMessage(GitError errorCode, const QString& details);

        Ui::MainWindow* ui; // Pointeur vers l'objet de l'interface utilisateur
        GitManager* m_gitManager; // Pointeur vers le gestionnaire Git
        QProgressDialog* m_progressDialog; // Boite de dialogue de progression

        // Configuration Git
        QString m_repositoryPath;
        QString m_remoteUrl;
        QString m_branch;
        QString m_githubUsername;
        QString m_githubToken;  // NOUVEAU
        
        bool m_operationInProgress; // Indique si une operation Git est en cours
};

#endif // MAINWINDOW_H
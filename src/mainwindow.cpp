#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QSettings>
#include <QInputDialog>
#include <QLineEdit>
#include <QCloseEvent>
#include <QProgressDialog>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_gitManager(new GitManager(this))
    , m_progressDialog(nullptr)
    , m_branch("main")
    , m_operationInProgress(false) {

    ui->setupUi(this);

    setWindowTitle("Rogue Publisher - Gestionnaire Git");

    // Connecter les signaux de GitManager
    connect(m_gitManager, &GitManager::operationStarted,
        this, &MainWindow::onGitOperationStarted);
    connect(m_gitManager, &GitManager::operationSuccess,
        this, &MainWindow::onGitOperationSuccess);
    connect(m_gitManager, &GitManager::operationFailed,
        this, &MainWindow::onGitOperationFailed);
    connect(m_gitManager, &GitManager::operationCancelled,
        this, &MainWindow::onGitOperationCancelled);

    // NOUVEAU: Connecter les signaux de retry et de connexion
    connect(m_gitManager, &GitManager::retryAttempt,
        this, &MainWindow::onRetryAttempt);
    connect(m_gitManager, &GitManager::connectionCheckStarted,
        this, &MainWindow::onConnectionCheckStarted);
    connect(m_gitManager, &GitManager::connectionCheckCompleted,
        this, &MainWindow::onConnectionCheckCompleted);
    
    // Charger la configuration
    loadSettings();
    
    // Verifier Git
    if (!m_gitManager->isGitAvailable()) {
        QMessageBox::critical(this, "Git non disponible",
            "Git n'est pas installe ou inaccessible.\n\n"
            "Veuillez installer Git depuis:\n"
            "https://git-scm.com/\n\n"
            "L'application va se fermer.");
        logError("FATAL: Git n'est pas disponible");
        QTimer::singleShot(100, qApp, &QApplication::quit);
    } else {
        logSuccess("Git detecte: " + m_gitManager->lastOutput());
        logMessage("Bienvenue sur Rogue Publisher !");
        
        if (!m_repositoryPath.isEmpty()) {
            logMessage("Configuration chargee: " + m_repositoryPath);
            
            // NOUVEAU: Effectuer un pull automatique au démarrage si le dépôt est configuré
            if (m_gitManager->isGitRepository(m_repositoryPath) && !m_remoteUrl.isEmpty()) {
                logMessage("Synchronisation avec le depot distant au demarrage...");
                
                // Demander le token si nécessaire pour le pull initial (optionnel)
                // Ici on fait un pull simple sans authentification
                // Si votre dépôt nécessite une authentification, décommentez les lignes ci-dessous
                /*
                bool ok;
                QString token = QInputDialog::getText(this,
                    "Synchronisation initiale",
                    "Token GitHub (optionnel pour les depots publics):",
                    QLineEdit::Password,
                    QString(),
                    &ok);
                */
                
                // Pull sans authentification (pour dépôts publics)
                if (m_gitManager->pull(m_repositoryPath, m_branch, QString(), QString())) {
                    logSuccess("Synchronisation initiale terminee");
                } else {
                    logMessage("Synchronisation initiale ignoree (depot peut-etre vide ou prive)");
                }
            }
        } else {
            logMessage("Configurez votre depot via: Actions > Configurer Git");
        }
    }
    
    // Message d'aide dans les logs au lieu de la barre d'état
    logMessage("Workflow: 1) Ajouter fichiers → 2) Push sur GitHub");
}

MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_operationInProgress) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Operation en cours",
            "Une operation Git est en cours.\n"
            "Voulez-vous vraiment quitter et annuler l'operation ?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            m_gitManager->cancelOperation();
            logMessage("Application fernee - Operation annulee");
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        if (ui->fileListWidget->count() > 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Fichiers non pousses",
                "Vous avez des fichiers non pousses.\n"
                "Voulez-vous vraiment quitter ?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                logMessage("Application fernee");
                event->accept();
            } else {
                event->ignore();
            }
        } else {
            event->accept();
        }
    }
}

void MainWindow::loadSettings() {
    QSettings settings("Foufou-exe", "RoguePublisher");
    
    m_repositoryPath = settings.value("git/repositoryPath", "").toString();
    m_remoteUrl = settings.value("git/remoteUrl", "").toString();
    m_branch = settings.value("git/branch", "main").toString();
    m_githubUsername = settings.value("github/username", "").toString();
    
    // Restaurer la geometrie de la fenetre
    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/state").toByteArray());
}

void MainWindow::saveSettings() {
    QSettings settings("Foufou-exe", "RoguePublisher");
    
    settings.setValue("git/repositoryPath", m_repositoryPath);
    settings.setValue("git/remoteUrl", m_remoteUrl);
    settings.setValue("git/branch", m_branch);
    settings.setValue("github/username", m_githubUsername);
    
    // Sauvegarder la geometrie de la fenetre
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
}

bool MainWindow::validateGitConfig() {
    if (m_repositoryPath.isEmpty()) {
        QMessageBox::warning(this, "Configuration requise",
            "Le chemin du depot local n'est pas configure.\n\n"
            "Veuillez configurer le depot Git via:\n"
            "Menu Actions > Configurer Git");
        return false;
    }
    
    if (m_remoteUrl.isEmpty()) {
        QMessageBox::warning(this, "Configuration requise",
            "L'URL du depot distant n'est pas configuree.\n\n"
            "Veuillez configurer le depot Git via:\n"
            "Menu Actions > Configurer Git");
        return false;
    }
    
    return true;
}

bool MainWindow::confirmAction(const QString& title, const QString& message) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, title, message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    return reply == QMessageBox::Yes;
}

void MainWindow::showProgressDialog(const QString& message) {
    if (!m_progressDialog) {
        m_progressDialog = new QProgressDialog(this);
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setMinimumDuration(0);
        m_progressDialog->setAutoClose(false);
        m_progressDialog->setAutoReset(false);
        m_progressDialog->setRange(0, 0); // Indeterminate
        
        connect(m_progressDialog, &QProgressDialog::canceled, this, [this]() {
            if (confirmAction("Annuler l'operation",
                            "Voulez-vous vraiment annuler l'operation en cours ?")) {
                m_gitManager->cancelOperation();
            } else {
                m_progressDialog->reset();
                m_progressDialog->show();
            }
        });
    }
    
    m_progressDialog->setLabelText(message);
    m_progressDialog->show();
}

void MainWindow::hideProgressDialog() {
    if (m_progressDialog) {
        m_progressDialog->hide();
        m_progressDialog->reset();
    }
}

QString MainWindow::getErrorMessage(GitError errorCode, const QString& details) {
    QString baseMessage;
    
    switch (errorCode) {
        case GitError::GitNotInstalled:
            baseMessage = "Git n'est pas installe.\n"
                         "Installez-le depuis: https://git-scm.com/";
            break;
        case GitError::InvalidRepository:
            baseMessage = "Le repertoire specifie n'est pas un depot Git valide.";
            break;
        case GitError::RemoteNotFound:
            baseMessage = "Le depot distant est introuvable.\n"
                         "Verifiez l'URL du depot.";
            break;
        case GitError::AuthenticationFailed:
            baseMessage = "Echec d'authentification.\n"
                         "Verifiez votre nom d'utilisateur et votre token.";
            break;
        case GitError::NetworkError:
            baseMessage = "Erreur reseau.\n"
                         "Verifiez votre connexion internet.";
            break;
        case GitError::FileNotFound:
            baseMessage = "Fichier introuvable.";
            break;
        case GitError::NothingToCommit:
            baseMessage = "Aucune modification a commiter.";
            break;
        case GitError::Timeout:
            baseMessage = "L'operation a pris trop de temps.";
            break;
        case GitError::UserCancelled:
            baseMessage = "Operation annulee par l'utilisateur.";
            break;
        default:
            baseMessage = "Erreur inconnue.";
    }
    
    if (!details.isEmpty()) {
        return baseMessage + "\n\nDetails: " + details;
    }
    return baseMessage;
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logOutput->append(QString("<span style='color: gray;'>[%1]</span> %2")
                         .arg(timestamp, message));
}

void MainWindow::logError(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logOutput->append(QString("<span style='color: gray;'>[%1]</span> "
                                 "<span style='color: red; font-weight: bold;'>✗ ERREUR:</span> %2")
                         .arg(timestamp, message));
}

void MainWindow::logSuccess(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logOutput->append(QString("<span style='color: gray;'>[%1]</span> "
                                 "<span style='color: green; font-weight: bold;'>✓ </span> %2")
                         .arg(timestamp, message));
}

void MainWindow::on_addFilesButton_clicked() {
    if (m_operationInProgress) {
        QMessageBox::warning(this, "Operation en cours",
                           "Une operation Git est deja en cours.");
        return;
    }
    
    // Verifier que le depot est configure
    if (m_repositoryPath.isEmpty()) {
        QMessageBox::warning(this, "Configuration requise",
            "Veuillez d'abord configurer le chemin du depot local.\n\n"
            "Menu Actions > Configurer Git");
        return;
    }
    
    // Creer le depot s'il n'existe pas
    QDir repoDir(m_repositoryPath);
    if (!repoDir.exists()) {
        if (!repoDir.mkpath(m_repositoryPath)) {
            QMessageBox::critical(this, "Erreur",
                "Impossible de creer le repertoire:\n" + m_repositoryPath);
            return;
        }
    }
    
    // NOUVEAU: Choix entre fichiers ou dossiers
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Type de selection",
        "Que voulez-vous ajouter ?",
        QMessageBox::StandardButton::Open | QMessageBox::StandardButton::Open | QMessageBox::StandardButton::Cancel,
        QMessageBox::StandardButton::Cancel
    );
    
    // Utiliser un dialogue personnalisé
    QMessageBox choiceBox(this);
    choiceBox.setWindowTitle("Type de selection");
    choiceBox.setText("Que voulez-vous ajouter ?");
    QPushButton* filesBtn = choiceBox.addButton("Fichiers", QMessageBox::ActionRole);
    QPushButton* folderBtn = choiceBox.addButton("Dossier/Projet", QMessageBox::ActionRole);
    QPushButton* cancelBtn = choiceBox.addButton("Annuler", QMessageBox::RejectRole);
    
    choiceBox.exec();
    
    QStringList paths;
    
    if (choiceBox.clickedButton() == filesBtn) {
        // Selection de fichiers
        paths = QFileDialog::getOpenFileNames(
            this,
            "Selectionner des fichiers",
            QDir::homePath(),
            "Tous les fichiers (*.*)"
        );
    } 
    else if (choiceBox.clickedButton() == folderBtn) {
        // Selection de dossier
        QString folder = QFileDialog::getExistingDirectory(
            this,
            "Selectionner un dossier",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        
        if (!folder.isEmpty()) {
            paths << folder;
        }
    } 
    else {
        logMessage("Selection annulee.");
        return;
    }
    
    if (paths.isEmpty()) {
        logMessage("Aucun element selectionne.");
        return;
    }
    
    // Copier les fichiers/dossiers dans le depot
    if (!m_gitManager->copyProjectRecursively(m_repositoryPath, paths)) {
        return;
    }
    
    // Ajouter a l'affichage
    for (const QString& path : paths) {
        QFileInfo info(path);
        QString displayName = info.fileName();
        
        if (info.isDir()) {
            displayName += " (dossier)";
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
        
        if (info.isDir()) {
            item->setForeground(QColor(0, 100, 200)); // Bleu pour les dossiers
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        } else {
            item->setForeground(QColor(0, 150, 0)); // Vert pour les fichiers
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
        
        ui->fileListWidget->addItem(item);
    }
    
    logSuccess(QString("%1 element(s) ajoute(s) au depot").arg(paths.count()));
}

void MainWindow::on_pushToGitHubButton_clicked() {
    if (m_operationInProgress) {
        QMessageBox::warning(this, "Operation en cours",
                           "Une operation Git est deja en cours.\n"
                           "Veuillez patienter ou annuler l'operation actuelle.");
        return;
    }
    
    if (ui->fileListWidget->count() == 0) {
        QMessageBox::warning(this, "Aucun fichier",
            "Veuillez ajouter des fichiers avant de continuer.\n\n"
            "Utilisez le bouton 'Ajouter des Fichiers...'");
        return;
    }
    
    if (!validateGitConfig()) {
        return;
    }
    
    // Confirmation avant push
    if (!confirmAction("Confirmer le push",
                      QString("Vous allez pousser %1 fichier(s) sur:\n"
                             "%2\n\n"
                             "Voulez-vous continuer ?")
                      .arg(ui->fileListWidget->count())
                      .arg(m_remoteUrl))) {
        logMessage("Push annule par l'utilisateur.");
        return;
    }
    
    // Demander le token GitHub
    bool ok;
    QString token = QInputDialog::getText(this,
        "Authentification GitHub",
        "Entrez votre token d'acces personnel GitHub:\n\n"
        "Vous pouvez en creer un sur:\n"
        "https://github.com/settings/tokens\n\n"
        "Permissions requises: repo (acces complet)",
        QLineEdit::Password,
        QString(),
        &ok);
    
    if (!ok || token.isEmpty()) {
        logMessage("Push annule: aucun token fourni.");
        return;
    }
    
    // Demander le message de commit
    QString commitMessage = QInputDialog::getText(this,
        "Message de commit",
        "Entrez le message du commit:",
        QLineEdit::Normal,
        "Commit depuis Rogue Publisher",
        &ok);
    
    if (!ok) {
        logMessage("Push annule: pas de message de commit.");
        return;
    }
    
    if (commitMessage.isEmpty()) {
        commitMessage = "Commit depuis Rogue Publisher";
    }
    
    m_operationInProgress = true;
    logMessage("=== DEBUT DES OPERATIONS GIT ===");
    
    // 1. Verifier/Initialiser le depot
    if (!m_gitManager->isGitRepository(m_repositoryPath)) {
        if (!confirmAction("Initialiser le depot",
                          "Le repertoire n'est pas un depot Git.\n"
                          "Voulez-vous l'initialiser maintenant ?")) {
            logMessage("Initialisation annulee.");
            m_operationInProgress = false;
            return;
        }
        
        if (!m_gitManager->initRepository(m_repositoryPath)) {
            m_operationInProgress = false;
            return;
        }
    }
    
    // 2. Configurer le remote
    if (!m_gitManager->setRemoteUrl(m_repositoryPath, m_remoteUrl)) {
        m_operationInProgress = false;
        return;
    }
    
    // 3. Ajouter TOUS les fichiers (récursif)
    if (!m_gitManager->addAllFiles(m_repositoryPath)) {
        m_operationInProgress = false;
        return;
    }
    
    // 4. Commit
    if (!m_gitManager->commit(m_repositoryPath, commitMessage)) {
        m_operationInProgress = false;
        return;
    }
    
    // 5. Push
    if (!m_gitManager->push(m_repositoryPath, m_branch, m_githubUsername, token)) {
        m_operationInProgress = false;
        return;
    }
    
    m_operationInProgress = false;
    logSuccess("=== OPERATIONS GIT TERMINEES AVEC SUCCES ===");
    
    QMessageBox::information(this, "Succes",
        "Le projet a ete pousse sur GitHub avec succes !\n\n"
        "Depot: " + m_remoteUrl + "\n"
        "Branche: " + m_branch);
    
    // Nettoyer la liste
    ui->fileListWidget->clear();
}

void MainWindow::on_actionConfigurer_triggered() {
    if (m_operationInProgress) {
        QMessageBox::warning(this, "Operation en cours",
                           "Impossible de modifier la configuration pendant une operation Git.");
        return;
    }
    
    bool modified = false;
    bool ok;
    
    // Configuration du chemin local
    QString repoPath = QFileDialog::getExistingDirectory(
        this,
        "Selectionnez le repertoire du depot local",
        m_repositoryPath.isEmpty() ? QDir::homePath() : m_repositoryPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!repoPath.isEmpty() && repoPath != m_repositoryPath) {
        m_repositoryPath = repoPath;
        modified = true;
    }
    
    // Configuration de l'URL distante
    QString remoteUrl = QInputDialog::getText(this,
        "URL du depot distant",
        "Entrez l'URL du depot GitHub:\n"
        "(ex: https://github.com/username/repo.git)",
        QLineEdit::Normal,
        m_remoteUrl,
        &ok);
    
    if (ok && !remoteUrl.isEmpty() && remoteUrl != m_remoteUrl) {
        // Validation basique de l'URL
        if (!remoteUrl.startsWith("http://") && !remoteUrl.startsWith("https://") &&
            !remoteUrl.startsWith("git@")) {
            QMessageBox::warning(this, "URL invalide",
                               "L'URL doit commencer par https://, http:// ou git@");
        } else {
            m_remoteUrl = remoteUrl;
            modified = true;
        }
    }
    
    // Configuration de la branche
    QString branch = QInputDialog::getText(this,
        "Branche",
        "Entrez le nom de la branche:",
        QLineEdit::Normal,
        m_branch,
        &ok);
    
    if (ok && !branch.isEmpty() && branch != m_branch) {
        m_branch = branch;
        modified = true;
    }
    
    // Configuration du nom d'utilisateur
    QString username = QInputDialog::getText(this,
        "Nom d'utilisateur GitHub",
        "Entrez votre nom d'utilisateur GitHub:",
        QLineEdit::Normal,
        m_githubUsername,
        &ok);
    
    if (ok && !username.isEmpty() && username != m_githubUsername) {
        m_githubUsername = username;
        modified = true;
    }
    
    if (modified) {
        saveSettings();
        logSuccess("Configuration mise a jour");
        
        QMessageBox::information(this, "Configuration enregistree",
            QString("Configuration enregistree:\n\n"
                    "Depot local: %1\n"
                    "Depot distant: %2\n"
                    "Branche: %3\n"
                    "Utilisateur: %4")
                .arg(m_repositoryPath, m_remoteUrl, m_branch, m_githubUsername));
    } else {
        logMessage("Configuration annulee - Aucune modification.");
    }
}

void MainWindow::on_actionOuvrir_triggered() {
    on_addFilesButton_clicked();
}

void MainWindow::on_actionQuitter_triggered() {
    close();
}

void MainWindow::onGitOperationStarted(const QString& message) {
    logMessage("[GIT] " + message);
    showProgressDialog(message);
}

void MainWindow::onGitOperationSuccess(const QString& message) {
    logSuccess("[GIT] " + message);
    hideProgressDialog();
}

void MainWindow::onGitOperationFailed(const QString& error, GitError errorCode) {
    QString fullMessage = getErrorMessage(errorCode, error);
    logError("[GIT] " + fullMessage);
    hideProgressDialog();
    
    m_operationInProgress = false;
    
    // Afficher un message d'erreur detaille
    QMessageBox::critical(this, "Erreur Git", fullMessage);
}

void MainWindow::onGitOperationCancelled() {
    logMessage("[GIT] Operation annulee par l'utilisateur");
    hideProgressDialog();
    m_operationInProgress = false;
    
    QMessageBox::information(this, "Operation annulee",
                           "L'operation Git a ete annulee.");
}

void MainWindow::onRetryAttempt(int attempt, int maxAttempts) {
    QString message = QString("Nouvelle tentative %1/%2...").arg(attempt).arg(maxAttempts);
    logMessage("[GIT] " + message);
    
    if (m_progressDialog) {
        m_progressDialog->setLabelText(message);
    }
}

void MainWindow::onConnectionCheckStarted() {
    logMessage("[RESEAU] Verification de la connexion...");
}

void MainWindow::onConnectionCheckCompleted(bool success) {
    if (success) {
        logSuccess("[RESEAU] Connexion OK");
    } else {
        logError("[RESEAU] Aucune connexion detectee");
    }
}
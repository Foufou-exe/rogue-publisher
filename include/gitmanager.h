#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief Enumeration des codes d'erreur Git
 */
enum class GitError {
    None = 0,
    GitNotInstalled,
    InvalidRepository,
    RemoteNotFound,
    AuthenticationFailed,
    NetworkError,
    FileNotFound,
    NothingToCommit,
    Timeout,
    ProcessFailed,
    UserCancelled,
    ConnectionRefused,
    SSLError,
    ProxyError,
    UnknownError
};

/**
 * @class GitManager
 * @brief Gere les operations Git avec gestion complete des erreurs
 */
class GitManager : public QObject {
    Q_OBJECT

public:
    explicit GitManager(QObject* parent = nullptr);
    ~GitManager();

    bool isGitAvailable();
    bool isGitRepository(const QString& repoPath);
    bool initRepository(const QString& repoPath);
    bool setRemoteUrl(const QString& repoPath, const QString& remoteUrl);
    bool copyAndAddFiles(const QString& repoPath, const QStringList& files,
                         const QString& subdir = QString());
    bool addFiles(const QString& repoPath, const QStringList& files);
    
    /**
     * @brief Copie recursivement des dossiers et fichiers dans le depot
     * @param repoPath Chemin du depot
     * @param paths Liste de chemins (fichiers ou dossiers)
     * @param preserveStructure Si true, preserve la structure des dossiers
     * @return true si succes
     */
    bool copyProjectRecursively(const QString& repoPath, const QStringList& paths, 
                                 bool preserveStructure = true);
    
    /**
     * @brief Ajoute recursivement tous les fichiers d'un depot a Git
     * @param repoPath Chemin du depot
     * @return true si succes
     */
    bool addAllFiles(const QString& repoPath);
    
    bool commit(const QString& repoPath, const QString& message);

    /**
     * @brief Pousse les commits avec retry automatique
     * @param repoPath Chemin du depot
     * @param branch Branche cible
     * @param username Nom d'utilisateur GitHub
     * @param token Token d'acces personnel GitHub
     * @param maxRetries Nombre maximum de tentatives (defaut: 3)
     * @return true si succes
     */
    bool push(const QString& repoPath, const QString& branch = "main",
              const QString& username = QString(), const QString& token = QString(),
              int maxRetries = 3);

    /**
     * @brief Tire les updates d'un depot distant
     * @param repoPath Chemin du depot
     * @param branch Branche a mettre a jour
     * @param username Nom d'utilisateur GitHub
     * @param token Token d'acces personnel GitHub
     * @return true si succes
     */
    bool pull(const QString& repoPath, const QString& branch,
              const QString& username, const QString& token);

    /**
     * @brief Tire les updates avec rebase d'un depot distant
     * @param repoPath Chemin du depot
     * @param branch Branche a mettre a jour
     * @param username Nom d'utilisateur GitHub
     * @param token Token d'acces personnel GitHub
     * @return true si succes
     */
    bool pullRebase(const QString& repoPath, const QString& branch,
                    const QString& username, const QString& token);

    /**
     * @brief Verifie l'etat du depot distant
     * @param repoPath Chemin du depot
     * @param branch Branche a verifier
     * @return true si a jour
     */
    bool checkRemoteStatus(const QString& repoPath, const QString& branch);

    /**
     * @brief Verifie la connexion internet
     * @return true si connecte
     */
    bool checkInternetConnection();

    /**
     * @brief Verifie l'accessibilite de GitHub
     * @param timeout Timeout en ms (defaut: 5000)
     * @return true si GitHub est accessible
     */
    bool checkGitHubConnectivity(int timeout = 5000);

    /**
     * @brief Annule l'operation en cours
     */
    void cancelOperation();

    GitError lastErrorCode() const { return m_lastErrorCode; }
    QString lastError() const { return m_lastError; }
    QString lastOutput() const { return m_lastOutput; }
    bool isOperationRunning() const { return m_operationRunning; }

signals:
    void operationStarted(const QString& message);
    void operationSuccess(const QString& message);
    void operationFailed(const QString& error, GitError errorCode);
    void operationCancelled();
    void retryAttempt(int attempt, int maxAttempts);
    void connectionCheckStarted();
    void connectionCheckCompleted(bool success);
    void progressUpdate(int current, int total, const QString& currentItem);

private:
    bool executeGitCommand(const QString& workingDir, const QStringList& arguments,
                          int timeoutMs = 30000);
    void setError(GitError code, const QString& message);
    QString getGitErrorMessage(const QString& gitOutput);
    GitError detectErrorType(const QString& errorOutput);
    bool shouldRetry(GitError errorCode);
    void waitBeforeRetry(int attemptNumber);
    
    /**
     * @brief Copie recursivement un dossier
     * @param sourcePath Chemin source
     * @param destPath Chemin destination
     * @param copiedFiles Liste des fichiers copies (output)
     * @return Nombre de fichiers copies
     */
    int copyDirectoryRecursively(const QString& sourcePath, const QString& destPath, 
                                  QStringList& copiedFiles);

    QString m_lastError;
    QString m_lastOutput;
    GitError m_lastErrorCode;
    QProcess* m_process;
    QNetworkAccessManager* m_networkManager;
    bool m_operationRunning;
    bool m_cancelRequested;
};

#endif // GITMANAGER_H
#include "include/gitmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

GitManager::GitManager(QObject* parent)
    : QObject(parent)
    , m_lastErrorCode(GitError::None)
    , m_process(new QProcess(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_operationRunning(false)
    , m_cancelRequested(false) {
}

GitManager::~GitManager() {
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

void GitManager::cancelOperation() {
    if (m_operationRunning) {
        m_cancelRequested = true;
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
        }
        emit operationCancelled();
    }
}

void GitManager::setError(GitError code, const QString& message) {
    m_lastErrorCode = code;
    m_lastError = message;
}

QString GitManager::getGitErrorMessage(const QString& gitOutput) {
    QString output = gitOutput.toLower();
    
    if (output.contains("authentication failed") || output.contains("could not authenticate")) {
        return "Echec d'authentification. Verifiez votre nom d'utilisateur et token.";
    }
    if (output.contains("remote not found") || output.contains("could not read from remote")) {
        return "Depot distant introuvable. Verifiez l'URL du depot.";
    }
    if (output.contains("nothing to commit")) {
        return "Aucune modification a commiter.";
    }
    if (output.contains("network") || output.contains("connection")) {
        return "Erreur reseau. Verifiez votre connexion internet.";
    }
    if (output.contains("permission denied")) {
        return "Permission refusee. Verifiez vos droits d'acces.";
    }
    if (output.contains("not a git repository")) {
        return "Ce repertoire n'est pas un depot Git.";
    }
    
    return gitOutput;
}

GitError GitManager::detectErrorType(const QString& errorOutput) {
    QString output = errorOutput.toLower();
    
    if (output.contains("authentication failed") || 
        output.contains("could not authenticate") ||
        output.contains("invalid credentials") ||
        output.contains("403")) {
        return GitError::AuthenticationFailed;
    }
    
    if (output.contains("could not resolve host") ||
        output.contains("failed to connect") ||
        output.contains("connection refused") ||
        output.contains("network unreachable") ||
        output.contains("connection timed out") ||
        output.contains("could not read from remote")) {
        return GitError::NetworkError;
    }
    
    if (output.contains("ssl") || output.contains("certificate")) {
        return GitError::SSLError;
    }
    
    if (output.contains("proxy")) {
        return GitError::ProxyError;
    }
    
    if (output.contains("remote not found") ||
        output.contains("repository not found") ||
        output.contains("404")) {
        return GitError::RemoteNotFound;
    }
    
    if (output.contains("nothing to commit")) {
        return GitError::NothingToCommit;
    }
    
    return GitError::UnknownError;
}

bool GitManager::shouldRetry(GitError errorCode) {
    switch (errorCode) {
        case GitError::NetworkError:
        case GitError::Timeout:
        case GitError::ConnectionRefused:
        case GitError::ProxyError:
            return true;
        
        case GitError::AuthenticationFailed:
        case GitError::RemoteNotFound:
        case GitError::UserCancelled:
        case GitError::InvalidRepository:
        case GitError::NothingToCommit:
            return false;
        
        default:
            return false;
    }
}

void GitManager::waitBeforeRetry(int attemptNumber) {
    int waitTime = qMin(2000 * (1 << attemptNumber), 10000);
    qDebug() << "Attente de" << waitTime << "ms avant nouvelle tentative";
    QThread::msleep(waitTime);
}

bool GitManager::checkInternetConnection() {
    emit connectionCheckStarted();
    
    QStringList testUrls = {
        "https://www.google.com",
        "https://1.1.1.1",
        "https://8.8.8.8"
    };
    
    for (const QString& url : testUrls) {
        QNetworkRequest request(url);
        request.setTransferTimeout(3000);
        
        QNetworkReply* reply = m_networkManager->head(request);
        
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        
        loop.exec();
        
        bool success = (reply->error() == QNetworkReply::NoError);
        reply->deleteLater();
        
        if (success) {
            emit connectionCheckCompleted(true);
            return true;
        }
    }
    
    emit connectionCheckCompleted(false);
    return false;
}

bool GitManager::checkGitHubConnectivity(int timeout) {
    emit connectionCheckStarted();
    
    QNetworkRequest request(QUrl("https://api.github.com"));
    request.setTransferTimeout(timeout);
    request.setRawHeader("User-Agent", "RoguePublisher/1.0");
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);
    
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    
    emit connectionCheckCompleted(success);
    return success;
}

bool GitManager::isGitAvailable() {
    QProcess process;
    process.start("git", QStringList() << "--version");
    
    if (!process.waitForStarted(5000)) {
        setError(GitError::GitNotInstalled, 
                "Git n'est pas installe ou inaccessible.\n"
                "Telechargez-le depuis: https://git-scm.com/");
        return false;
    }
    
    process.waitForFinished(5000);
    
    if (process.exitCode() == 0) {
        m_lastOutput = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        setError(GitError::None, QString());
        return true;
    }
    
    setError(GitError::GitNotInstalled, "Impossible de verifier la version de Git.");
    return false;
}

bool GitManager::isGitRepository(const QString& repoPath) {
    if (repoPath.isEmpty()) {
        setError(GitError::InvalidRepository, "Chemin de depot vide.");
        return false;
    }
    
    QDir dir(repoPath);
    if (!dir.exists()) {
        setError(GitError::InvalidRepository, "Le repertoire n'existe pas: " + repoPath);
        return false;
    }
    
    bool isRepo = QFileInfo(dir.absoluteFilePath(".git")).exists();
    if (!isRepo) {
        setError(GitError::InvalidRepository, "Ce n'est pas un depot Git: " + repoPath);
    }
    
    return isRepo;
}

bool GitManager::initRepository(const QString& repoPath) {
    if (repoPath.isEmpty()) {
        setError(GitError::InvalidRepository, "Chemin de depot vide.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    QDir dir(repoPath);
    if (!dir.exists()) {
        if (!dir.mkpath(repoPath)) {
            setError(GitError::InvalidRepository, "Impossible de creer le repertoire: " + repoPath);
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
    }
    
    emit operationStarted("Initialisation du depot Git...");
    
    if (!executeGitCommand(repoPath, QStringList() << "init")) {
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    if (m_cancelRequested) {
        m_cancelRequested = false;
        return false;
    }
    
    emit operationSuccess("Depot Git initialise avec succes");
    return true;
}

bool GitManager::setRemoteUrl(const QString& repoPath, const QString& remoteUrl) {
    if (remoteUrl.isEmpty()) {
        setError(GitError::RemoteNotFound, "URL du depot distant vide.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted("Configuration du depot distant...");
    
    if (executeGitCommand(repoPath, QStringList() << "remote" << "get-url" << "origin")) {
        if (!executeGitCommand(repoPath, QStringList() << "remote" << "set-url" << "origin" << remoteUrl)) {
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
    } else {
        if (!executeGitCommand(repoPath, QStringList() << "remote" << "add" << "origin" << remoteUrl)) {
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
    }
    
    if (m_cancelRequested) {
        m_cancelRequested = false;
        return false;
    }
    
    emit operationSuccess("Depot distant configure: " + remoteUrl);
    return true;
}

bool GitManager::copyAndAddFiles(const QString& repoPath, const QStringList& files, 
                                const QString& subdir) {
    if (files.isEmpty()) {
        setError(GitError::FileNotFound, "Aucun fichier a copier.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted(QString("Copie et ajout de %1 fichier(s)...").arg(files.count()));
    
    QString targetDir = repoPath;
    if (!subdir.isEmpty()) {
        targetDir = QDir(repoPath).filePath(subdir);
        QDir dir;
        if (!dir.mkpath(targetDir)) {
            setError(GitError::FileNotFound, "Impossible de creer le repertoire: " + targetDir);
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
    }
    
    QStringList relativeFiles;
    int copiedCount = 0;
    
    for (const QString& sourceFile : files) {
        if (m_cancelRequested) {
            m_cancelRequested = false;
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
            return false;
        }
        
        QFileInfo sourceInfo(sourceFile);
        if (!sourceInfo.exists()) {
            qWarning() << "Fichier source introuvable, ignore:" << sourceFile;
            continue;
        }
        
        QString fileName = sourceInfo.fileName();
        QString destPath = QDir(targetDir).filePath(fileName);
        
        QFile destFile(destPath);
        if (destFile.exists()) {
            if (!destFile.remove()) {
                qWarning() << "Impossible de supprimer le fichier existant:" << destPath;
                continue;
            }
        }
        
        if (!QFile::copy(sourceFile, destPath)) {
            qWarning() << "Echec de la copie:" << sourceFile << "vers" << destPath;
            continue;
        }
        
        QString relativePath;
        if (!subdir.isEmpty()) {
            relativePath = subdir + "/" + fileName;
        } else {
            relativePath = fileName;
        }
        
        relativeFiles << relativePath;
        copiedCount++;
        
        emit operationStarted(QString("Copie: %1").arg(fileName));
    }
    
    if (copiedCount == 0) {
        setError(GitError::FileNotFound, "Aucun fichier n'a pu etre copie.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted(QString("Ajout de %1 fichier(s) a Git...").arg(copiedCount));
    
    for (const QString& relativePath : relativeFiles) {
        if (m_cancelRequested) {
            m_cancelRequested = false;
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
            return false;
        }
        
        if (!executeGitCommand(repoPath, QStringList() << "add" << relativePath)) {
            qWarning() << "Echec lors de l'ajout de:" << relativePath;
        }
    }
    
    emit operationSuccess(QString("%1 fichier(s) copie(s) et ajoute(s)").arg(copiedCount));
    return true;
}

bool GitManager::addFiles(const QString& repoPath, const QStringList& files) {
    if (files.isEmpty()) {
        setError(GitError::FileNotFound, "Aucun fichier a ajouter.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted(QString("Ajout de %1 fichier(s)...").arg(files.count()));
    
    int addedCount = 0;
    for (const QString& file : files) {
        if (m_cancelRequested) {
            m_cancelRequested = false;
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
            return false;
        }
        
        QDir repoDir(repoPath);
        QString relativePath = repoDir.relativeFilePath(file);
        
        if (relativePath.startsWith("..")) {
            qWarning() << "Fichier en dehors du depot, ignore:" << file;
            continue;
        }
        
        if (!QFileInfo::exists(file)) {
            qWarning() << "Fichier introuvable, ignore:" << file;
            continue;
        }
        
        if (!executeGitCommand(repoPath, QStringList() << "add" << relativePath)) {
            qWarning() << "Echec lors de l'ajout de:" << relativePath;
            continue;
        }
        addedCount++;
    }
    
    if (addedCount == 0) {
        setError(GitError::FileNotFound, "Aucun fichier n'a pu etre ajoute.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationSuccess(QString("%1 fichier(s) ajoute(s) a l'index").arg(addedCount));
    return true;
}

bool GitManager::commit(const QString& repoPath, const QString& message) {
    if (message.isEmpty()) {
        setError(GitError::UnknownError, "Message de commit vide.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted("Creation du commit...");
    
    if (!executeGitCommand(repoPath, QStringList() << "commit" << "-m" << message)) {
        if (m_lastOutput.toLower().contains("nothing to commit")) {
            setError(GitError::NothingToCommit, "Aucune modification a commiter.");
        }
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    if (m_cancelRequested) {
        m_cancelRequested = false;
        return false;
    }
    
    emit operationSuccess("Commit cree avec succes");
    return true;
}

bool GitManager::push(const QString& repoPath, const QString& branch,
                     const QString& username, const QString& token, int maxRetries) {
    if (branch.isEmpty()) {
        setError(GitError::UnknownError, "Nom de branche vide.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted("Verification de la connexion internet...");
    if (!checkInternetConnection()) {
        setError(GitError::NetworkError, 
                "Aucune connexion internet detectee.\n"
                "Verifiez votre connexion et reessayez.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted("Verification de l'accessibilite de GitHub...");
    if (!checkGitHubConnectivity()) {
        setError(GitError::NetworkError,
                "GitHub est inaccessible.\n"
                "Verifiez que vous pouvez acceder a github.com depuis votre navigateur.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationStarted("Push vers le depot distant...");
    
    QStringList args;
    args << "push";
    
    if (!username.isEmpty() && !token.isEmpty()) {
        if (executeGitCommand(repoPath, QStringList() << "remote" << "get-url" << "origin")) {
            QString remoteUrl = m_lastOutput.trimmed();
            
            if (remoteUrl.startsWith("https://")) {
                remoteUrl.remove(0, 8);
                QString authenticatedUrl = QString("https://%1:%2@%3")
                    .arg(username, token, remoteUrl);
                args << authenticatedUrl << branch;
            } else {
                args << "origin" << branch;
            }
        }
    } else {
        args << "origin" << branch;
    }
    
    int attempt = 0;
    bool success = false;
    
    while (attempt < maxRetries && !success) {
        if (m_cancelRequested) {
            m_cancelRequested = false;
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
        
        if (attempt > 0) {
            emit retryAttempt(attempt + 1, maxRetries);
            emit operationStarted(QString("Nouvelle tentative (%1/%2)...")
                                .arg(attempt + 1)
                                .arg(maxRetries));
            
            if (!checkInternetConnection()) {
                qWarning() << "Connexion perdue, attente avant retry...";
                waitBeforeRetry(attempt);
                attempt++;
                continue;
            }
        }
        
        if (executeGitCommand(repoPath, args, 120000)) {
            success = true;
            break;
        }
        
        GitError errorType = detectErrorType(m_lastError);
        
        if (!shouldRetry(errorType)) {
            if (errorType == GitError::AuthenticationFailed) {
                setError(GitError::AuthenticationFailed,
                        "Echec d'authentification.\n\n"
                        "Verifiez:\n"
                        "- Votre nom d'utilisateur GitHub\n"
                        "- La validite de votre token\n"
                        "- Les permissions du token (repo)");
            } else if (errorType == GitError::RemoteNotFound) {
                setError(GitError::RemoteNotFound,
                        "Depot distant introuvable.\n\n"
                        "Verifiez:\n"
                        "- L'URL du depot\n"
                        "- Que le depot existe sur GitHub\n"
                        "- Vos permissions d'acces au depot");
            }
            
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
        
        if (attempt < maxRetries - 1) {
            waitBeforeRetry(attempt);
        }
        
        attempt++;
    }
    
    if (!success) {
        setError(GitError::NetworkError,
                QString("Echec apres %1 tentatives.\n\n"
                       "Erreur: %2\n\n"
                       "Suggestions:\n"
                       "- Verifiez votre connexion internet\n"
                       "- Verifiez vos parametres proxy\n"
                       "- Reessayez plus tard")
                    .arg(maxRetries)
                    .arg(m_lastError));
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationSuccess("Push effectue avec succes vers " + branch);
    return true;
}

bool GitManager::pull(const QString& repoPath, const QString& branch,
                      const QString& username, const QString& token) {
    emit operationStarted("Recuperation des modifications distantes...");
    
    // Récupérer l'URL du remote
    QString remoteUrl;
    if (executeGitCommand(repoPath, QStringList() << "remote" << "get-url" << "origin")) {
        remoteUrl = m_lastOutput.trimmed();
    } else {
        emit operationFailed("Impossible de recuperer l'URL du depot distant", GitError::RemoteNotFound);
        return false;
    }
    
    QStringList args;
    args << "pull";
    
    // Construire l'URL avec authentification si nécessaire
    if (!username.isEmpty() && !token.isEmpty() && remoteUrl.startsWith("https://")) {
        remoteUrl.remove(0, 8); // Enlever "https://"
        QString authenticatedUrl = QString("https://%1:%2@%3")
            .arg(username, token, remoteUrl);
        args << authenticatedUrl << branch;
    } else {
        args << "origin" << branch;
    }
    
    bool success = executeGitCommand(repoPath, args);
    
    if (success) {
        emit operationSuccess("Pull termine avec succes");
    } else {
        emit operationFailed("Echec du pull", GitError::NetworkError);
    }
    
    return success;
}

bool GitManager::pullRebase(const QString& repoPath, const QString& branch,
                            const QString& username, const QString& token) {
    emit operationStarted("Recuperation et rebase des modifications...");
    
    QStringList args;
    args << "pull" << "--rebase" << "origin" << branch;
    
    bool success = executeGitCommand(repoPath, args);
    
    if (success) {
        emit operationSuccess("Pull avec rebase termine");
    } else {
        emit operationFailed("Echec du pull rebase", GitError::NetworkError);
    }
    
    return success;
}

bool GitManager::checkRemoteStatus(const QString& repoPath, const QString& branch) {
    // Fetch pour voir les changements distants
    QStringList fetchArgs;
    fetchArgs << "fetch" << "origin" << branch;
    
    if (!executeGitCommand(repoPath, fetchArgs)) {
        return false;
    }
    
    // Comparer local vs distant
    QStringList statusArgs;
    statusArgs << "rev-list" << "--count" 
               << QString("HEAD..origin/%1").arg(branch);
    
    if (executeGitCommand(repoPath, statusArgs)) {
        int commitsAhead = m_lastOutput.trimmed().toInt();
        return commitsAhead == 0; // Retourne true si à jour
    }
    
    return false;
}

bool GitManager::executeGitCommand(const QString& workingDir, const QStringList& arguments, int timeoutMs) {
    m_lastError.clear();
    m_lastOutput.clear();
    m_lastErrorCode = GitError::None;
    m_operationRunning = true;
    m_cancelRequested = false;
    
    if (!QDir(workingDir).exists()) {
        setError(GitError::InvalidRepository, "Repertoire inexistant: " + workingDir);
        m_operationRunning = false;
        return false;
    }
    
    m_process->setWorkingDirectory(workingDir);
    m_process->start("git", arguments);
    
    if (!m_process->waitForStarted(5000)) {
        setError(GitError::ProcessFailed, "Impossible de demarrer Git. Verifiez qu'il est installe.");
        m_operationRunning = false;
        return false;
    }
    
    if (!m_process->waitForFinished(timeoutMs)) {
        if (m_cancelRequested) {
            m_process->kill();
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
        } else {
            m_process->kill();
            setError(GitError::Timeout, QString("Timeout apres %1 secondes.").arg(timeoutMs / 1000));
        }
        m_operationRunning = false;
        return false;
    }
    
    m_lastOutput = QString::fromUtf8(m_process->readAllStandardOutput());
    QString errorOutput = QString::fromUtf8(m_process->readAllStandardError());
    
    m_operationRunning = false;
    
    if (m_process->exitCode() != 0) {
        QString errorMsg = errorOutput.isEmpty() ? m_lastOutput : errorOutput;
        errorMsg = getGitErrorMessage(errorMsg);
        
        if (errorMsg.contains("authentication", Qt::CaseInsensitive)) {
            setError(GitError::AuthenticationFailed, errorMsg);
        } else if (errorMsg.contains("network", Qt::CaseInsensitive) || 
                   errorMsg.contains("connection", Qt::CaseInsensitive)) {
            setError(GitError::NetworkError, errorMsg);
        } else if (errorMsg.contains("not found", Qt::CaseInsensitive)) {
            setError(GitError::RemoteNotFound, errorMsg);
        } else if (errorMsg.contains("nothing to commit", Qt::CaseInsensitive)) {
            setError(GitError::NothingToCommit, errorMsg);
        } else {
            setError(GitError::UnknownError, errorMsg);
        }
        
        return false;
    }
    
    if (!errorOutput.isEmpty()) {
        m_lastOutput += "\n" + errorOutput;
    }
    
    return true;
}

int GitManager::copyDirectoryRecursively(const QString& sourcePath, const QString& destPath, 
                                         QStringList& copiedFiles) {
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        return 0;
    }
    
    QDir destDir(destPath);
    if (!destDir.exists()) {
        destDir.mkpath(destPath);
    }
    
    int count = 0;
    
    // Copier tous les fichiers du dossier actuel
    QFileInfoList files = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : files) {
        if (m_cancelRequested) {
            return count;
        }
        
        QString destFile = destDir.filePath(fileInfo.fileName());
        
        // Supprimer si existe déjà
        if (QFile::exists(destFile)) {
            QFile::remove(destFile);
        }
        
        if (QFile::copy(fileInfo.filePath(), destFile)) {
            copiedFiles << destFile;
            count++;
            emit progressUpdate(count, -1, fileInfo.fileName());
        } else {
            qWarning() << "Echec de copie:" << fileInfo.filePath() << "vers" << destFile;
        }
    }
    
    // Copier recursivement tous les sous-dossiers
    QFileInfoList dirs = sourceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& dirInfo : dirs) {
        if (m_cancelRequested) {
            return count;
        }
        
        QString subDestPath = destDir.filePath(dirInfo.fileName());
        count += copyDirectoryRecursively(dirInfo.filePath(), subDestPath, copiedFiles);
    }
    
    return count;
}

bool GitManager::copyProjectRecursively(const QString& repoPath, const QStringList& paths, 
                                        bool preserveStructure) {
    if (paths.isEmpty()) {
        setError(GitError::FileNotFound, "Aucun fichier ou dossier a copier.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    QStringList allCopiedFiles;
    int totalCount = 0;
    
    emit operationStarted(QString("Copie de %1 element(s)...").arg(paths.count()));
    
    QDir repoDir(repoPath);
    
    for (const QString& path : paths) {
        if (m_cancelRequested) {
            m_cancelRequested = false;
            setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
            emit operationFailed(m_lastError, m_lastErrorCode);
            return false;
        }
        
        QFileInfo pathInfo(path);
        
        if (!pathInfo.exists()) {
            qWarning() << "Element introuvable, ignore:" << path;
            continue;
        }
        
        if (pathInfo.isFile()) {
            // Copier un fichier unique
            QString destFile = repoDir.filePath(pathInfo.fileName());
            
            if (QFile::exists(destFile)) {
                QFile::remove(destFile);
            }
            
            if (QFile::copy(path, destFile)) {
                allCopiedFiles << destFile;
                totalCount++;
                emit operationStarted(QString("Copie: %1").arg(pathInfo.fileName()));
            } else {
                qWarning() << "Echec de copie:" << path;
            }
            
        } else if (pathInfo.isDir()) {
            // Copier un dossier recursivement
            QString folderName = pathInfo.fileName();
            QString destFolder = repoDir.filePath(folderName);
            
            emit operationStarted(QString("Copie du dossier: %1...").arg(folderName));
            
            QStringList dirFiles;
            int dirCount = copyDirectoryRecursively(path, destFolder, dirFiles);
            
            allCopiedFiles.append(dirFiles);
            totalCount += dirCount;
            
            emit operationSuccess(QString("Dossier %1: %2 fichier(s) copie(s)")
                                .arg(folderName)
                                .arg(dirCount));
        }
    }
    
    if (totalCount == 0) {
        setError(GitError::FileNotFound, "Aucun fichier n'a pu etre copie.");
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    emit operationSuccess(QString("Total: %1 fichier(s) copie(s)").arg(totalCount));
    return true;
}

bool GitManager::addAllFiles(const QString& repoPath) {
    emit operationStarted("Ajout de tous les fichiers au depot Git...");
    
    // Utiliser "git add -A" pour ajouter tous les fichiers
    if (!executeGitCommand(repoPath, QStringList() << "add" << "-A")) {
        emit operationFailed(m_lastError, m_lastErrorCode);
        return false;
    }
    
    if (m_cancelRequested) {
        m_cancelRequested = false;
        setError(GitError::UserCancelled, "Operation annulee par l'utilisateur.");
        return false;
    }
    
    emit operationSuccess("Tous les fichiers ont ete ajoutes a l'index Git");
    return true;
}
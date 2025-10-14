#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

// Constructeur
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Personnalisation de l'interface
    setWindowTitle("Rogue Publisher");
    logMessage("Bienvenue sur Rogue Publisher ! Pr�t � publier.");
}

// Destructeur
MainWindow::~MainWindow() {
    delete ui;
}

// Affiche un message dans la zone de log avec un horodatage
void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->logOutput->append(QString("[%1] %2").arg(timestamp, message));
}

// Slot pour le bouton "Ajouter des Fichiers"
void MainWindow::on_addFilesButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "S�lectionner des fichiers � ajouter",
        QDir::homePath(),
        "Tous les fichiers (*.*)"
    );

    if (!files.isEmpty()) {
        ui->fileListWidget->addItems(files);
        logMessage(QString("%1 fichier(s) ajout�(s) � la liste.").arg(files.count()));
    }
    else {
        logMessage("Aucun fichier s�lectionn�.");
    }
}

// Slot pour l'action "Ouvrir..." du menu
void MainWindow::on_actionOuvrir_triggered() {
    on_addFilesButton_clicked();
}

// Slot pour l'action "Quitter"
void MainWindow::on_actionQuitter_triggered() {
    QApplication::quit();
}

// Slot pour le bouton "Simuler le Commit & Push"
void MainWindow::on_pushToGitHubButton_clicked() {
    if (ui->fileListWidget->count() == 0) {
        QMessageBox::warning(this, "Aucun fichier", "Veuillez ajouter des fichiers avant de continuer.");
        logMessage("Tentative de push sans fichiers.");
        return;
    }

    logMessage("--- D�but de la simulation Git ---");

    for (int i = 0; i < ui->fileListWidget->count(); ++i) {
        QString filePath = ui->fileListWidget->item(i)->text();
        logMessage(QString("Ex�cution de : git add \"%1\"").arg(filePath));
    }

    QString commitMessage = "Commit automatique depuis Rogue Publisher";
    logMessage(QString("Ex�cution de : git commit -m \"%1\"").arg(commitMessage));
    logMessage("Ex�cution de : git push origin main");
    logMessage("--- Simulation termin�e avec succ�s ---");

    QMessageBox::information(this, "Succ�s", "La simulation de commit et push sur GitHub s'est termin�e avec succ�s !");

    ui->fileListWidget->clear();
}
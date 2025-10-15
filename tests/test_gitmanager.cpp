#include <QtTest/QtTest>
#include "include/gitmanager.h" // Assurez-vous que le chemin est correct

class TestGitManager : public QObject
{
    Q_OBJECT

private slots:
    void testIsGitAvailable();
};

void TestGitManager::testIsGitAvailable()
{
    GitManager manager;
    // Ce test suppose que 'git' est disponible dans l'environnement d'exécution de la CI
    QVERIFY(manager.isGitAvailable());
}

QTEST_MAIN(TestGitManager)
#include "test_gitmanager.moc"
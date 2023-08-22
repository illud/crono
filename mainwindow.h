#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addGameBtn_clicked();
    QString getGameImage(QString gameName);
    void addedGame(const QString &gameName,const QString &gameExePath);
    void getGame();
    QString secondsToTime(int time);
    QVector<QString> RemoveDupWord(std::string str);
    void on_btnPlay_clicked(int gameId, QString gameExePath);
    bool isProcessRunning(const QString &processName);
    void checkRunningGame(int gameId, QString gameName);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

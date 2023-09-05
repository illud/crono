#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int numCols = 4;
    QString gameNameValue;
    int gameIdValue;
    QString gameExePathValue;
    QString gameExeValue;
    Qt::WindowFlags initialFlags;
    QPoint initialPosition;

private slots:
    void on_addGameBtn_clicked();
    void addedGame(const QString &gameName, const QString &gameExePath);
    void getGame(bool goToGamesPage);
    void on_btnPlay_clicked(QString gameName, int gameId, QString gameExePath, QString gameExe);
    void checkRunningGame(int gameId, QString gameName);
    void on_btnClose_clicked();
    void on_minimizeBtn_clicked();
    void on_maxBtn_clicked();
    void on_btnGames_clicked();
    void on_btnBack_clicked();
    void goToGame(QString gameName, int gameId, QString gameExePath, QString gameExe);
    void on_btnStartGame_clicked();
    void deleteGame(int gameId, QString gameName);
    void on_btnHltb_clicked();
    void updateGame(int gameId, QString gameName, QString gameExeLocation);

    void on_radioBtnTimeIndicator_clicked();

public slots:
    void on_statsBtn_clicked();

signals:
    void refreshStats();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QPoint startPos;
};
#endif // MAINWINDOW_H

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
    void on_btnPlay_clicked(int gameId, QString gameExePath, QString gameExe);
    void checkRunningGame(int gameId, QString gameName);

    void on_btnClose_clicked();

    void on_minimizeBtn_clicked();

    void on_maxBtn_clicked();


    void on_btnGames_clicked();

    void on_statsBtn_clicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    QTimer* timer;
    QPoint startPos;

};
#endif // MAINWINDOW_H

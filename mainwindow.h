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
    void on_btnPlay_clicked(QString gameExePath);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

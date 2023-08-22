#ifndef NEWGAME_H
#define NEWGAME_H

#include <QDialog>

namespace Ui {
class NewGame;
}

class NewGame : public QDialog
{
    Q_OBJECT

public:
    explicit NewGame(QWidget *parent = nullptr);
    ~NewGame();

signals:
    void gameAdded(const QString &gameName,const QString &gameExePath);

private slots:
    void on_addBtn_clicked();

    void on_searchGameExeBtn_clicked();

    void on_closeBtn_clicked();

private:
    Ui::NewGame *ui;
};

#endif // NEWGAME_H

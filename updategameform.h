#ifndef UPDATEGAMEFORM_H
#define UPDATEGAMEFORM_H

#include <QWidget>

namespace Ui
{
    class UpdateGameForm;
}

class UpdateGameForm : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateGameForm(int gameId, QString gameName, QString gameExeLocation, QWidget *parent = nullptr);
    ~UpdateGameForm();
    QString gameExePath;
    int gameId;
    const QString path = "crono.db";

private slots:
    void on_updateBtn_clicked();

    void on_searchGameExeBtn_clicked();

    void on_closeBtn_clicked();
    void checkActiveTheme();

signals:
    void gameUpdated();

private:
    Ui::UpdateGameForm *ui;
};

#endif // UPDATEGAMEFORM_H

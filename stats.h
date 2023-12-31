#ifndef STATS_H
#define STATS_H

#include <QWidget>

namespace Ui
{
    class Stats;
}

class Stats : public QWidget
{
    Q_OBJECT

public:
    explicit Stats(QWidget *parent = nullptr);
    ~Stats();
    const QString path = "crono.db";

private slots:

    void on_btnReload_clicked();

public slots:
    void refreshStats();

private:
    Ui::Stats *ui;
};

#endif // STATS_H

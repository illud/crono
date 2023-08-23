#ifndef STATS_H
#define STATS_H

#include <QWidget>

namespace Ui {
class Stats;
}

class Stats : public QWidget
{
    Q_OBJECT

public:
    explicit Stats(QWidget *parent = nullptr);
    ~Stats();

private:
    Ui::Stats *ui;
};

#endif // STATS_H

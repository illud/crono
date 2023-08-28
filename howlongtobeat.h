#ifndef HOWLONGTOBEAT_H
#define HOWLONGTOBEAT_H

#include <QWidget>

namespace Ui
{
    class HowLongTobeat;
}

class HowLongTobeat : public QWidget
{
    Q_OBJECT

public:
    explicit HowLongTobeat(QWidget *parent = nullptr);
    ~HowLongTobeat();

private slots:
    void on_btnSearch_clicked();

private:
    Ui::HowLongTobeat *ui;
};

#endif // HOWLONGTOBEAT_H

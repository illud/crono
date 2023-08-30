#ifndef CARD_H
#define CARD_H


#include <QWidget>
#include <QLabel>

class Card : public QWidget {
    Q_OBJECT

public:
    Card(QWidget *parent = nullptr);
    void setLabelText(const QString &text);

private:
    QLabel *label;
};

#endif // CARD_H

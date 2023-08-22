// CustomRowCard.h
#ifndef CUSTOMROWCARD_H
#define CUSTOMROWCARD_H

#include "qlabel.h"
#include "qpushbutton.h"
#include <QWidget>

class CustomRowCard : public QWidget {
    Q_OBJECT

public:
    explicit CustomRowCard(QWidget *parent = nullptr);
    void setGameData(const QString &imagePath, const QString &gameName, const QString &timePlayed, bool isRunning, const QString &gameExePath);

signals:
    void playButtonClicked(const QString &gameExePath);

private:
    QLabel *imageLabel;
    QLabel *nameLabel;
    QLabel *timeLabel;
    QPushButton *playButton;
};

#endif // CUSTOMROWCARD_H

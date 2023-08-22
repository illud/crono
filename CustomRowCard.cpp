// CustomRowCard.cpp
#include "CustomRowCard.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>

CustomRowCard::CustomRowCard(QWidget *parent) : QWidget(parent) {
    imageLabel = new QLabel(this);
    nameLabel = new QLabel(this);
    timeLabel = new QLabel(this);
    playButton = new QPushButton(this);

    // Set up layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(imageLabel);
    layout->addWidget(nameLabel);
    layout->addWidget(timeLabel);
    layout->addWidget(playButton);

    setLayout(layout);

    // Connect playButton's clicked signal to emit custom signal
    connect(playButton, &QPushButton::clicked, [this]() {
        emit playButtonClicked(playButton->property("gameExePath").toString());
    });
}

void CustomRowCard::setGameData(const QString &imagePath, const QString &gameName, const QString &timePlayed, bool isRunning, const QString &gameExePath) {
    QImage image(imagePath);
    QPixmap pixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(pixmap.scaledToHeight(100)); // Set an appropriate size

    nameLabel->setText(gameName);
    timeLabel->setText(timePlayed);

    playButton->setProperty("gameExePath", gameExePath);

    if (isRunning) {
        playButton->setText("RUNNING");
        playButton->setStyleSheet("..."); // Set your style sheet for running state
    } else {
        playButton->setText("PLAY");
        playButton->setStyleSheet("..."); // Set your style sheet for not running state
    }
}

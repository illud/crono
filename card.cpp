#include "card.h"
#include "qboxlayout.h"

Card::Card(QWidget *parent) : QWidget(parent) {
    // Create and set up the QLabel
    label = new QLabel("Default Text", this);
    label->setAlignment(Qt::AlignCenter);

    // Set the layout for the custom widget
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

void Card::setLabelText(const QString &text) {
    label->setText(text);
}

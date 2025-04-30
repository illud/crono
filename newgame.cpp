#include "newgame.h"
#include "ImageUtil.h"
#include "qmessagebox.h"
#include "qpainter.h"
#include "qpainterpath.h"
#include "qtablewidget.h"
#include "ui_newgame.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

NewGame::NewGame(QWidget *parent) : QDialog(parent),
                                    ui(new Ui::NewGame)
{
    ui->setupUi(this);
    NewGame::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    // Inside your slot where you are handling the image download and set the icon

    // Inside your slot where you are handling the image download and set the icon

    ImageUtil *imageUtil = new ImageUtil();
    imageUtil->loadFromUrl(QUrl("https://i6.imageban.ru/out/2024/12/13/5ccf1fb10ab8d0a59066983058ac3270.jpg"));
    imageUtil->connect(imageUtil, &ImageUtil::loaded, this,
                       [=]() {
                           QImage image = imageUtil->image();  // Get the image from ImageUtil

                           // Scale the image to a larger size (e.g., 100x100)
                           QImage scaledImage = image.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                           // Convert the scaled QImage to QPixmap for display
                           QPixmap pixmap = QPixmap::fromImage(scaledImage);

                           // Create a rounded QPixmap
                           QPixmap roundedPixmap(pixmap.size());
                           roundedPixmap.fill(Qt::transparent);  // Transparent background

                           // Use QPainter to draw rounded corners
                           QPainter painter(&roundedPixmap);
                           painter.setRenderHint(QPainter::Antialiasing, true);

                           QPainterPath path;
                           path.addRoundedRect(0, 0, pixmap.width(), pixmap.height(), 15, 15);  // Rounded corners
                           painter.setClipPath(path);

                           painter.drawPixmap(0, 0, pixmap);

                           // Now add the icon to your combo box
                           QIcon icon(roundedPixmap);  // Create QIcon from the rounded QPixmap

                           // Add the item to the combo box (with text "text" and the rounded icon)
                           ui->imagePicker->addItem(icon, "");

                           // Increase the size of the items in the combo box (adjust icon size)
                           ui->imagePicker->setIconSize(QSize(50, 50));  // Set larger icon size

                           // Optionally, adjust the height of the combo box if needed
                           ui->imagePicker->setFixedHeight(50);  // Adjust combo box height

                           // Clean up
                           imageUtil->deleteLater();
                       });


}

QString gameExe;

NewGame::~NewGame()
{
    delete ui;
}

void NewGame::on_addBtn_clicked()
{
    if (ui->gameName->text().isEmpty() || gameExe.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setWindowFlags(Qt::FramelessWindowHint);

        msgBox.setStyleSheet("QMessageBox{background-color: rgb(22, 22, 22);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
        msgBox.setText("Please fill in all the fields");

        QPushButton *yesButton = msgBox.addButton(" Ok ", QMessageBox::ActionRole);

        yesButton->setStyleSheet("QPushButton {"
                                 "background-color: transparent;"
                                 "font: 900 10pt 'Arial Black';"
                                 "color: rgb(255, 255, 255);"
                                 "border: 1px;"
                                 "border-color: rgb(255, 255, 255);"
                                 "border-style: outset;"
                                 "border-radius: 10px;"
                                 "}"

                                 "QPushButton::hover{ "
                                 "  background-color: rgb(252, 196, 25);"
                                 "font: 900 10pt 'Arial Black';"
                                 "color: rgb(255, 255, 255);"
                                 " border: 0px;"

                                 "}"

                                 "QPushButton::focus:pressed{ "
                                 "background-color: rgb(252, 72, 25);"
                                 "font: 900 10pt 'Arial Black';"
                                 "color: rgb(255, 255, 255);"
                                 "border: 0px;"
                                 "}");

        msgBox.exec();

        if (msgBox.clickedButton() == yesButton)
        {
        }
    }
    else
    {
        // Replace forward slashes with backslashes
        gameExe.replace("/", "\\");

        emit gameAdded(ui->gameName->text(), gameExe);
        this->close();

        this->deleteLater();
    }
}

void NewGame::on_searchGameExeBtn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Get Game EXE");
    QDir d = QFileInfo(filePath).absoluteDir();
    QString absolute = d.absoluteFilePath(filePath);
    gameExe = absolute;
    // qDebug() <<  gameExe;
}

void NewGame::on_closeBtn_clicked()
{
    this->close();
    this->deleteLater();
}

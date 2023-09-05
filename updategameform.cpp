#include "updategameform.h"
#include "qmessagebox.h"
#include "ui_updategameform.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include "dbmanager.h"
#include "util.h"

UpdateGameForm::UpdateGameForm(int _gameId, QString _gameName, QString _gameExeLocation, QWidget *parent) : QWidget(parent),
                                                                                                            ui(new Ui::UpdateGameForm)
{
    ui->setupUi(this);

    UpdateGameForm::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->gameName->setText(_gameName);
    gameExePath = _gameExeLocation;
    gameId = _gameId;
}

UpdateGameForm::~UpdateGameForm()
{
    delete ui;
}

void UpdateGameForm::on_updateBtn_clicked()
{
    if (ui->gameName->text() == "" || gameExePath == "")
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
        gameExePath.replace("/", "\\");

        Util util;

        static const QString path = "crono.db";

        // Get image url
        QString imageUrl = util.getGameImage(ui->gameName->text());

        // Instance db conn
        DbManager *db = new DbManager(path);

        // Inser into games table
        db->updateGame(imageUrl, ui->gameName->text(), util.removeDataFromLasBackSlash(gameExePath), util.findLastBackSlashWord(gameExePath.toStdString()), gameId);

        delete db;

        emit gameUpdated();

        this->close();
        this->deleteLater();
    }
}

void UpdateGameForm::on_searchGameExeBtn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Get Game EXE");
    QDir d = QFileInfo(filePath).absoluteDir();
    QString absolute = d.absoluteFilePath(filePath);
    gameExePath = absolute;
}

void UpdateGameForm::on_closeBtn_clicked()
{
    this->close();
    this->deleteLater();
}

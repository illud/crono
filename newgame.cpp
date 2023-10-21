#include "newgame.h"
#include "dbmanager.h"
#include "qmessagebox.h"
#include "ui_newgame.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

NewGame::NewGame(QWidget *parent) : QDialog(parent),
                                    ui(new Ui::NewGame)
{
    ui->setupUi(this);
    NewGame::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    NewGame::checkActiveTheme();
}

QString gameExe;

NewGame::~NewGame()
{
    delete ui;
}

void NewGame::on_addBtn_clicked()
{
    if (ui->gameName->text() == "" || gameExe == "")
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

void NewGame::checkActiveTheme(){
    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Achivements> achivementsData = db->getAchivements();

    if(achivementsData[0].active){
        NewGame::setStyleSheet("background-color: rgb(33, 33, 33)");
    }

    if(achivementsData[1].active){
        NewGame::setStyleSheet("background-color: rgb(40,45,40)");
    }

    if(achivementsData[2].active){
        NewGame::setStyleSheet("background-color: rgb(0, 72, 129)");
    }

    if(achivementsData[3].active){
        NewGame::setStyleSheet("background-color: rgb(34,61,64)");
    }

    if(achivementsData[4].active){
        NewGame::setStyleSheet("background-color: rgb(23, 116, 107)");
    }
}

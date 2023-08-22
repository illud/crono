#include "newgame.h"
#include "ui_newgame.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

NewGame::NewGame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewGame)
{
    ui->setupUi(this);
    NewGame::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

QString gameExe;

NewGame::~NewGame()
{
    delete ui;
}

void NewGame::on_addBtn_clicked()
{
    emit gameAdded(ui->gameName->text(), gameExe);
    this->close();
}


void NewGame::on_searchGameExeBtn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Get Game EXE");
    QDir d = QFileInfo(filePath).absoluteDir();
    QString absolute = d.absoluteFilePath(filePath);
    gameExe = absolute;
    //qDebug() <<  gameExe;
}


void NewGame::on_closeBtn_clicked()
{
    this->close();
}


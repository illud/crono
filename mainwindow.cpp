#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newgame.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_addGameBtn_clicked()
{
    NewGame *newGame = new NewGame(this);

    connect(newGame, &NewGame::gameAdded, this, &MainWindow::addedGame);

    newGame->show();
}

void MainWindow::addedGame(const QString &gameName, const QString &gameExePath){
    //Sets tableWidget row count
    ui->tableWidget->setRowCount(2);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem(gameName));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem(gameExePath));
    qDebug() <<  gameName  <<  gameExePath;
}

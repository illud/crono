#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newgame.h"
#include <curl/curl.h>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <sstream>
#include <QTimer>
#include "ImageUtil.h"
#include "dbmanager.h"
#include "util.h"
#include <QMouseEvent>
#ifdef Q_OS_WIN // Windows-specific code
#include <windows.h>
#endif
#include <QFile>
#include <QTextStream>
#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include "stats.h"
#include "updategameform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MainWindow::setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    // Install the event filter to capture mouse events for resizing
    this->installEventFilter(this);

    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Sets all running column to false at start of the app
    db->updateAllGameRunning();
    delete db;

    // Get games and start of the app
    GetGame();

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Disable the vertical header (row index counter)
    // Remove the header
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);

    ui->tableWidget->setVerticalScrollMode(QTableWidget::ScrollPerPixel);

    QMenu *menu = new QMenu();

    menu->setStyleSheet("QMenu{background-color: rgba(33, 31, 29, 100); color: white;}");
    QAction *actionEdit = new QAction("Edit", this);
    menu->addAction(actionEdit);

    connect(actionEdit, &QAction::triggered, [=]()
            { MainWindow::UpdateGame(gameIdValue, gameNameValue, gameExePathValue); });

    QAction *actionDelete = new QAction("Delete", this);
    menu->addAction(actionDelete);

    connect(actionDelete, &QAction::triggered, [=]()
            { MainWindow::DeleteGame(gameIdValue, gameNameValue); });

    ui->toolButton->setMenu(menu);
    ui->toolButton->setPopupMode(QToolButton::InstantPopup);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// NewGame form dialog
void MainWindow::on_addGameBtn_clicked()
{
    NewGame *newGame = new NewGame(this);

    connect(newGame, &NewGame::gameAdded, this, &MainWindow::AddedGame);

    newGame->show();
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void MainWindow::AddedGame(const QString &gameName, const QString &gameExePath)
{
    Util util;

    static const QString path = "crono.db";

    // Get image url
    QString imageUrl = util.GetGameImage(gameName);

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Inser into games table
    db->insertGame(imageUrl, gameName, util.RemoveDataFromLasBackSlash(gameExePath), util.FindLastBackSlashWord(gameExePath.toStdString()));

    delete db;

    // Get games and start of the app
    GetGame();
}

void MainWindow::GetGame()
{
    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Games> gamesResult = db->getGames();

    delete db;

    // size of columns
    int chunkSize = numCols; // Size of each chunk

    // Calculate the number of chunks needed for calculate row
    int tableWidgetRows = (gamesResult.count() + chunkSize - 1) / chunkSize;

    // Sets tableWidget row count
    ui->tableWidget->setRowCount(tableWidgetRows);

    // Sets tableWidget row count
    // ui->tableWidget->setRowCount(gamesResult.count());

    // Sets icon size
    ui->tableWidget->setIconSize(QSize(300, 300));

    ui->tableWidget->setColumnCount(numCols);

    ui->tableWidget->viewport()->setCursor(Qt::PointingHandCursor);

    for (int col = 0; col < gamesResult.count(); ++col)
    {
        // Set the row height for a specific row
        ui->tableWidget->setRowHeight(col, 300);

        if (gamesResult[col].gameImage == "")
        {
            // Create a QTableWidgetItem and set the image as its icon
            QTableWidgetItem *item = new QTableWidgetItem(gamesResult[col].gameName.toUpper());

            // Convert QImage to QPixmap for display
            // QPixmap pixmap(":/item.png");

            // Sets icon
            // item->setIcon(QIcon(pixmap));

            // ui->tableWidget->cellWidget(0, col)->setStyleSheet("QTableWidget::item { border: 1px solid %3; }");

            // Set the background color for the item
            QColor backgroundColor(22, 22, 22); // Replace with your desired color
            item->setBackground(QBrush(backgroundColor));

            // Set the text color for the item
            item->setTextAlignment(Qt::AlignCenter);

            // Set the font for the item
            QFont font("Arial", 9, QFont::Bold); // Replace with your desired font details
            item->setFont(font);

            // Create a QVariant to hold the game data for the current column
            QVariant gameDataVariant;

            // Set the value of the QVariant to the game data from the gamesResult list
            gameDataVariant.setValue(gamesResult[col]);

            // Set the QVariant containing game data as user data for the QTableWidgetItem
            // This is done using the Qt::UserRole constant, which is a role for custom data
            item->setData(Qt::UserRole, gameDataVariant);

            // Set the item in the table widget
            ui->tableWidget->setItem(0, col, item);
        }
        else
        {
            // Download image from url and set image as icon
            ImageUtil *imageUtil = new ImageUtil();
            imageUtil->loadFromUrl(QUrl(gamesResult[col].gameImage));
            imageUtil->connect(imageUtil, &ImageUtil::loaded,
                               [=]()
                               {
                                   QImage image = imageUtil->image(); // Get the image from ImageUtil

                                   // Convert QImage to QPixmap for display
                                   QPixmap pixmap = QPixmap::fromImage(image);

                                   // Create a QTableWidgetItem and set the image as its icon
                                   QTableWidgetItem *imageItem = new QTableWidgetItem();

                                   // Sets icon
                                   imageItem->setIcon(QIcon(pixmap));

                                   // Set the item in the table widget
                                   ui->tableWidget->setItem(0, col, imageItem);

                                   // Create a QVariant to hold the game data for the current column
                                   QVariant gameDataVariant;

                                   // Set the value of the QVariant to the game data from the gamesResult list
                                   gameDataVariant.setValue(gamesResult[col]);

                                   // Set the QVariant containing game data as user data for the QTableWidgetItem
                                   // This is done using the Qt::UserRole constant, which is a role for custom data
                                   imageItem->setData(Qt::UserRole, gameDataVariant);

                                   // Free memory if else couses memory leak
                                   imageUtil->deleteLater();
                               });
        }
    }

    // Connect the cellClicked signal of the table widget
    connect(ui->tableWidget, &QTableWidget::cellClicked, [=](int row, int col)
            {

        // Retrieve the QVariant containing game data from the user role of the clicked cell
        QVariant gameDataVariant = ui->tableWidget->item(row, col)->data(Qt::UserRole);

        // Check if the retrieved QVariant is valid
        if (gameDataVariant.isValid()) {
            // Retrieve the stored game data from the QVariant
            DbManager::Games gameData = gameDataVariant.value<DbManager::Games>();

            // Perform the custom action using the retrieved game data
            GoToGame(gameData.gameName, gameData.id, gameData.gameExePath, gameData.gameExe);
        } });

    // Return to games view
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::GoToGame(QString gameName, int gameId, QString gameExePath, QString gameExe)
{
    gameNameValue = gameName;
    gameIdValue = gameId;
    gameExePathValue = gameExePath;
    gameExeValue = gameExe;

    // Update running
    static const QString path = "crono.db";
    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Games> gamesResult = db->getGameById(gameId);

    Util util;
    // Set total time played
    ui->timePlayedText->setText(util.SecondsToTime(gamesResult[0].timePlayed));

    ui->gameNameText->setText(gameName.toUpper());

    // Gets time played filter by game id
    int timePlayedResult = db->totalPlayTime(gameId);
    ui->timePlayedText->setText(util.SecondsToTime(timePlayedResult));

    // Gets time played this week filter by game id
    int timePlayedWekkResult = db->totalPlayTimeThisWeek(gameId);
    ui->timePlayedThisWeek->setText(util.SecondsToTime(timePlayedWekkResult));

    // Set lst time played for specific game
    ui->lastTimePlayedText->setText(gamesResult[0].updatedAt);

    // Set total time played today for specific game
    ui->timePlayedTodayText->setText(util.SecondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

    delete db;

    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btnStartGame_clicked()
{
    // Add new game_historical
    static const QString path = "crono.db";
    // Instance db conn
    DbManager *db = new DbManager(path);

    // If record dosn't exists for today creates new record
    if (db->getTodayGameHistorical(gameIdValue) == 0)
    {
        db->insertGameHistorical(gameIdValue);
    }

    delete db;

    on_btnPlay_clicked(gameNameValue, gameIdValue, gameExePathValue, gameExeValue);
}

void MainWindow::on_btnPlay_clicked(QString gameName, int gameId, QString gameExePath, QString gameExe)
{
    // Update running
    static const QString path = "crono.db";
    // Instance db conn
    DbManager *db = new DbManager(path);

    ui->btnStartGame->setDisabled(true); // Disable click butoon when game running to prevent open game again
    ui->btnStartGame->setText("RUNNING");
    ui->btnStartGame->setStyleSheet("QPushButton {"
                                    "    background-color: rgb(46, 125, 50);"
                                    "    font: 900 9pt 'Arial Black';"
                                    "    color: rgb(255, 255, 255);"
                                    "    border: 0px;"
                                    "    border-radius: 10px;"
                                    "    border-style: outset;"
                                    "    width: 100px;" // Adjust the width value as needed
                                    "    height: 20px;" // Adjust the height value as needed
                                    "}"
                                    "QPushButton::hover {"
                                    "    background-color: rgb(33, 78, 203);"
                                    "    font: 900 9pt 'Arial Black';"
                                    "    color: rgb(255, 255, 255);"
                                    "    border: 0px;"
                                    "}"
                                    "QPushButton::focus:pressed {"
                                    "    background-color: rgb(38, 72, 184);"
                                    "    font: 900 9pt 'Arial Black';"
                                    "    color: rgb(255, 255, 255);"
                                    "    border: 0px;"
                                    "}");

    Util util;

    QVector<DbManager::Games> gamesResult = db->getGameById(gameId);

    // Set total time played
    ui->timePlayedText->setText(util.SecondsToTime(gamesResult[0].timePlayed));

    // Set total time played today for specific game
    ui->timePlayedTodayText->setText(util.SecondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

    ui->stackedWidget->setCurrentIndex(2);

    ui->gameNameText->sizeIncrement();
    ui->gameNameText->setText(gameName.toUpper());

    // Create or update crono_runner.bat
    bool fileCration = util.CreateCronoRunnerBatFile(gameExePath, gameExe);

    if (fileCration)
    {
        // Executes game gameExePath
        QProcess::startDetached("crono_runner.bat", QStringList());
        // qDebug() <<  gameExePath;
    }

    bool updateRunning = db->updateGameRunning(gameId, true);

    if (updateRunning)
    {
        qDebug() << "is running updated to true. " << gameId;
    }

    delete db;

    // Waits 1 minute so it can let the game start
    // This is becouse some games takes up to 30-60 secons to start
    QTimer timerDelay;
    QEventLoop loop;

    // Create a lambda function to execute after the delay
    QObject::connect(&timerDelay, &QTimer::timeout, [&]()
                     {
                         qDebug() << "One minute has passed.";
                         loop.quit(); // Exit the event loop
                     });

    // Start the timer with a delay of one minute (60000 milliseconds)
    timerDelay.singleShot(60000, &loop, &QEventLoop::quit);

    // Start the event loop
    loop.exec();

    // Cron job to call checkRunningGame every 1 minute
    timer = new QTimer(this);
    timer->stop();

    // Disconnect any previous connection of the timeout signal
    disconnect(timer, &QTimer::timeout, nullptr, nullptr);

    // Create a lambda function to connect to the timeout signal
    auto timerFunction = [this, gameId, gameExe]()
    {
        checkRunningGame(gameId, gameExe);
    };

    // Connect the timer's timeout signal to the lambda function
    connect(timer, &QTimer::timeout, this, timerFunction);

    // Start the timer initially (30000 milliseconds) 30 seconds
    timer->start(30000);
}

void MainWindow::checkRunningGame(int gameId, QString gameName)
{
    QString processNameToCheck = gameName;

    static const QString path = "crono.db";
    // Instance db conn
    DbManager *db = new DbManager(path);

    Util util;
    if (util.IsProcessRunning(processNameToCheck))
    {
        QVector<DbManager::Games> gamesResult = db->getGameById(gameId);
        // qDebug() << gamesResult[0].gameName;

        // Set total time played
        ui->timePlayedText->setText(util.SecondsToTime(gamesResult[0].timePlayed));

        // Set total time played today for specific game
        ui->timePlayedTodayText->setText(util.SecondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

        // Gets time played this week filter by game id
        ui->timePlayedThisWeek->setText(util.SecondsToTime(db->totalPlayTimeThisWeek(gamesResult[0].id)));

        // Adds 30 seconds to timePlayed
        bool updateTime = db->updateTimePlayed(gameId, gamesResult[0].timePlayed + 30);

        if (updateTime)
        {
            qDebug() << "Process" << processNameToCheck << "is running. " << gameId;
            // Update running
            bool updateRunning = db->updateGameRunning(gameId, true);

            if (gamesResult[0].running)
            {
                // Get game_historical timePlayed
                DbManager::GameHistorical playedTime = db->getGameHistoricalToday(gamesResult[0].id);
                // Update game_historical timePlayed
                db->updateGameHistoricalTimePlayed(playedTime.id, playedTime.timePlayed + 30);

                ui->btnStartGame->setDisabled(true); // Disable click butoon when game running to prevent open game again
                ui->btnStartGame->setText("RUNNING");
                ui->btnStartGame->setStyleSheet("QPushButton {"
                                                "    background-color: rgb(46, 125, 50);"
                                                "    font: 900 9pt 'Arial Black';"
                                                "    color: rgb(255, 255, 255);"
                                                "    border: 0px;"
                                                "    border-radius: 10px;"
                                                "    border-style: outset;"
                                                "    width: 100px;" // Adjust the width value as needed
                                                "    height: 20px;" // Adjust the height value as needed
                                                "}"
                                                "QPushButton::hover {"
                                                "    background-color: rgb(33, 78, 203);"
                                                "    font: 900 9pt 'Arial Black';"
                                                "    color: rgb(255, 255, 255);"
                                                "    border: 0px;"
                                                "}"
                                                "QPushButton::focus:pressed {"
                                                "    background-color: rgb(38, 72, 184);"
                                                "    font: 900 9pt 'Arial Black';"
                                                "    color: rgb(255, 255, 255);"
                                                "    border: 0px;"
                                                "}");

                if (updateRunning)
                {
                    qDebug() << "is running updated to true. " << gameId;
                }
            }

            // Gets games when timePlayed is updated
            GetGame();
        }
        else
        {
            qDebug() << "Process" << processNameToCheck << "is not running.";
            timer->stop(); // Stop the timer

            // Update running
            bool updateRunning = db->updateGameRunning(gameId, false);

            if (updateRunning)
            {
                qDebug() << "is running updated to false." << gameId;
            }

            // Gets games to update running
            GetGame();

            ui->btnStartGame->setCursor(Qt::PointingHandCursor);
            ui->btnStartGame->setDisabled(false); // Anables PLAY button
            ui->btnStartGame->setText("PLAY");
            ui->btnStartGame->setStyleSheet("QPushButton {"
                                            "    background-color: transparent;"
                                            "    font: 900 9pt 'Arial Black';"
                                            "    color: rgb(255, 255, 255);"
                                            "    border: 1px;"
                                            "	border-color: rgb(255, 255, 255);"
                                            "    border-radius: 10px;"
                                            "    border-style: outset;"
                                            "    width: 100px;" // Adjust the width value as needed
                                            "    height: 20px;" // Adjust the height value as needed
                                            "}"
                                            "QPushButton::hover {"
                                            "    background-color: rgb(252, 196, 25);"
                                            "    font: 900 9pt 'Arial Black';"
                                            "    color: rgb(255, 255, 255);"
                                            "    border: 0px;"
                                            "    cursor: pointer;"
                                            "}"
                                            "QPushButton::focus:pressed {"
                                            "    background-color: rgb(252, 72, 25);"
                                            "    font: 900 9pt 'Arial Black';"
                                            "    color: rgb(255, 255, 255);"
                                            "    border: 0px;"
                                            "}");
        }
    }
    else
    {
        timer->stop(); // Stop the timer
        ui->btnStartGame->setCursor(Qt::PointingHandCursor);
        ui->btnStartGame->setDisabled(false); // Anables PLAY button
        ui->btnStartGame->setText("PLAY");
        ui->btnStartGame->setStyleSheet("QPushButton {"
                                        "    background-color: transparent;"
                                        "    font: 900 9pt 'Arial Black';"
                                        "    color: rgb(255, 255, 255);"

                                        "    border: 1px;"
                                        "	border-color: rgb(255, 255, 255);"
                                        "    border-radius: 10px;"
                                        "    border-style: outset;"
                                        "    width: 100px;" // Adjust the width value as needed
                                        "    height: 20px;" // Adjust the height value as needed
                                        "}"
                                        "QPushButton::hover {"
                                        "    background-color: rgb(252, 196, 25);"
                                        "    font: 900 9pt 'Arial Black';"
                                        "    color: rgb(255, 255, 255);"
                                        "    border: 0px;"
                                        "    cursor: pointer;"
                                        "}"
                                        "QPushButton::focus:pressed {"
                                        "    background-color: rgb(252, 72, 25);"
                                        "    font: 900 9pt 'Arial Black';"
                                        "    color: rgb(255, 255, 255);"
                                        "    border: 0px;"
                                        "}");
    }

    delete db;
}

void MainWindow::on_btnClose_clicked()
{
    this->close();
}

void MainWindow::on_minimizeBtn_clicked()
{
    this->showMinimized();
}

void MainWindow::on_maxBtn_clicked()
{
    if (this->isMaximized())
    {
        this->showNormal(); // Restore the window to its normal size
        numCols = 4;        // Sets row to 4 when minimized
        GetGame();          // Refresh data
    }
    else
    {
        this->showMaximized(); // Maximize the window
        numCols = 8;           // Sets row to 8 when Maximize
        GetGame();             // Refresh data
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    startPos = event->pos();
    QWidget::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - startPos;
    QWidget *w = window();
    if (w)
        w->move(w->pos() + delta);
    QWidget::mouseMoveEvent(event);
}

void MainWindow::on_btnGames_clicked()
{
    ui->btnGames->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 2px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->btnHltb->setStyleSheet("QPushButton {"
                               " text-align: left;"
                               " padding-left: 50px;"
                               "border-left-color: rgb(255, 255, 255);"
                               "border-left: 0px rgb(252, 196, 25);"
                               "background-color:  transparent;"
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(255, 255, 255);"
                               "border-style: outset;"
                               "}"
                               "QPushButton:hover{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}"

                               "QPushButton::focus:pressed{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}");

    ui->statsBtn->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_statsBtn_clicked()
{
    Stats receiver;

    connect(this, &MainWindow::RefreshStats, &receiver, &Stats::RefreshStats);

    emit RefreshStats();

    ui->btnGames->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->btnHltb->setStyleSheet("QPushButton {"
                               " text-align: left;"
                               " padding-left: 50px;"
                               "border-left-color: rgb(255, 255, 255);"
                               "border-left: 0px rgb(252, 196, 25);"
                               "background-color:  transparent;"
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(255, 255, 255);"
                               "border-style: outset;"
                               "}"
                               "QPushButton:hover{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}"

                               "QPushButton::focus:pressed{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}");

    ui->statsBtn->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 2px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btnHltb_clicked()
{
    ui->btnGames->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->statsBtn->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 50px;"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(252, 196, 25);"
                                "background-color:  transparent;"
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 11pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->btnHltb->setStyleSheet("QPushButton {"
                               " text-align: left;"
                               " padding-left: 50px;"
                               "border-left-color: rgb(255, 255, 255);"
                               "border-left: 2px rgb(252, 196, 25);"
                               "background-color:  transparent;"
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(255, 255, 255);"
                               "border-style: outset;"
                               "}"
                               "QPushButton:hover{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}"

                               "QPushButton::focus:pressed{ "
                               "font: 900 11pt 'Arial Black';"
                               "color: rgb(163, 163, 163);"
                               "border: 0px;"
                               "}");

    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_btnBack_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::DeleteGame(int gameId, QString gameName)
{
    qDebug() << gameId;

    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::FramelessWindowHint);

    msgBox.setStyleSheet("QMessageBox{background-color: rgba(33, 31, 29, 90);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    msgBox.setText("¿Are you shure you want to delete " + gameName + "? ");

    // Add custom buttons
    QPushButton *cancelButton = msgBox.addButton(" Cancel ", QMessageBox::ActionRole);
    QPushButton *yesButton = msgBox.addButton(" Yes ", QMessageBox::ActionRole);

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

    cancelButton->setStyleSheet("QPushButton {"
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
        // qDebug() << "YES";
        static const QString path = "crono.db";

        // Instance db conn
        DbManager *db = new DbManager(path);

        bool deleteGameResult = db->DeleteGame(gameId);

        if (!deleteGameResult)
        {
            qDebug() << "Error deleting game";
        }

        bool deleteGameHistoricalResult = db->DeleteGameHistorical(gameId);

        if (!deleteGameHistoricalResult)
        {
            qDebug() << "Error deleting game historical";
        }

        delete db;

        // Get games
        GetGame();

        ui->stackedWidget->setCurrentIndex(0);
    }
    else if (msgBox.clickedButton() == cancelButton)
    {
        qDebug() << "NO";
    }
}

void MainWindow::UpdateGame(int gameId, QString gameName, QString gameExePath)
{
    UpdateGameForm *updateForm = new UpdateGameForm(gameId, gameName, gameExePath);

    connect(updateForm, &UpdateGameForm::GameUpdated, this, &MainWindow::GetGame);

    updateForm->show();
    // ui->stackedWidget->setCurrentIndex(0);
}

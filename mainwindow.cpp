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
#include <QPainter>
#include <QPainterPath>
#include <QLabel>

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
    getGame(true);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Disable the vertical header (row index counter)
    // Remove the header
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);

    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QMenu *menu = new QMenu();

    menu->setStyleSheet("QMenu{background-color: rgba(33, 31, 29, 100); color: white;}");
    QAction *actionEdit = new QAction("Edit", this);
    menu->addAction(actionEdit);

    connect(actionEdit, &QAction::triggered, [=]()
            { MainWindow::updateGame(gameIdValue, gameNameValue, gameExePathValue); });

    QAction *actionDelete = new QAction("Delete", this);
    menu->addAction(actionDelete);

    connect(actionDelete, &QAction::triggered, [=]()
            { MainWindow::deleteGame(gameIdValue, gameNameValue); });

    ui->toolButton->setMenu(menu);
    ui->toolButton->setPopupMode(QToolButton::InstantPopup);

    initialFlags = ui->timePlayedTodayText->windowFlags();
    initialPosition = ui->timePlayedTodayText->pos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// NewGame form dialog
void MainWindow::on_addGameBtn_clicked()
{
    NewGame *newGame = new NewGame(this);

    connect(newGame, &NewGame::gameAdded, this, &MainWindow::addedGame);

    newGame->show();
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void MainWindow::addedGame(const QString &gameName, const QString &gameExePath)
{
    Util *util = new Util();

    static const QString path = "crono.db";

    // Get image url
    QString imageUrl = util->getGameImage(gameName);

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Inser into games table
    db->insertGame(imageUrl, gameName, util->removeDataFromLasBackSlash(gameExePath), util->findLastBackSlashWord(gameExePath.toStdString()));

    delete db;

    // Get games and start of the app
    getGame(true);
}

void MainWindow::getGame(bool goToGamesPage)
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
    ui->tableWidget->setIconSize(QSize(200, 280));

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
            gameDataVariant.setValue(gamesResult[col]);

            // Set the QVariant containing game data as user data for the QTableWidgetItem
            // This is done using the Qt::UserRole constant, which is a role for custom data
            item->setData(Qt::UserRole, gameDataVariant);

            // Set the item in the table widget
            ui->tableWidget->setItem(0, col, item);

            /* // Create a QTableWidgetItem to hold the widget
            QTableWidgetItem *item = new QTableWidgetItem();
            ui->tableWidget->setItem(0, col, item);

            // Create a QFrame to hold the QLabel (and potentially other widgets)
            QFrame *frame = new QFrame();
            frame->setFixedSize(188, 290);

            // Create a QLabel to display text
            QLabel *label = new QLabel(gamesResult[col].gameName.toUpper(), frame);

            // Enable word wrap for the label to allow text to wrap to new lines
            label->setWordWrap(true);

            // Set text color to white
            label->setStyleSheet("font: 900 8pt 'Arial Black'; color: white;");

            // Center-align the text
            label->setAlignment(Qt::AlignCenter);

            // Create a layout for the frame
            QVBoxLayout *layout = new QVBoxLayout(frame);
            layout->addWidget(label);

            // Set the background color using QSS (Qt Style Sheet)
            frame->setStyleSheet("margin-top: 5px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #222222, stop: 1 #1E1E1E); border-radius: 10px;");

            // Set the size policy for the QLabel to make it expand as needed
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            // Create a QVariant to hold the game data for the current column
            QVariant gameDataVariant;

            gameDataVariant.setValue(gamesResult[col]);

            // Set the QVariant containing game data as user data for the QTableWidgetItem
            item->setData(Qt::UserRole, gameDataVariant);

            // Set the widget in the QTableWidgetItem
            ui->tableWidget->setCellWidget(0, col, frame);*/
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

                                   // Create a rounded QPixmap
                                   QPixmap roundedPixmap(pixmap.size()); // Create a QPixmap with the same size as the original pixmap
                                   roundedPixmap.fill(Qt::transparent);  // Fill the QPixmap with a transparent background

                                   // Create a QPainter to draw on the roundedPixmap
                                   QPainter painter(&roundedPixmap);

                                   // Enable anti-aliasing for smoother edges
                                   painter.setRenderHint(QPainter::Antialiasing, true);

                                   // Create a QPainterPath for defining the rounded rectangle shape
                                   QPainterPath path;

                                   // Add a rounded rectangle to the path
                                   // The parameters are (x, y, width, height, x-radius, y-radius)
                                   path.addRoundedRect(0, 0, pixmap.width(), pixmap.height(), 15, 15);
                                   // Adjust the corner radius (10, 10) as needed to control the roundness of corners

                                   // Set the clipping path for the QPainter to the rounded rectangle shape
                                   painter.setClipPath(path);

                                   // Draw the original pixmap onto the roundedPixmap, respecting the clipping path
                                   painter.drawPixmap(0, 0, pixmap);

                                   // Create a QTableWidgetItem and set the rounded image as its icon
                                   QTableWidgetItem *imageItem = new QTableWidgetItem();

                                   // Set the icon of the QTableWidgetItem to the rounded QPixmap
                                   imageItem->setIcon(QIcon(roundedPixmap));

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
            goToGame(gameData.gameName, gameData.id, gameData.gameExePath, gameData.gameExe);
        } });

    if (goToGamesPage)
    {
        // Return to games view
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::goToGame(QString gameName, int gameId, QString gameExePath, QString gameExe)
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

    Util *util = new Util();
    // Set total time played
    ui->timePlayedText->setText(util->secondsToTime(gamesResult[0].timePlayed));

    ui->gameNameText->setText(gameName.toUpper());

    // Gets time played filter by game id
    int timePlayedResult = db->totalPlayTime(gameId);
    ui->timePlayedText->setText(util->secondsToTime(timePlayedResult));

    // Gets time played this week filter by game id
    int timePlayedWekkResult = db->totalPlayTimeThisWeek(gameId);
    ui->timePlayedThisWeek->setText(util->secondsToTime(timePlayedWekkResult));

    // Set lst time played for specific game
    ui->lastTimePlayedText->setText(gamesResult[0].updatedAt);

    // Set total time played today for specific game
    ui->timePlayedTodayText->setText(util->secondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

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

    Util *util = new Util();

    QVector<DbManager::Games> gamesResult = db->getGameById(gameId);

    // Set total time played
    ui->timePlayedText->setText(util->secondsToTime(gamesResult[0].timePlayed));

    // Set total time played today for specific game
    ui->timePlayedTodayText->setText(util->secondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

    ui->stackedWidget->setCurrentIndex(2);

    ui->gameNameText->sizeIncrement();
    ui->gameNameText->setText(gameName.toUpper());

    // Create or update crono_runner.bat
    bool fileCration = util->createCronoRunnerBatFile(gameExePath, gameExe);

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

    Util *util = new Util();
    if (util->isProcessRunning(processNameToCheck))
    {
        QVector<DbManager::Games> gamesResult = db->getGameById(gameId);
        // qDebug() << gamesResult[0].gameName;

        // Set total time played
        ui->timePlayedText->setText(util->secondsToTime(gamesResult[0].timePlayed));

        // Set total time played today for specific game
        ui->timePlayedTodayText->setText(util->secondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

        // Gets time played this week filter by game id
        ui->timePlayedThisWeek->setText(util->secondsToTime(db->totalPlayTimeThisWeek(gamesResult[0].id)));

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
        getGame(false);     // Refresh data
    }
    else
    {
        this->showMaximized(); // Maximize the window
        numCols = 8;           // Sets row to 8 when Maximize
        getGame(false);        // Refresh data
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

    connect(this, &MainWindow::refreshStats, &receiver, &Stats::refreshStats);

    emit refreshStats();

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

void MainWindow::deleteGame(int gameId, QString gameName)
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

        bool deleteGameResult = db->deleteGame(gameId);

        if (!deleteGameResult)
        {
            qDebug() << "Error deleting game";
        }

        bool deleteGameHistoricalResult = db->deleteGameHistorical(gameId);

        if (!deleteGameHistoricalResult)
        {
            qDebug() << "Error deleting game historical";
        }

        delete db;

        // Get games
        getGame(true);
    }
    else if (msgBox.clickedButton() == cancelButton)
    {
        qDebug() << "NO";
    }
}

void MainWindow::updateGame(int gameId, QString gameName, QString gameExePath)
{
    UpdateGameForm *updateForm = new UpdateGameForm(gameId, gameName, gameExePath);

    connect(updateForm, &UpdateGameForm::gameUpdated, this, [this]()
            { MainWindow::getGame(true); });

    updateForm->show();
    // ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_radioBtnTimeIndicator_clicked()
{
    if (ui->radioBtnTimeIndicator->isChecked())
    {
        QLabel *timeIndicator = ui->timePlayedTodayText;

        timeIndicator->setAttribute(Qt::WA_TranslucentBackground);
        timeIndicator->setStyleSheet("QLabel { color: yellow; background: transparent; }");
        timeIndicator->setFont(QFont("Arial", 15));

        // Set QLabel properties
        int x = 0; // X-coordinate
        int y = 0; // Y-coordinate
        timeIndicator->move(x, y);

        // Set the QLabel to always stay on top
        timeIndicator->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

        // Show the QLabel
        timeIndicator->show();
    }
    else
    {
        QLabel *timeIndicator = ui->timePlayedTodayText;
        timeIndicator->setStyleSheet("QLabel { background-color: transparent; color: rgb(255, 255, 255); font: 900 13pt 'Arial Black';}");
        timeIndicator->setWindowFlags(initialFlags);
        timeIndicator->move(initialPosition);

        // Show the QLabel
        timeIndicator->show();
    }
}

void MainWindow::on_reloadBtn_clicked()
{
    getGame(false);
}

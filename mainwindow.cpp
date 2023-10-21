#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newgame.h"
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
#include <QStatusBar>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MainWindow::setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    MainWindow::checkActiveTheme();

    // Install the event filter to capture mouse events for resizing
    this->installEventFilter(this);

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Sets all running column to false at start of the app
    db->updateAllGameRunning();

    // Get games and start of the app
    getGame(true, false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Disable the vertical header (row index counter)
    // Remove the header
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);

    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);


    initialFlags = ui->timePlayedTodayText->windowFlags();
    initialPosition = ui->timePlayedTodayText->pos();

    // hoursPlayedPerdayTheLastWeek

    // Set the fixed width for the horizontal header
    ui->tableWidget_2->horizontalHeader()->setDefaultSectionSize(145);

    // Set the fixed height for the vertical header
    ui->tableWidget_2->verticalHeader()->setDefaultSectionSize(145);

    ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);

    // Action menu
    MainWindow::actionsMenu();

    delete db;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actionsMenu(){
    // Instance db conn
    DbManager *db = new DbManager(path);

    // Gets achivements
    QVector<DbManager::Achivements> achivementsData = db->getAchivements();

    QMenu *menu = new QMenu();

    menu->setStyleSheet("QMenu{background-color: rgba(44, 44, 44, 100); color: white;}");

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

    // Get image url
    QString imageUrl = util->getGameImage(gameName);

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Inser into games table
    db->insertGame(imageUrl, gameName, util->removeDataFromLasBackSlash(gameExePath), util->findLastBackSlashWord(gameExePath.toStdString()));

    delete db;

    // Set the status bar style
    statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

    statusBar()->show();

    // Create a QTimer to hide the message after 3 seconds
    QTimer::singleShot(3000, statusBar(), [this]()
                       {
                           statusBar()->clearMessage(); // Clear the message after 3 seconds
                           statusBar()->hide();         // Hide the status bar
                       });

    // Show the status message
    statusBar()->showMessage(tr("Game added."));

    // Get games and start of the app
    getGame(true, false);
}

void MainWindow::getGame(bool goToGamesPage, bool updatedGame)
{
    // Reset tableWidget row count so it won´t create a copy of last item when deleting game
    // This also prevents to repeat items when maximazing app
    ui->tableWidget->setRowCount(0);

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
    connect(ui->tableWidget, &QTableWidget::cellClicked, [=](int row, int col){

            // Retrieve the QTableWidgetItem at the specified row and column
            QTableWidgetItem* item = ui->tableWidget->item(row, col);

            // Check if the item is not null (i.e., it exists) and if it has a user role
            if (item && item->data(Qt::UserRole).isValid()) {
                // The item has a user role set, and it's valid
                QVariant gameDataVariant = item->data(Qt::UserRole);

                // Now you can safely retrieve and use the user role data
                DbManager::Games gameData = gameDataVariant.value<DbManager::Games>();
                goToGame(gameData.gameName, gameData.id, gameData.gameExePath, gameData.gameExe);
            } else {
                // Handle the case where the item does not have a user role set
                // or it does not exist (is null)
                qDebug() << "No data found";
            }
    });

    if (updatedGame)
    {
        // Set the status bar style
        statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

        statusBar()->show();

        // Create a QTimer to hide the message after 3 seconds
        QTimer::singleShot(3000, statusBar(), [this]()
                           {
                               statusBar()->clearMessage(); // Clear the message after 3 seconds
                               statusBar()->hide();         // Hide the status bar
                           });

        // Show the status message
        statusBar()->showMessage(tr("Game updated."));
    }

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

    // Renders Hours Played Per Day Of The Last Week Table
    MainWindow::hoursPlayedPerDayForTheLastWeekTable(gameId);

    delete db;

    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::hoursPlayedPerDayForTheLastWeekTable(int gameId)
{
    // Update running
    // Instance db conn
    DbManager *db = new DbManager(path);

    Util *util = new Util();

    // Sets tableWidget row count
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Disable the vertical header (row index counter)
    // Remove the header
    ui->tableWidget_2->horizontalHeader()->setVisible(true);
    ui->tableWidget_2->verticalHeader()->setVisible(false);
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_2->setShowGrid(false);

    ui->tableWidget_2->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->tableWidget_2->setRowCount(1);
    ui->tableWidget_2->setColumnCount(8);
    // ui->tableWidget_2->scrollToTop();

    // Sets table cell height
    ui->tableWidget_2->setRowHeight(0, 10);

    // Get Hours Played Per Day Of The Last Week Data
    QVector<DbManager::HoursPlayedPerDayOfTheLastWeekData> hoursPlayedPerDayOfTheLastWeekResult = db->hoursPlayedPerDayOfTheLastWeek(gameId);

    QVector<QString> days;

    for (int idx = 0; idx < hoursPlayedPerDayOfTheLastWeekResult.count(); ++idx)
    {
        // qDebug() << "---------------- " << idx;

        days.push_back(util->dayNumberToWeekDay(hoursPlayedPerDayOfTheLastWeekResult[idx].dayOfWeek));

        QTableWidgetItem *item = new QTableWidgetItem(util->secondsToTime(hoursPlayedPerDayOfTheLastWeekResult[idx].totalTimePlayed));
        // Create a QFont to set the text style
        QFont font;
        font.setBold(true);       // Make the text bold
        font.setItalic(false);    // Make the text italic
        font.setUnderline(false); // Underline the text
        font.setPointSize(11);    // Set the font size to 14
        // Set the QFont for the item
        item->setFont(font);

        ui->tableWidget_2->setItem(0, idx, item);
        ui->tableWidget_2->item(0, idx)->setTextAlignment(Qt::AlignCenter);
    }

    // Set horizontal header labels
    QStringList horizontalHeaderLabels;
    horizontalHeaderLabels << days[0] << days[1] << days[2] << days[3] << days[4] << days[5] << days[6] << "TODAY";

    ui->tableWidget_2->setHorizontalHeaderLabels(horizontalHeaderLabels);
}

void MainWindow::on_btnStartGame_clicked()
{
    // Add new game_historical
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

    // Instance db conn
    DbManager *db = new DbManager(path);

    Util *util = new Util();
    if (util->isProcessRunning(processNameToCheck))
    {
        // Renders Hours Played Per Day Of The Last Week Table
        MainWindow::hoursPlayedPerDayForTheLastWeekTable(gameId);

        QVector<DbManager::Games> gamesResult = db->getGameById(gameId);
        // qDebug() << gamesResult[0].gameName;

        // Set total time played
        ui->timePlayedText->setText(util->secondsToTime(gamesResult[0].timePlayed));

        // Set total time played today for specific game
        ui->timePlayedTodayText->setText(util->secondsToTime(db->totalPlayTimeToday(gamesResult[0].id)));

        // Gets time played this week filter by game id
        ui->timePlayedThisWeek->setText(util->secondsToTime(db->totalPlayTimeThisWeek(gamesResult[0].id)));

        // Achivements
        if(static_cast<double>(db->totalTimePlayed()) / (60 * 60) > 1){
            bool silver = db->updateAchivement(1);
            if(silver){
                qDebug() << "Silver updated";

                // Set the status bar style
                statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

                statusBar()->show();

                // Create a QTimer to hide the message after 3 seconds
                QTimer::singleShot(3000, statusBar(), [this]()
                                   {
                                       statusBar()->clearMessage(); // Clear the message after 3 seconds
                                       statusBar()->hide();         // Hide the status bar
                                   });

                // Show the status message
                statusBar()->showMessage(tr("Achivement unlock Silver."));
            }
        }
        if(static_cast<double>(db->totalTimePlayed()) / (60 * 60) > 2){
            bool nova = db->updateAchivement(2);
            if(nova){
                qDebug() << "Nova updated";
                // Set the status bar style
                statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

                statusBar()->show();

                // Create a QTimer to hide the message after 3 seconds
                QTimer::singleShot(3000, statusBar(), [this]()
                                   {
                                       statusBar()->clearMessage(); // Clear the message after 3 seconds
                                       statusBar()->hide();         // Hide the status bar
                                   });

                // Show the status message
                statusBar()->showMessage(tr("Achivement unlock Nova."));
            }
        }
        if(static_cast<double>(db->totalTimePlayed()) / (60 * 60) > 3){
            bool platinum = db->updateAchivement(3);
            if(platinum){
                qDebug() << "Platinum updated";
                // Set the status bar style
                statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

                statusBar()->show();

                // Create a QTimer to hide the message after 3 seconds
                QTimer::singleShot(3000, statusBar(), [this]()
                                   {
                                       statusBar()->clearMessage(); // Clear the message after 3 seconds
                                       statusBar()->hide();         // Hide the status bar
                                   });

                // Show the status message
                statusBar()->showMessage(tr("Achivement unlock Platinum."));
            }
        }
        if(static_cast<double>(db->totalTimePlayed()) / (60 * 60) > 4){
            bool dimond = db->updateAchivement(4);
            if(dimond){
                qDebug() << "Diamond updated";
                // Set the status bar style
                statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

                statusBar()->show();

                // Create a QTimer to hide the message after 3 seconds
                QTimer::singleShot(3000, statusBar(), [this]()
                                   {
                                       statusBar()->clearMessage(); // Clear the message after 3 seconds
                                       statusBar()->hide();         // Hide the status bar
                                   });

                // Show the status message
                statusBar()->showMessage(tr("Achivement unlock Diamond."));
            }
        }

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
        this->showNormal();    // Restore the window to its normal size
        numCols = 5;           // Sets row to 4 when minimized
        getGame(false, false); // Refresh data
    }
    else
    {
        this->showMaximized(); // Maximize the window
        numCols = 8;           // Sets row to 8 when Maximize
        getGame(false, false); // Refresh data
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
                                " padding-left: 13px;"
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
                               " padding-left: 13px;"
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

    ui->btnAchivements->setStyleSheet("QPushButton {"
                               " text-align: left;"
                               " padding-left: 13px;"
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
                                " padding-left: 13px;"
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
                                " padding-left: 13px;"
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
                               " padding-left: 13px;"
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

    ui->btnAchivements->setStyleSheet("QPushButton {"
                                      " text-align: left;"
                                      " padding-left: 13px;"
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
                                " padding-left: 13px;"
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
                                " padding-left: 13px;"
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
                                " padding-left: 13px;"
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

    ui->btnAchivements->setStyleSheet("QPushButton {"
                                      " text-align: left;"
                                      " padding-left: 13px;"
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
                               " padding-left: 13px;"
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

    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Achivements> achivementsData = db->getAchivements();

    if(achivementsData[0].active){
        msgBox.setStyleSheet("QMessageBox{background-color: rgb(22, 24, 22);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    }

    if(achivementsData[1].active){
        msgBox.setStyleSheet("QMessageBox{background-color: rgb(40,42,40);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    }

    if(achivementsData[2].active){
        msgBox.setStyleSheet("QMessageBox{background-color: rgb(0, 70, 129);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    }

    if(achivementsData[3].active){
        msgBox.setStyleSheet("QMessageBox{background-color: rgb(34,60,64);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    }

    if(achivementsData[4].active){
        msgBox.setStyleSheet("QMessageBox{background-color: rgb(23, 113, 107);} QMessageBox QLabel {color: white; font: 900 10pt 'Arial Black';}");
    }
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

        // Set the status bar style
        statusBar()->setStyleSheet("color: #ffffff; background-color: #388E3C; font-size: 13px;");

        statusBar()->show();

        // Create a QTimer to hide the message after 3 seconds
        QTimer::singleShot(3000, statusBar(), [this]()
                           {
                               statusBar()->clearMessage(); // Clear the message after 3 seconds
                               statusBar()->hide();         // Hide the status bar
                           });

        // Show the status message
        statusBar()->showMessage(tr("Game deleted."));

        // Get games
        getGame(true, false);
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
            { MainWindow::getGame(true, true); });

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
    getGame(false, false);
}

void MainWindow::on_btnGithub_clicked()
{
    QUrl url("https://github.com/illud/crono");

    if (QDesktopServices::openUrl(url)) {
        qDebug() << "URL opened successfully";
    } else {
        qDebug() << "Failed to open the URL";
    }
}


void MainWindow::on_btnAchivements_clicked()
{
    ui->btnGames->setStyleSheet("QPushButton {"
                                " text-align: left;"
                                " padding-left: 13px;"
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
                                " padding-left: 13px;"
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
                                " padding-left: 13px;"
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

    ui->btnAchivements->setStyleSheet("QPushButton {"
                               " text-align: left;"
                               " padding-left: 13px;"
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


    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Achivements> achivementsData = db->getAchivements();

    if(achivementsData[1].unlocked == true){
        QIcon icon(":/silver.png");
        ui->btnSilver->setIcon(icon);
        ui->btnSilver->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnSilver->update();
        ui->silverTxt->setStyleSheet("color: white;");
        ui->btnSilver->setDisabled(false);

    }else{
        QIcon icon(":/silver0.png");
        ui->btnSilver->setIcon(icon);
        ui->btnSilver->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnSilver->update();
        ui->silverTxt->setStyleSheet("color: grey;");
        ui->btnSilver->setDisabled(true);
    }

    if(achivementsData[2].unlocked == true){
        ui->btnNova->setDisabled(false);
        QIcon icon(":/nova.png");
        ui->btnNova->setIcon(icon);
        ui->btnNova->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnNova->update();
        ui->goldTxt->setStyleSheet("color: white;");
    }else{
        ui->btnNova->setDisabled(true);
        QIcon icon(":/nova0.png");
        ui->btnNova->setIcon(icon);
        ui->btnNova->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnNova->update();
        ui->goldTxt->setStyleSheet("color: grey;");
    }

    if(achivementsData[3].unlocked == true){
        ui->btnPlatinum->setDisabled(false);
        QIcon icon(":/platinum.png");
        ui->btnPlatinum->setIcon(icon);
        ui->btnPlatinum->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnPlatinum->update();
        ui->platinumTxt->setStyleSheet("color: white;");
    }else{
        ui->btnPlatinum->setDisabled(true);
        QIcon icon(":/platinum0.png");
        ui->btnPlatinum->setIcon(icon);
        ui->btnPlatinum->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnPlatinum->update();
        ui->platinumTxt->setStyleSheet("color: grey;");
    }

    if(achivementsData[4].unlocked == true){
        ui->btnDiamond->setDisabled(false);
        QIcon icon(":/dimond.png");
        ui->btnDiamond->setIcon(icon);
        ui->btnDiamond->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnDiamond->update();
        ui->diamondTxt->setStyleSheet("color: white;");
    }else{
        ui->btnDiamond->setDisabled(true);
        QIcon icon(":/dimond0.png");
        ui->btnDiamond->setIcon(icon);
        ui->btnDiamond->setIconSize(icon.actualSize(ui->btnSilver->size()));
        ui->btnDiamond->update();
        ui->diamondTxt->setStyleSheet("color: grey;");
    }

    ui->stackedWidget->setCurrentIndex(4);
}


void MainWindow::on_btnCrono_clicked()
{
    //MainWindow::setStyleSheet("background-color: rgb(22, 22, 22)");

    MainWindow::setStyleSheet("QMainWindow {"
                              "background-image: url(:/bg.jpg);"
                              "background-repeat: no-repeat;"
                              "background-position: center;"
                              "background-size: 100% 100%;"
                              "}");

    // Instance db conn
    DbManager *db = new DbManager(path);

    db->updateActiveTheme(1);

    // Action menu
    MainWindow::actionsMenu();
}

void MainWindow::on_btnSilver_clicked()
{
    //MainWindow::setStyleSheet("background-color: rgb(40,40,40)");

    MainWindow::setStyleSheet("QMainWindow {"
                              "background-image: url(:/bg1.png);"
                              "background-repeat: no-repeat;"
                              "background-position: center;"
                              "background-size: 100% 100%;"
                              "}");

    // Instance db conn
    DbManager *db = new DbManager(path);

    db->updateActiveTheme(2);

    // Action menu
    MainWindow::actionsMenu();
}


void MainWindow::on_btnNova_clicked()
{
    //MainWindow::setStyleSheet("background-color: rgb(0, 68, 129)");

    MainWindow::setStyleSheet("QMainWindow {"
                              "background-image: url(:/bg2.jpg);"
                              "background-repeat: no-repeat;"
                              "background-position: center;"
                              "background-size: cover;"
                              "}");

    // Instance db conn
    DbManager *db = new DbManager(path);

    db->updateActiveTheme(3);

    // Action menu
    MainWindow::actionsMenu();
}


void MainWindow::on_btnPlatinum_clicked()
{
    //MainWindow::setStyleSheet("background-color: rgb(34,58,64)");

    MainWindow::setStyleSheet("QMainWindow {"
                              "background-image: url(:/bg3.jpg);"
                              "background-repeat: no-repeat;"
                              "background-position: center;"
                              "background-size: 100% 100%;"
                              "}");

    // Instance db conn
    DbManager *db = new DbManager(path);

    db->updateActiveTheme(4);

    // Action menu
    MainWindow::actionsMenu();
}


void MainWindow::on_btnDiamond_clicked()
{
    //MainWindow::setStyleSheet("background-color: rgb(23, 111, 107)");

    MainWindow::setStyleSheet("QMainWindow {"
                              "background-image: url(:/bg4.jpg);"
                              "background-repeat: no-repeat;"
                              "background-position: center;"
                              "background-size: 100% 100%;"
                              "}");

    // Instance db conn
    DbManager *db = new DbManager(path);

    db->updateActiveTheme(5);

    // Action menu
    MainWindow::actionsMenu();
}

void MainWindow::checkActiveTheme(){
    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Achivements> achivementsData = db->getAchivements();

    if(achivementsData[0].active){
       // MainWindow::setStyleSheet("background-color: rgb(22, 22, 22)");

        MainWindow::setStyleSheet("QMainWindow {"
                                  "background-image: url(:/bg.jpg);"
                                  "background-repeat: no-repeat;"
                                  "background-position: center;"
                                  "background-size: cover;"
                                  "}");
    }

    if(achivementsData[1].active){
        //MainWindow::setStyleSheet("background-color: rgb(40,40,40)");
        MainWindow::setStyleSheet("QMainWindow {"
                                  "background-image: url(:/bg1.png);"
                                  "background-repeat: no-repeat;"
                                  "background-position: center;"
                                  "background-size: 100% 100%;"
                                  "}");
    }

    if(achivementsData[2].active){
        //MainWindow::setStyleSheet("background-color: rgb(0, 68, 129)");
        MainWindow::setStyleSheet("QMainWindow {"
                                  "background-image: url(:/bg2.jpg);"
                                  "background-repeat: no-repeat;"
                                  "background-position: center;"
                                  "background-size: 100% 100%;"
                                  "}");
    }

    if(achivementsData[3].active){
        //MainWindow::setStyleSheet("background-color: rgb(34,58,64)");
        MainWindow::setStyleSheet("QMainWindow {"
                                  "background-image: url(:/bg3.jpg);"
                                  "background-repeat: no-repeat;"
                                  "background-position: center;"
                                  "background-size: 100% 100%;"
                                  "}");
    }

    if(achivementsData[4].active){
        //MainWindow::setStyleSheet("background-color: rgb(23, 111, 107)");
        MainWindow::setStyleSheet("QMainWindow {"
                                  "background-image: url(:/bg4.jpg);"
                                  "background-repeat: no-repeat;"
                                  "background-position: center;"
                                  "background-size: 100% 100%;"
                                  "}");
    }

}

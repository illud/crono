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

    // Get games and start of the app
    getGame();

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Disable the vertical header (row index counter)
    // Remove the header
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);

    ui->tableWidget->setVerticalScrollMode(QTableWidget::ScrollPerPixel);
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

QString MainWindow::getGameImage(QString gameName)
{
    Util util;
    QVector<QString> splitWords = util.removeDupWord(gameName.toStdString());

    // build a string by sequentially adding data to it.
    std::stringstream ss;

    for (int i = 0; i < splitWords.size(); ++i)
    {
        if (i > 0)
        {
            ss << ", ";
        }
        ss << "\"" << splitWords[i].toStdString() << "\""; // Include the " symbols around each element
    }

    std::string searchTerms = ss.str();

    // qDebug() << splitWords.data()->toStdString();
    // ui->lineEdit->text().toStdString()
    CURL *curl;
    CURLcode res;
    struct curl_slist *header;
    std::string readBuffer;
    std::string jsonstr = "{\"searchType\": \"games\",\"searchTerms\": [" + searchTerms + "],\"searchPage\": 1,\"size\": 20,\"searchOptions\": { \"games\": {\"userId\": 0,\"platform\": \"\",\"sortCategory\": \"popular\",\"rangeCategory\": \"main\",\"rangeTime\": { \"min\": 0, \"max\": 0},\"gameplay\": { \"perspective\": \"\", \"flow\": \"\", \"genre\": \"\"},\"modifier\": \"\" }, \"users\": {\"sortCategory\": \"postcount\" }, \"filter\": \"\", \"sort\": 0, \"randomizer\": 0} }";
    // qDebug() << jsonstr;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl)
    {

        header = NULL;
        header = curl_slist_append(header, "Content-Type: application/json");
        header = curl_slist_append(header, "Accept: */*");
        header = curl_slist_append(header, "Origin: https://howlongtobeat.com");
        header = curl_slist_append(header, "Referer: https://howlongtobeat.com");
        header = curl_slist_append(header, "User-Agent: Mozilla/4.0 (Windows 7 6.1) Java/1.7.0_51");

        /* First set the URL that is about to receive our POST. This URL can
       just as well be an https:// URL if that is what should receive the
       data. */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.howlongtobeat.com/api/search");

        /* Now specify the POST data */
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonstr.length());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res == CURLE_OK)
            qDebug() << "Good";
        else
            qDebug() << curl_easy_strerror((CURLcode)res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    // Parse JSON object
    QJsonDocument jsonResponse = QJsonDocument::fromJson(readBuffer.c_str());

    QString imageUrl = "https://howlongtobeat.com/img/hltb_brand2.png";

    QJsonArray jsonArray = jsonResponse["data"].toArray();

    // Check if theres data
    if (jsonResponse["data"].toArray().count() > 0)
    {
        imageUrl = "https://howlongtobeat.com/games/" + jsonArray[0].toObject()["game_image"].toString();
    }
    return imageUrl;
}

void MainWindow::addedGame(const QString &gameName, const QString &gameExePath)
{
    static const QString path = "crono.db";

    // Get image url
    QString imageUrl = getGameImage(gameName);

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Inser into games table
    Util util;

    db->insertGame(imageUrl, gameName, util.removeDataFromLasBackSlash(gameExePath), util.findLastBackSlashWord(gameExePath.toStdString()));
    QVector<DbManager::Games> gamesResult = db->getGames();

    // Get games and start of the app
    getGame();
}

int numCols = 4;
void MainWindow::getGame()
{
    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Games> gamesResult = db->getGames();

    // Sets tableWidget row count
    ui->tableWidget->setRowCount(gamesResult.count());

    // Sets icon size
    ui->tableWidget->setIconSize(QSize(300, 300));

    ui->tableWidget->setColumnCount(numCols);

    for (int col = 0; col < gamesResult.count(); ++col)
    {
        // Set the row height for a specific row
        ui->tableWidget->setRowHeight(col, 300);
        ui->tableWidget->viewport()->setCursor(Qt::PointingHandCursor);
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
                           });
    }

    // Connect the cellClicked signal of the table widget outside the loop
    connect(ui->tableWidget, &QTableWidget::cellClicked, [=](int row, int col)
            {
        // Load the image and do other operations (same code as before)

        // Call your custom function
        //on_btnPlay_clicked(gamesResult[col].gameName, gamesResult[col].id, gamesResult[col].gameExePath, gamesResult[col].gameExe);

        GoToGame(gamesResult[col].gameName, gamesResult[col].id, gamesResult[col].gameExePath, gamesResult[col].gameExe); });
}

QString gameNameValue;
int gameIdValue;
QString gameExePathValue;
QString gameExeValue;

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
    ui->timePlayedText->setText(util.secondsToTime(gamesResult[0].timePlayed));

    ui->gameNameText->setText(gameName.toUpper());

    // Gets time played filter by game id
    int timePlayedResult = db->totalPlayTime(gameId);
    ui->timePlayedText->setText(util.secondsToTime(timePlayedResult));
    qDebug() << "--------------" << timePlayedResult;
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btnStartGame_clicked()
{
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
    ui->timePlayedText->setText(util.secondsToTime(gamesResult[0].timePlayed));

    ui->stackedWidget->setCurrentIndex(2);

    ui->gameNameText->sizeIncrement();
    ui->gameNameText->setText(gameName.toUpper());

    // Create or update crono_runner.bat
    bool fileCration = util.createCronoRunnerBatFile(gameExePath, gameExe);

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

    // Gets games to update running
    getGame();

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
    if (util.isProcessRunning(processNameToCheck))
    {
        QVector<DbManager::Games> gamesResult = db->getGameById(gameId);
        // qDebug() << gamesResult[0].gameName;

        // Set total time played
        ui->timePlayedText->setText(util.secondsToTime(gamesResult[0].timePlayed));

        // Adds 30 seconds to timePlayed
        bool updateTime = db->updateTimePlayed(gameId, gamesResult[0].timePlayed + 30);

        if (updateTime)
        {
            qDebug() << "Process" << processNameToCheck << "is running. " << gameId;
            // Update running
            bool updateRunning = db->updateGameRunning(gameId, true);

            if (gamesResult[0].running)
            {
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
            getGame();
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
            getGame();

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
                                            "    background-color: rgb(33, 78, 203);"
                                            "    font: 900 9pt 'Arial Black';"
                                            "    color: rgb(255, 255, 255);"
                                            "    border: 0px;"
                                            "    cursor: pointer;"
                                            "}"
                                            "QPushButton::focus:pressed {"
                                            "    background-color: rgb(38, 72, 184);"
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
                                        "    background-color: rgb(33, 78, 203);"
                                        "    font: 900 9pt 'Arial Black';"
                                        "    color: rgb(255, 255, 255);"
                                        "    border: 0px;"
                                        "    cursor: pointer;"
                                        "}"
                                        "QPushButton::focus:pressed {"
                                        "    background-color: rgb(38, 72, 184);"
                                        "    font: 900 9pt 'Arial Black';"
                                        "    color: rgb(255, 255, 255);"
                                        "    border: 0px;"
                                        "}");
    }
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
        getGame();          // Refresh data
    }
    else
    {
        this->showMaximized(); // Maximize the window
        numCols = 8;           // Sets row to 8 when Maximize
        getGame();             // Refresh data
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
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 2px rgb(33, 78, 203);"
                                "background-color:  transparent;"
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->statsBtn->setStyleSheet("QPushButton {"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(33, 78, 203);"
                                "background-color:  transparent;"
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_statsBtn_clicked()
{
    ui->btnGames->setStyleSheet("QPushButton {"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 0px rgb(33, 78, 203);"
                                "background-color:  transparent;"
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->statsBtn->setStyleSheet("QPushButton {"
                                "border-left-color: rgb(255, 255, 255);"
                                "border-left: 2px rgb(33, 78, 203);"
                                "background-color:  transparent;"
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(255, 255, 255);"
                                "border-style: outset;"
                                "}"
                                "QPushButton:hover{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}"

                                "QPushButton::focus:pressed{ "
                                "font: 900 9pt 'Arial Black';"
                                "color: rgb(163, 163, 163);"
                                "border: 0px;"
                                "}");

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btnBack_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

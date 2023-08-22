#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newgame.h"
#include <QProcess>
#include <curl/curl.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <sstream>
#include <QTimer>
#include "ImageUtil.h"
#include "dbmanager.h"
#ifdef Q_OS_WIN // Windows-specific code
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Get games and start of the app
    getGame();
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

QVector<QString> MainWindow::RemoveDupWord(std::string str)
{
    QVector<QString> words;
    // Used to split string around spaces.
    std::istringstream  ss(str);

    std::string word; // for storing each word

    // Traverse through all wordsF
    // while loop till we get
    // strings to store in string word
    while (ss >> word)
    {
        // print the read word
        words.push_back(word.c_str());
    }

    return words;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

QString MainWindow::getGameImage(QString gameName){
    QVector<QString> splitWords = RemoveDupWord(gameName.toStdString());

    // build a string by sequentially adding data to it.
    std::stringstream ss;

    for (int i = 0; i < splitWords.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << "\"" << splitWords[i].toStdString() << "\""; // Include the " symbols around each element
    }

    std::string searchTerms = ss.str();

    //qDebug() << searchTerms;

    //qDebug() << splitWords.data()->toStdString();
    //ui->lineEdit->text().toStdString()
    CURL *curl;
    CURLcode res;
    struct curl_slist *header;
    std::string readBuffer;
    std::string jsonstr =  "{\"searchType\": \"games\",\"searchTerms\": ["+searchTerms+"],\"searchPage\": 1,\"size\": 20,\"searchOptions\": { \"games\": {\"userId\": 0,\"platform\": \"\",\"sortCategory\": \"popular\",\"rangeCategory\": \"main\",\"rangeTime\": { \"min\": 0, \"max\": 0},\"gameplay\": { \"perspective\": \"\", \"flow\": \"\", \"genre\": \"\"},\"modifier\": \"\" }, \"users\": {\"sortCategory\": \"postcount\" }, \"filter\": \"\", \"sort\": 0, \"randomizer\": 0} }";
    //qDebug() << jsonstr;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {

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
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.howlongtobeat.com/api/search");

        /* Now specify the POST data */
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonstr.length());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res == CURLE_OK)
            qDebug() << "Good" ;
        else
            qDebug() << curl_easy_strerror((CURLcode)res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    //Parse JSON object
    QJsonDocument jsonResponse = QJsonDocument::fromJson(readBuffer.c_str());

    QString imageUrl = "https://howlongtobeat.com/img/hltb_brand2.png";

    QJsonArray jsonArray = jsonResponse["data"].toArray();

    //Check if theres data
    if (jsonResponse["data"].toArray().count() > 0) {
        imageUrl = "https://howlongtobeat.com/games/" + jsonArray[0].toObject()["game_image"].toString();
    }
    return imageUrl;
}

//Games struct
/*struct Games{
    int id;
    QString gameImage;
    QString gameName;
    QString gameExePath;
};*/

//QVector<Games> games;


QString MainWindow::secondsToTime(int time){
    int h = time / 3600;
    int m = time % 3600 / 60;

    QString hours = QString::number(h);

    QString minutes = QString::number(m);

    return hours + "h " + minutes + "m";
}

void MainWindow::addedGame(const QString &gameName, const QString &gameExePath){
    static const QString path = "crono.db";

    // Get image url
    QString imageUrl = getGameImage(gameName);

    // Push data to vector of games
    //games.push_back(Games{static_cast<long>(games.count()) ,imageUrl, gameName, gameExePath});

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Inser into games table
    db->insertGame(imageUrl, gameName, gameExePath);
    QVector<DbManager::Games> gamesResult = db->getGames();

    //qDebug() <<  gamesResult[0].gameName;

    //qDebug() <<  games.count();
     //Sets tableWidget row count
    int currentRow = 0;
    //Sets tableWidget row count
    ui->tableWidget->setRowCount(gamesResult.count());

    // Sets icon size
    ui->tableWidget->setIconSize(QSize(300, 300));

    for (int gamesList = 0; gamesList < gamesResult.count(); gamesList++ ) {
         //qDebug()<< gamesList;

        // Set the row height for a specific row
        ui->tableWidget->setRowHeight(gamesList, 300);

        // Download image from url and set image as icon
        ImageUtil* imageUtil = new ImageUtil();
        imageUtil->loadFromUrl(QUrl(gamesResult[gamesList].gameImage));
        imageUtil->connect(imageUtil, &ImageUtil::loaded,
                            [=]() {
                                QImage image = imageUtil->image(); // Get the image from ImageUtil

                                // Convert QImage to QPixmap for display
                                QPixmap pixmap = QPixmap::fromImage(image);

                                // Create a QTableWidgetItem and set the image as its icon
                                QTableWidgetItem *imageItem = new QTableWidgetItem();

                                // Sets icon
                                imageItem->setIcon(QIcon(pixmap));

                                // Set the item in the table widget
                                ui->tableWidget->setItem(currentRow, 0, imageItem);
                            });

        ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(gamesResult[gamesList].gameName));
        //ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(gamesResult[gamesList].gameExePath));
        ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(secondsToTime(gamesResult[gamesList].timePlayed)));

        // CELL BUTTON
        QPushButton* button = new QPushButton();
        button->setText("PLAY");
        button->setStyleSheet("QPushButton {    background-color: rgb(41, 98, 255);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;	border-radius: 10px;	border-style: outset;}QPushButton::hover{     background-color: rgb(33, 78, 203);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;}QPushButton::focus:pressed{ 	background-color: rgb(38, 72, 184);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;}");

        //Sets button property to identify button
        button->setProperty("gameExePath", gamesResult[gamesList].gameExePath);

        //Adds button to current index row
        ui->tableWidget->setCellWidget(currentRow, 3, button);

        //c++ 11 Lambda to call  on_btnPlay_clicked() function with gameExePath parameter to identify tableWidget row
        connect(button, &QPushButton::clicked, [this, button, gamesList, gamesResult](){
            on_btnPlay_clicked(gamesResult[gamesList].id , button->property("gameExePath").toString());
        });

        //Increases currentRow
        currentRow = currentRow + 1;
    }
    //qDebug() <<  gameName  <<  gameExePath;
}

void MainWindow::getGame(){
    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    QVector<DbManager::Games> gamesResult = db->getGames();

    // If theres games renders tableWidget
    if(gamesResult.count() != 0){
    //Sets tableWidget row count
    int currentRow = 0;
    //Sets tableWidget row count
    ui->tableWidget->setRowCount(gamesResult.count());

    // Sets icon size
    ui->tableWidget->setIconSize(QSize(300, 300));

    for (int gamesList = 0; gamesList < gamesResult.count(); gamesList++ ) {
        //qDebug()<< gamesList;

        // Set the row height for a specific row
        ui->tableWidget->setRowHeight(gamesList, 300);

        // Download image from url and set image as icon
        ImageUtil* imageUtil = new ImageUtil();
        imageUtil->loadFromUrl(QUrl(gamesResult[gamesList].gameImage));
        imageUtil->connect(imageUtil, &ImageUtil::loaded,
                           [=]() {
                               QImage image = imageUtil->image(); // Get the image from ImageUtil

                               // Convert QImage to QPixmap for display
                               QPixmap pixmap = QPixmap::fromImage(image);

                               // Create a QTableWidgetItem and set the image as its icon
                               QTableWidgetItem *imageItem = new QTableWidgetItem();

                               // Sets icon
                               imageItem->setIcon(QIcon(pixmap));

                               // Set the item in the table widget
                               ui->tableWidget->setItem(currentRow, 0, imageItem);
                           });

        ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(gamesResult[gamesList].gameName));
        //ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(gamesResult[gamesList].gameExePath));
        ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(secondsToTime(gamesResult[gamesList].timePlayed)));

        // CELL BUTTON
        QPushButton* button = new QPushButton();
        button->setText("PLAY");
        button->setStyleSheet("QPushButton {    background-color: rgb(41, 98, 255);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;	border-radius: 10px;	border-style: outset;}QPushButton::hover{     background-color: rgb(33, 78, 203);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;}QPushButton::focus:pressed{ 	background-color: rgb(38, 72, 184);	font: 900 9pt 'Arial Black';	color: rgb(255, 255, 255);    border: 0px;}");

        //Sets button property to identify button
        button->setProperty("gameExePath", gamesResult[gamesList].gameExePath);

        //Adds button to current index row
        ui->tableWidget->setCellWidget(currentRow, 3, button);

        //c++ 11 Lambda to call  on_btnPlay_clicked() function with gameExePath parameter to identify tableWidget row
        connect(button, &QPushButton::clicked, [this, button, gamesList, gamesResult](){
            on_btnPlay_clicked(gamesResult[gamesList].id , button->property("gameExePath").toString());
        });

        //Increases currentRow
        currentRow = currentRow + 1;
    }
    //qDebug() <<  gameName  <<  gameExePath;
    }
}

void MainWindow::on_btnPlay_clicked(int gameId, QString gameExePath){
    // Executes game
    QProcess::startDetached(gameExePath, QStringList());
    //qDebug() <<  gameExePath;

    QTimer* timer = new QTimer(this);
    timer->stop();
    // Create a lambda function to connect to the timeout signal
    auto timerFunction = [this, gameId]() {
        checkRunningGame(gameId, "crono.exe");
    };

    // Connect the timer's timeout signal to the lambda function
    connect(timer, &QTimer::timeout, this, timerFunction);

    // Start the timer initially (10000 milliseconds)
    timer->start(3000); // Start with a 10-second interval
}

void MainWindow::checkRunningGame(int gameId,QString gameName){
    QString processNameToCheck = gameName;

    if (isProcessRunning(processNameToCheck)) {
        static const QString path = "crono.db";
        // Instance db conn
        DbManager *db = new DbManager(path);

        QVector<DbManager::Games> gamesResult = db->getGameById(gameId);
        qDebug() << gamesResult[0].gameName;
        bool updateTime = db->updateTimePlayed(gameId, gamesResult[0].timePlayed + 30);
        if(updateTime){
            qDebug() << "Process" << processNameToCheck << "is running. " << gameId;
        }
        getGame();

    } else {
        qDebug() << "Process" << processNameToCheck << "is not running.";
    }
}

bool MainWindow::isProcessRunning(const QString &processName) {
    QProcess process;
#ifdef Q_OS_WIN
    process.start("tasklist");
#else
    process.start("ps", QStringList() << "aux");
#endif
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QString outputStr = QString::fromLocal8Bit(output);

    return outputStr.contains(processName, Qt::CaseInsensitive);
}

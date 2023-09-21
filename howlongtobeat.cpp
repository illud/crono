#include "howlongtobeat.h"
#include "ui_howlongtobeat.h"
#include <QAbstractItemView>
#include <QTableWidget>
#include <QHeaderView>
#include "dbmanager.h"
#include <charconv>
#include <curl/curl.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <sstream>
#include <QDesktopServices>
#include "ImageUtil.h"
#include "util.h"

HowLongTobeat::HowLongTobeat(QWidget *parent) : QWidget(parent),
                                                ui(new Ui::HowLongTobeat)
{
    ui->setupUi(this);

    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    // Sets all running column to false at start of the app
    db->updateAllGameRunning();
    delete db;

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Remove the header
    // ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

}

HowLongTobeat::~HowLongTobeat()
{
    delete ui;
}

size_t WriteCallbackHowLongTobeat(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void HowLongTobeat::on_btnSearch_clicked()
{
    Util *util = new Util();

    QVector<QString> splitWords = util->removeDupWord(ui->searchBox->text().toStdString());

    // Build a JSON array of search terms
    QJsonArray searchTermsArray;
    for (const QString &word : splitWords)
    {
        searchTermsArray.append(word);
    }
    QJsonObject searchObject;
    searchObject["searchType"] = "games";
    searchObject["searchTerms"] = searchTermsArray;
    searchObject["searchPage"] = 1;
    searchObject["size"] = 20;
    QJsonObject searchOptionsObject;
    QJsonObject gamesObject;
    gamesObject["userId"] = 0;
    gamesObject["platform"] = "";
    gamesObject["sortCategory"] = "popular";
    gamesObject["rangeCategory"] = "main";
    QJsonObject rangeTimeObject;
    rangeTimeObject["min"] = 0;
    rangeTimeObject["max"] = 0;
    gamesObject["rangeTime"] = rangeTimeObject;
    QJsonObject gameplayObject;
    gameplayObject["perspective"] = "";
    gameplayObject["flow"] = "";
    gameplayObject["genre"] = "";
    gamesObject["gameplay"] = gameplayObject;
    gamesObject["modifier"] = "";
    searchOptionsObject["games"] = gamesObject;
    QJsonObject usersObject;
    usersObject["sortCategory"] = "postcount";
    searchOptionsObject["users"] = usersObject;
    searchObject["searchOptions"] = searchOptionsObject;
    searchObject["filter"] = "";
    searchObject["sort"] = 0;
    searchObject["randomizer"] = 0;
    QJsonDocument jsonDocument(searchObject);
    QString jsonstr = QString(jsonDocument.toJson(QJsonDocument::Compact));

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://www.howlongtobeat.com/api/search"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Origin", "https://howlongtobeat.com");
    request.setRawHeader("Referer", "https://howlongtobeat.com");
    request.setRawHeader("User-Agent", "Mozilla/4.0 (Windows 7 6.1) Java/1.7.0_51");

    QByteArray postData = jsonstr.toUtf8();
    QNetworkReply *reply = manager.post(request, postData);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QString imageUrl = "";

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonArray jsonArray = jsonResponse["data"].toArray();

        // Sets tableWidget row count
        ui->tableWidget->setRowCount(jsonArray.count());

        // Sets icon size
        ui->tableWidget->setIconSize(QSize(200, 280));

        int currentRow = 0;
        for (int var = 0; var < jsonArray.count(); ++var)
        {
            // Set the row height for a specific row
            ui->tableWidget->setRowHeight(var, 300);

            // Download image from url and set image as icon
            ImageUtil *imageUtil = new ImageUtil();
            imageUtil->loadFromUrl(QUrl("https://howlongtobeat.com/games/" + jsonArray[var].toObject()["game_image"].toString()));
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
                                   ui->tableWidget->setItem(currentRow, 0, imageItem);

                                   imageUtil->deleteLater();
                               });

            // ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem("https://howlongtobeat.com/games/" + jsonArray[var].toObject()["game_image"].toString()));
            // ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(jsonArray[var].toObject()["game_name"].toString()));
            // ui->tableWidget->item(currentRow, 1)->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(util->secondsToTime(jsonArray[var].toObject()["comp_main"].toInt())));
            ui->tableWidget->item(currentRow, 1)->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(util->secondsToTime(jsonArray[var].toObject()["comp_plus"].toInt())));
            ui->tableWidget->item(currentRow, 2)->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget->setItem(currentRow, 3, new QTableWidgetItem(util->secondsToTime(jsonArray[var].toObject()["comp_100"].toInt())));
            ui->tableWidget->item(currentRow, 3)->setTextAlignment(Qt::AlignCenter);

            currentRow = currentRow + 1;
        }
    }
    else
    {
        qDebug() << "Error: " << reply->errorString();
    }

    reply->deleteLater();
}


#include "util.h"
#include "qdebug.h"
#include "qeventloop.h"
#include "qnetworkreply.h"
#include "qtcpsocket.h"
#include <sstream>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QTcpSocket>
#include <curl/curl.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QHttpMultiPart>
#include <QObject>

Util::Util()
{
}

QVector<QString> Util::removeDupWord(std::string str)
{
    QVector<QString> words;
    // Used to split string around spaces.
    std::istringstream ss(str);

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

// Converts secons to hours and minutes
QString Util::secondsToTime(int time)
{
    int h = time / 3600;
    int m = time % 3600 / 60;

    QString hours = QString::number(h);

    QString minutes = QString::number(m);

    return hours + "h " + minutes + "m";
}

// Finds game exe name (game.exe)
QString Util::findLastBackSlashWord(std::string path)
{
    auto const pos = path.find_last_of("\\");
    const auto leaf = path.substr(pos + 1);

    return leaf.c_str();
}

// Removes last back slash
QString Util::removeDataFromLasBackSlash(QString filePath)
{
    // Find the index of the last backslash
    int lastIndex = filePath.lastIndexOf("\\");

    if (lastIndex != -1)
    {
        // Extract the part of the string before the last backslash
        QString newPath = filePath.left(lastIndex + 1);

        return newPath;
    }

    return "Error";
}

// Creates .bat file containing game exe location
bool Util::createCronoRunnerBatFile(QString gameExePath, QString gameExe)
{
    // Specify the file name/path
    QString fileName = "crono_runner.bat";

    // Create a QFile instance
    QFile file(fileName);

    // Open the file in write mode
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Create a QTextStream to write to the file
        QTextStream out(&file);

        // Write text to the file
        out << "start /d "
               "\""
            << gameExePath << "\" " << gameExe << Qt::endl;

        // Close the file
        file.close();

        return true;
    }
    else
    {
        // Handle error if the file couldn't be opened
        qDebug() << "Failed to open the file for writing.";
        return false;
    }
}

// Check if game is running
bool Util::isProcessRunning(const QString &processName)
{
    QProcess process;
#ifdef Q_OS_WIN
    process.start("tasklist");
#else
    process.start("ps", QStringList() << "aux");
#endif
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QString outputStr = QString::fromLocal8Bit(output);

    process.deleteLater();
    return outputStr.contains(processName, Qt::CaseInsensitive);
}

size_t Util::writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

size_t WriteCall(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Checks internet connection making a ping to google.com
bool Util::checkInternetConn()
{
    QTcpSocket *sock = new QTcpSocket();
    sock->connectToHost("www.google.com", 80);
    bool connected = sock->waitForConnected(30000); // ms

    if (!connected)
    {
        sock->abort();
        return false;
    }
    sock->close();
    return true;
}

QString Util::getGameImage(QString gameName)
{
    QVector<QString> splitWords = Util::removeDupWord(gameName.toStdString());

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
        if (jsonArray.count() > 0)
        {
            imageUrl = "https://howlongtobeat.com/games/" + jsonArray[0].toObject()["game_image"].toString();
        }
    }
    else
    {
        qDebug() << "Error: " << reply->errorString();
    }

    reply->deleteLater();
    return imageUrl;
}

/*QString Util::getGameImage(QString gameName)
{
    QVector<QString> splitWords = Util::removeDupWord(gameName.toStdString());

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

    // In windows, this will init the winsock stuff
    curl_global_init(CURL_GLOBAL_ALL);

    // get a curl handle
    curl = curl_easy_init();
    if (curl)
    {

        header = NULL;
        header = curl_slist_append(header, "Content-Type: application/json");
        header = curl_slist_append(header, "Accept: **");
        header = curl_slist_append(header, "Origin: https://howlongtobeat.com");
        header = curl_slist_append(header, "Referer: https://howlongtobeat.com");
        header = curl_slist_append(header, "User-Agent: Mozilla/4.0 (Windows 7 6.1) Java/1.7.0_51");

        // First set the URL that is about to receive our POST. This URL can
       //just as well be an https:// URL if that is what should receive the
      // data.
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.howlongtobeat.com/api/search");

        // Now specify the POST data
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonstr.length());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCall);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);
        // Check for errors
        if (res == CURLE_OK)
            qDebug() << "Good";
        else
            qDebug() << curl_easy_strerror((CURLcode)res);

        // always cleanup
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    // Parse JSON object
    QJsonDocument jsonResponse = QJsonDocument::fromJson(readBuffer.c_str());

    QString imageUrl = "";

    QJsonArray jsonArray = jsonResponse["data"].toArray();

    // Check if theres data
    if (jsonResponse["data"].toArray().count() > 0)
    {
        imageUrl = "https://howlongtobeat.com/games/" + jsonArray[0].toObject()["game_image"].toString();
    }
     return imageUrl;
}*/

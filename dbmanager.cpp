#include "dbmanager.h"
#include "qdatetime.h"
#include <QDebug>

DbManager::DbManager(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    m_db.setDatabaseName(path);

    allTables = new tablesAndColumns();

    if (!m_db.open())
    {
        qDebug() << "Error: connection with database fail";
    }
    else
    {
        qDebug() << "Database: connection ok";
        bool success = true;

        QSqlQuery query;
        query.prepare("CREATE TABLE games(id INTEGER PRIMARY KEY AUTOINCREMENT,gameName TEXT, gameImage TEXT, gameExePath TEXT, gameExe TEXT, running BOOLEAN, timePlayed INTEGER, createdAt TEXT, updatedAt TEXT);");

        if (!query.exec())
        {
            qDebug() << "Table already games created.";
            success = false;
        }

        QSqlQuery queryGameHistorical;
        queryGameHistorical.prepare("CREATE TABLE game_historical(id INTEGER PRIMARY KEY AUTOINCREMENT,gameId INTEGER, timePlayed INTEGER, createdAt TEXT, updatedAt TEXT);");

        if (!queryGameHistorical.exec())
        {
            qDebug() << "Table already game_historical created.";
        }
    }
}

DbManager::~DbManager()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool DbManager::insertGame(const QString gameImage, const QString &gameName, const QString &gameExePath, const QString &gameExe)
{
    bool success = false;

    if (!gameName.isEmpty())
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO games (gameImage,gameName,gameExePath,gameExe,running,timePlayed,createdAt, updatedAt) VALUES (:gameImage,:gameName,:gameExePath,:gameExe,:running,:timePlayed,:createdAt,:updatedAt)");
        queryAdd.bindValue(":gameImage", gameImage);
        queryAdd.bindValue(":gameName", gameName);
        queryAdd.bindValue(":gameExePath", gameExePath);
        queryAdd.bindValue(":gameExe", gameExe);
        queryAdd.bindValue(":running", false);
        queryAdd.bindValue(":timePlayed", 0);

        QDate currentDate = QDate::currentDate();
        int year = currentDate.year();
        int month = currentDate.month();
        int day = currentDate.day();
        QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

        queryAdd.bindValue(":createdAt", formattedDate);
        queryAdd.bindValue(":updatedAt", formattedDate);

        if (queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "Could not game: " << queryAdd.lastError();
        }
    }
    else
    {
        qDebug() << "Data is required to add.";
    }

    return success;
}

bool DbManager::UpdateGame(const QString gameImage, const QString &gameName, const QString &gameExePath, const QString &gameExe, const int &gameId)
{
    bool success = false;

    if (!gameName.isEmpty())
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("UPDATE games SET gameImage = :gameImage, gameName = :gameName, gameExePath = :gameExePath, gameExe = :gameExe, updatedAt = :updatedAt WHERE id = :gameId");
        queryAdd.bindValue(":gameImage", gameImage);
        queryAdd.bindValue(":gameName", gameName);
        queryAdd.bindValue(":gameExePath", gameExePath);
        queryAdd.bindValue(":gameExe", gameExe);

        QDate currentDate = QDate::currentDate();
        int year = currentDate.year();
        int month = currentDate.month();
        int day = currentDate.day();
        QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

        queryAdd.bindValue(":updatedAt", formattedDate);
        queryAdd.bindValue(":gameId", gameId);

        if (queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "Could update game: " << queryAdd.lastError();
        }
    }
    else
    {
        qDebug() << "Data is required to update.";
    }

    return success;
}

// Games struct
struct Games
{
    int id;
    QString gameImage;
    QString gameName;
    QString gameExePath;
    QString gameExe;
    bool running;
    int timePlayed;
    QString createdAt;
    QString updatedAt;
};

QVector<DbManager::Games> DbManager::getGames()
{
    QVector<Games> games;

    QSqlQuery query("SELECT * FROM games ORDER BY id DESC");
    int idIndex = query.record().indexOf("id");
    int gameImageIndex = query.record().indexOf("gameImage");
    int gameNameIndex = query.record().indexOf("gameName");
    int gameExePathIndex = query.record().indexOf("gameExePath");
    int gameExeIndex = query.record().indexOf("gameExe");
    int runningIndex = query.record().indexOf("running");
    int timePlayedIndex = query.record().indexOf("timePlayed");
    int createdAtIndex = query.record().indexOf("createdAt");
    int updatedAtIndex = query.record().indexOf("updatedAt");

    while (query.next())
    {
        int id = query.value(idIndex).toInt();
        QString gameImage = query.value(gameImageIndex).toString();
        QString gameName = query.value(gameNameIndex).toString();
        QString gameExePath = query.value(gameExePathIndex).toString();
        QString gameExe = query.value(gameExeIndex).toString();
        bool running = query.value(runningIndex).toBool();
        int timePlayed = query.value(timePlayedIndex).toInt();
        QString createdAt = query.value(createdAtIndex).toString();
        QString updatedAt = query.value(updatedAtIndex).toString();

        games.push_back(Games{id, gameImage, gameName, gameExePath, gameExe, running, timePlayed, createdAt, updatedAt});
    }

    return games;
}

QVector<DbManager::Games> DbManager::getGameById(int gameId)
{
    QVector<Games> games;

    QSqlQuery query;
    query.prepare("SELECT * FROM games WHERE id = :gameId ORDER BY id DESC");
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        int idIndex = query.record().indexOf("id");
        int gameImageIndex = query.record().indexOf("gameImage");
        int gameNameIndex = query.record().indexOf("gameName");
        int gameExePathIndex = query.record().indexOf("gameExePath");
        int gameExeIndex = query.record().indexOf("gameExe");
        bool runningIndex = query.record().indexOf("running");
        int timePlayedIndex = query.record().indexOf("timePlayed");
        int createdAtIndex = query.record().indexOf("createdAt");
        int updatedAtIndex = query.record().indexOf("updatedAt");

        while (query.next())
        {
            int id = query.value(idIndex).toInt();
            QString gameImage = query.value(gameImageIndex).toString();
            QString gameName = query.value(gameNameIndex).toString();
            QString gameExePath = query.value(gameExePathIndex).toString();
            QString gameExe = query.value(gameExeIndex).toString();
            bool running = query.value(runningIndex).toBool();
            int timePlayed = query.value(timePlayedIndex).toInt();
            QString createdAt = query.value(createdAtIndex).toString();
            QString updatedAt = query.value(updatedAtIndex).toString();

            games.push_back(Games{id, gameImage, gameName, gameExePath, gameExe, running, timePlayed, createdAt, updatedAt});
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return games;
}

bool DbManager::updateTimePlayed(int gameId, int timePlayed)
{
    bool success = false;

    if (timePlayed != 0)
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("UPDATE games SET timePlayed = :timePlayed, updatedAt = :updatedAt  WHERE id = :id");
        queryAdd.bindValue(":timePlayed", timePlayed);
        queryAdd.bindValue(":id", gameId);

        QDate currentDate = QDate::currentDate();
        int year = currentDate.year();
        int month = currentDate.month();
        int day = currentDate.day();
        QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

        queryAdd.bindValue(":updatedAt", formattedDate);

        if (queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "Error: " << queryAdd.lastError();
        }
    }
    return success;
}

bool DbManager::updateGameRunning(int gameId, bool running)
{
    bool success = false;

    if (gameId != 0)
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("UPDATE games SET running = :running WHERE id = :id");
        queryAdd.bindValue(":running", running);
        queryAdd.bindValue(":id", gameId);

        if (queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "Error: " << queryAdd.lastError();
        }
    }
    return success;
}

void DbManager::updateAllGameRunning()
{
    QSqlQuery queryAdd;
    queryAdd.prepare("UPDATE games SET running = false");

    if (queryAdd.exec())
    {
        qDebug() << "Success";
    }
    else
    {
        qDebug() << "Error: " << queryAdd.lastError();
    }
}

int DbManager::totalTimePlayed()
{
    int totalTimePlayedResult = 0;

    QSqlQuery query;
    query.prepare("SELECT SUM(timePlayed) as totalTimePlayed FROM games");

    if (query.exec())
    {
        int totalTimePlayedIndex = query.record().indexOf("totalTimePlayed");

        while (query.next())
        {
            int totalTimePlayed = query.value(totalTimePlayedIndex).toInt();

            totalTimePlayedResult = totalTimePlayed;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return totalTimePlayedResult;
}

// Gets total play time for specific game
int DbManager::totalPlayTime(int gameId)
{
    int totalTimePlayedResult = 0;

    QSqlQuery query;
    query.prepare("SELECT timePlayed FROM games WHERE id = :gameId");
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        int totalTimePlayedIndex = query.record().indexOf("timePlayed");

        while (query.next())
        {
            int totalTimePlayed = query.value(totalTimePlayedIndex).toInt();

            totalTimePlayedResult = totalTimePlayed;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return totalTimePlayedResult;
}

bool DbManager::insertGameHistorical(const int gameId)
{
    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();
    QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    bool success = false;

    if (gameId != 0)
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO game_historical (gameId,timePlayed,createdAt,updatedAt) VALUES (:gameId,:timePlayed,:createdAt,:updatedAt)");
        queryAdd.bindValue(":gameId", gameId);
        queryAdd.bindValue(":timePlayed", 0);
        queryAdd.bindValue(":createdAt", formattedDate);
        queryAdd.bindValue(":updatedAt", formattedDate);

        if (queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "Could not insert into game_historical: " << queryAdd.lastError();
        }
    }
    else
    {
        qDebug() << "Data is required to add.";
    }

    return success;
}

DbManager::GameHistorical DbManager::getGameHistoricalToday(int gameId)
{
    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();
    QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    DbManager::GameHistorical gameH;

    QSqlQuery query;
    query.prepare("SELECT id, timePlayed FROM game_historical WHERE gameId = :gameId AND createdAt = :createdAt");
    query.bindValue(":gameId", gameId);
    query.bindValue(":createdAt", formattedDate);

    if (query.exec())
    {
        int id = query.record().indexOf("id");
        int result = query.record().indexOf("timePlayed");

        while (query.next())
        {
            int idResult = query.value(id).toInt();
            int totalResult = query.value(result).toInt();

            gameH = {idResult, gameId, totalResult, "", ""};
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return gameH;
}

int DbManager::getTodayGameHistorical(int gameId)
{
    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();
    QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    int total = 0;

    QSqlQuery query;
    query.prepare("SELECT COUNT(id) AS total FROM game_historical WHERE gameId = :gameId AND createdAt = :createdAt");
    query.bindValue(":gameId", gameId);
    query.bindValue(":createdAt", formattedDate);

    if (query.exec())
    {
        int result = query.record().indexOf("total");

        while (query.next())
        {
            int totalResult = query.value(result).toInt();

            total = totalResult;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return total;
}

void DbManager::updateGameHistoricalTimePlayed(int gameHistoricalId, int timePlayed)
{
    QSqlQuery queryAdd;
    queryAdd.prepare("UPDATE game_historical SET timePlayed = :timePlayed WHERE id = :gameHistoricalId");
    queryAdd.bindValue(":timePlayed", timePlayed);
    queryAdd.bindValue(":gameHistoricalId", gameHistoricalId);

    if (queryAdd.exec())
    {
        qDebug() << "Success";
    }
    else
    {
        qDebug() << "Error: " << queryAdd.lastError();
    }
}

// Gets total play time for today
int DbManager::totalPlayTimeToday(int gameId)
{
    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();
    QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    int totalTimePlayedResult = 0;

    QSqlQuery query;
    query.prepare("SELECT SUM(timePlayed) AS timePlayed FROM game_historical WHERE createdAt = :createdAt AND gameId = :gameId");
    query.bindValue(":createdAt", formattedDate);
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        int totalTimePlayedIndex = query.record().indexOf("timePlayed");

        while (query.next())
        {
            int totalTimePlayed = query.value(totalTimePlayedIndex).toInt();

            totalTimePlayedResult = totalTimePlayed;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return totalTimePlayedResult;
}

// Gets total play time for this week
int DbManager::totalPlayTimeThisWeek(int gameId)
{

    QDate currentDate = QDate::currentDate();

    QDate lastWeekDate = currentDate.addDays(-7);
    int lastWeekYear = lastWeekDate.year();
    int lastWeekMonth = lastWeekDate.month();
    int lastWeekDay = lastWeekDate.day();
    QString lastWeek = QString("%1-%2-%3").arg(lastWeekYear).arg(lastWeekMonth, 2, 10, QChar('0')).arg(lastWeekDay, 2, 10, QChar('0'));

    QDate tomorrowDate = currentDate.addDays(1);
    int year = tomorrowDate.year();
    int month = tomorrowDate.month();
    int day = tomorrowDate.day();
    QString today = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    int totalTimePlayedResult = 0;

    QSqlQuery query;
    query.prepare("SELECT SUM(timePlayed) AS total FROM game_historical WHERE gameId = :gameId AND createdAt > :lastWeek AND createdAt < :today");
    query.bindValue(":lastWeek", lastWeek);
    query.bindValue(":today", today);
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        int totalTimePlayedIndex = query.record().indexOf("total");

        while (query.next())
        {
            int totalTimePlayed = query.value(totalTimePlayedIndex).toInt();

            totalTimePlayedResult = totalTimePlayed;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return totalTimePlayedResult;
}

DbManager::MostPlayGame DbManager::mostPlayedGame()
{

    DbManager::MostPlayGame gameResult;

    QSqlQuery query;
    query.prepare("SELECT gameName, MAX(timePlayed) AS total FROM games");

    if (query.exec())
    {
        int gameName = query.record().indexOf("gameName");
        int total = query.record().indexOf("total");

        while (query.next())
        {
            QString gameNameResult = query.value(gameName).toString();
            int totalResult = query.value(total).toInt();

            gameResult = {gameNameResult, totalResult};
        }
    }
    else
    {
        qDebug() << "Error executing query int mostPlayedGame func:" << query.lastError().text();
    }

    return gameResult;
}

int DbManager::totalTimePlayedToday()
{
    int totalResponse = 0;

    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();
    QString formattedDate = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    QSqlQuery query;
    query.prepare("SELECT SUM(timePlayed) AS total FROM game_historical WHERE createdAt = :createdAt");
    query.bindValue(":createdAt", formattedDate);

    if (query.exec())
    {
        int total = query.record().indexOf("total");

        while (query.next())
        {
            int totalResult = query.value(total).toInt();

            totalResponse = totalResult;
        }
    }
    else
    {
        qDebug() << "Error executing query int mostPlayedGame func:" << query.lastError().text();
    }

    return totalResponse;
}

// Gets total play time filter by days
int DbManager::TimePlayedFilter(int days)
{

    QDate currentDate = QDate::currentDate();

    QDate lastWeekDate = currentDate.addDays(-days);
    int lastWeekYear = lastWeekDate.year();
    int lastWeekMonth = lastWeekDate.month();
    int lastWeekDay = lastWeekDate.day();
    QString lastWeek = QString("%1-%2-%3").arg(lastWeekYear).arg(lastWeekMonth, 2, 10, QChar('0')).arg(lastWeekDay, 2, 10, QChar('0'));

    QDate tomorrowDate = currentDate.addDays(1);
    int year = tomorrowDate.year();
    int month = tomorrowDate.month();
    int day = tomorrowDate.day();
    QString today = QString("%1-%2-%3").arg(year).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0'));

    int totalTimePlayedResult = 0;

    QSqlQuery query;
    query.prepare("SELECT SUM(timePlayed) AS total FROM game_historical WHERE createdAt > :lastWeek AND createdAt < :today");
    query.bindValue(":lastWeek", lastWeek);
    query.bindValue(":today", today);

    if (query.exec())
    {
        int totalTimePlayedIndex = query.record().indexOf("total");

        while (query.next())
        {
            int totalTimePlayed = query.value(totalTimePlayedIndex).toInt();

            totalTimePlayedResult = totalTimePlayed;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return totalTimePlayedResult;
}

bool DbManager::DeleteGame(int gameId)
{

    QSqlQuery query;
    query.prepare("DELETE FROM games WHERE id = :gameId");
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        while (query.next())
        {
            return true;
        }
    }
    else
    {
        qDebug() << "Error executing query int DeleteGame func:" << query.lastError().text();
        return false;
    }

    return false;
}

bool DbManager::DeleteGameHistorical(int gameId)
{

    QSqlQuery query;
    query.prepare("DELETE FROM game_historical WHERE gameId = :gameId");
    query.bindValue(":gameId", gameId);

    if (query.exec())
    {
        while (query.next())
        {
            return true;
        }
    }
    else
    {
        qDebug() << "Error executing query int DeleteGameHistorical func:" << query.lastError().text();
        return false;
    }

    return false;
}

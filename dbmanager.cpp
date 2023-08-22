#include "dbmanager.h"
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
        query.prepare("CREATE TABLE games(id INTEGER PRIMARY KEY AUTOINCREMENT,gameName TEXT, gameImage TEXT, gameExePath TEXT, gameExe TEXT, running BOOLEAN, timePlayed INTEGER);");


        if (!query.exec())
        {
            qDebug() << "Table already created.";
            success = false;
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

bool DbManager::insertGame(const QString gameImage,const QString &gameName, const QString &gameExePath, const QString &gameExe)
{
    bool success = false;

    if (!gameName.isEmpty())
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO games (gameImage,gameName,gameExePath,gameExe,running,timePlayed) VALUES (:gameImage,:gameName,:gameExePath,:gameExe,:running,:timePlayed)");
        queryAdd.bindValue(":gameImage", gameImage);
        queryAdd.bindValue(":gameName", gameName);
        queryAdd.bindValue(":gameExePath", gameExePath);
        queryAdd.bindValue(":gameExe", gameExe);
        queryAdd.bindValue(":running", false);
        queryAdd.bindValue(":timePlayed", 0);

        if(queryAdd.exec())
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

//Games struct
struct Games{
    int id;
    QString gameImage;
    QString gameName;
    QString gameExePath;
    QString gameExe;
    bool running;
    int timePlayed;
};

QVector<DbManager::Games> DbManager::getGames() {
    QVector<Games> games;

    QSqlQuery query("SELECT * FROM games ORDER BY id DESC");
    int idIndex = query.record().indexOf("id");
    int gameImageIndex = query.record().indexOf("gameImage");
    int gameNameIndex = query.record().indexOf("gameName");
    int gameExePathIndex = query.record().indexOf("gameExePath");
    int gameExeIndex = query.record().indexOf("gameExe");
    int runningIndex = query.record().indexOf("running");
    int timePlayedIndex = query.record().indexOf("timePlayed");

    while (query.next()) {
        int id = query.value(idIndex).toInt();
        QString gameImage = query.value(gameImageIndex).toString();
        QString gameName = query.value(gameNameIndex).toString();
        QString gameExePath = query.value(gameExePathIndex).toString();
        QString gameExe = query.value(gameExeIndex).toString();
        bool running = query.value(runningIndex).toBool();
        int timePlayed = query.value(timePlayedIndex).toInt();

        games.push_back(Games{id, gameImage, gameName, gameExePath, gameExe, running, timePlayed});
    }

    return games;
}

QVector<DbManager::Games> DbManager::getGameById(int gameId) {
    QVector<Games> games;

    QSqlQuery query;
    query.prepare("SELECT * FROM games WHERE id = :gameId ORDER BY id DESC");
    query.bindValue(":gameId", gameId);

    if (query.exec()) {
        int idIndex = query.record().indexOf("id");
        int gameImageIndex = query.record().indexOf("gameImage");
        int gameNameIndex = query.record().indexOf("gameName");
        int gameExePathIndex = query.record().indexOf("gameExePath");
        int gameExeIndex = query.record().indexOf("gameExe");
        bool runningIndex = query.record().indexOf("running");
        int timePlayedIndex = query.record().indexOf("timePlayed");

        while (query.next()) {
            int id = query.value(idIndex).toInt();
            QString gameImage = query.value(gameImageIndex).toString();
            QString gameName = query.value(gameNameIndex).toString();
            QString gameExePath = query.value(gameExePathIndex).toString();
            QString gameExe = query.value(gameExeIndex).toString();
            bool running = query.value(runningIndex).toBool();
            int timePlayed = query.value(timePlayedIndex).toInt();

            games.push_back(Games{id, gameImage, gameName, gameExePath, gameExe, running, timePlayed});
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }

    return games;
}

bool DbManager::updateTimePlayed(int gameId, int timePlayed)
{
    bool success = false;

    if(timePlayed != 0)
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("UPDATE games SET timePlayed = :timePlayed WHERE id = :id");
        queryAdd.bindValue(":timePlayed", timePlayed);
        queryAdd.bindValue(":id", gameId);

        if(queryAdd.exec())
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

     if(gameId != 0)
     {
        QSqlQuery queryAdd;
        queryAdd.prepare("UPDATE games SET running = :running WHERE id = :id");
        queryAdd.bindValue(":running", running);
        queryAdd.bindValue(":id", gameId);

        if(queryAdd.exec())
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

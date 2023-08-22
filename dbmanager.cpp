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
        query.prepare("CREATE TABLE games(id INTEGER PRIMARY KEY AUTOINCREMENT,gameName TEXT, gameImage TEXT, gameExePath TEXT);");


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

bool DbManager::insertGame(const QString gameImage,const QString &gameName, const QString &gameExePath)
{
    bool success = false;

    if (!gameName.isEmpty())
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO games (gameImage,gameName,gameExePath) VALUES (:gameImage,:gameName,:gameExePath)");
        queryAdd.bindValue(":gameImage", gameImage);
        queryAdd.bindValue(":gameName", gameName);
        queryAdd.bindValue(":gameExePath", gameExePath);

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
};

QVector<DbManager::Games> DbManager::getGames() {
    QVector<Games> games;

    QSqlQuery query("SELECT * FROM games ORDER BY id DESC");
    int idIndex = query.record().indexOf("id");
    int gameImageIndex = query.record().indexOf("gameImage");
    int gameNameIndex = query.record().indexOf("gameName");
    int gameExePathIndex = query.record().indexOf("gameExePath");

    while (query.next()) {
        int id = query.value(idIndex).toInt();
        QString gameImage = query.value(gameImageIndex).toString();
        QString gameName = query.value(gameNameIndex).toString();
        QString gameExePath = query.value(gameExePathIndex).toString();

        games.push_back(Games{id, gameImage, gameName, gameExePath});
    }

    return games;
}

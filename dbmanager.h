#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQueryModel>

class DbManager
{
private:
    QSqlDatabase m_db;

public:
    DbManager(const QString &path);
    /**
     * @brief Destructor
     *
     * Close the db connection
     */
    ~DbManager();
    QString tableNamesSettings = "tblGames";

    typedef struct {
        int id;
        QString gameImage;
        QString gameName;
        QString gameExePath;
        int timePlayed;
    } table_games ;

    /**
     * @brief Bir üst alanda oluşturulan struct yapılarının toplanıp aynı yerden çağürıldığı yerdir.
     */
    struct tablesAndColumns{

        table_games _tableGames;

    };


    const tablesAndColumns *allTables;

    bool insertGame(const QString gameImage,const QString &gameName, const QString &gameExePath, const QString &gameExe);

    //Games struct
    struct Games{
        int id;
        QString gameImage;
        QString gameName;
        QString gameExePath;
        QString gameExe;
        int timePlayed;
    };

    QVector<Games> getGames();
    QVector<Games> getGameById(int gameId);

    bool updateTimePlayed(int gameId, int timePlayed);

};

#endif // DBMANAGER_H

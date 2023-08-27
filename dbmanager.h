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
    QString tblGameHistoricalSettings = "tblGameHistorical";

    typedef struct
    {
        int id;
        QString gameImage;
        QString gameName;
        QString gameExePath;
        bool running;
        int timePlayed;
        QString createdAt;
        QString updatedAt;
    } table_games;

    typedef struct
    {
        int id;
        int gameId;
        int timePlayed;
        QString createdAt;
        QString updatedAt;
    } table_game_historical;

    /**
     * @brief It is the place where struct structures created in a parent field are collected and called from the same place.
     */
    struct tablesAndColumns
    {

        table_games _tableGames;
        table_game_historical _tblGameHistorical;
    };

    const tablesAndColumns *allTables;

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

    // Game Historical struct
    struct GameHistorical
    {
        int id;
        int gameId;
        int timePlayed;
        QString createdAt;
        QString updatedAt;
    };

    // MostPlayGame Struct
    struct MostPlayGame
    {
        QString gameName;
        int timePlayed;
    };

    QVector<Games> getGames();
    QVector<Games> getGameById(int gameId);

    bool insertGame(const QString gameImage, const QString &gameName, const QString &gameExePath, const QString &gameExe);
    bool updateTimePlayed(int gameId, int timePlayed);
    bool updateGameRunning(int gameId, bool running);
    void updateAllGameRunning();
    int totalTimePlayed();
    int totalPlayTime(int gameId);
    GameHistorical getGameHistoricalToday(int gameId);
    int totalPlayTimeToday(int gameId);
    int totalPlayTimeThisWeek(int gameId);
    bool insertGameHistorical(const int gameId);
    int getTodayGameHistorical(const int gameId);
    void updateGameHistoricalTimePlayed(int gameHistoricalId, int timePlayed);
    MostPlayGame mostPlayedGame();
    int totalTimePlayedToday();
    bool DeleteGame(int gameId);
    bool DeleteGameHistorical(int gameId);
    int TimePlayedFilter(int days);
};

#endif // DBMANAGER_H

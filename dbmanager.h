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

    typedef struct
    {
        int id;
        QString achivement;
        bool unlocked;
        QString unlockedAt;
    } table_achivements;

    /**
     * @brief It is the place where struct structures created in a parent field are collected and called from the same place.
     */
    struct tablesAndColumns
    {

        table_games _tableGames;
        table_game_historical _tblGameHistorical;
        table_achivements _table_achivements;
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

    struct HoursPlayedPerDayOfTheLastWeekData
    {
        QString day;
        int dayOfWeek;
        int totalTimePlayed;
    };

    // Games struct
    struct Achivements
    {
        int id;
        QString achivement;
        bool unlocked;
        bool active;
        QString unlockedAt;
    };

    QVector<Games> getGames();
    QVector<Games> getGameById(int gameId);

    bool insertGame(const QString gameImage, const QString &gameName, const QString &gameExePath, const QString &gameExe);
    bool updateGame(const QString gameImage, const QString &gameName, const QString &gameExePath, const QString &gameExe, const int &gameId);
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
    bool deleteGame(int gameId);
    bool deleteGameHistorical(int gameId);
    int timePlayedFilter(int days);
    QVector<DbManager::HoursPlayedPerDayOfTheLastWeekData> hoursPlayedPerDayOfTheLastWeek(int gameId);
    QVector<DbManager::Achivements> getAchivements() ;
    bool updateAchivement(int id);
    void updateActiveTheme(int id);
};

#endif // DBMANAGER_H

#include "stats.h"
#include "dbmanager.h"
#include "ui_stats.h"
#include "util.h"

Stats::Stats(QWidget *parent) : QWidget(parent),
                                ui(new Ui::Stats)
{
    ui->setupUi(this);

    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    int totalTimePlayed = db->totalTimePlayed();

    Util *util = new Util();
    QString totalTimePlayedString = util->secondsToTime(totalTimePlayed);

    ui->totalTimePlayedText->setText(totalTimePlayedString);

    DbManager::MostPlayGame mostPlayedGameResult = db->mostPlayedGame();

    ui->mostPlayedGameText->setText(util->secondsToTime(mostPlayedGameResult.timePlayed));
    ui->mostPlayedGameTitle->setText("MOST PLAYED GAME( " + mostPlayedGameResult.gameName + " )");

    int timePlayedToday = db->totalTimePlayedToday();
    ui->timePlayedTodayText->setText(util->secondsToTime(timePlayedToday));

    int timePlayedForTheLastWeek = db->timePlayedFilter(7);
    ui->timePlayedThisWeek->setText(util->secondsToTime(timePlayedForTheLastWeek));

    int timePlayedForTheLastMonth = db->timePlayedFilter(30);
    ui->timePlayedThisMonth->setText(util->secondsToTime(timePlayedForTheLastMonth));

    int timePlayedForTheLastYear = db->timePlayedFilter(365);
    ui->timePlayedThForTheLastYear->setText(util->secondsToTime(timePlayedForTheLastYear));

    delete db;
}

Stats::~Stats()
{
    delete ui;
}

void Stats::refreshStats()
{
    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    Util *util = new Util();

    QString totalTimePlayedString = util->secondsToTime(db->totalTimePlayed());

    ui->totalTimePlayedText->setText(totalTimePlayedString);

    DbManager::MostPlayGame mostPlayedGameResult = db->mostPlayedGame();

    ui->mostPlayedGameText->setText(util->secondsToTime(mostPlayedGameResult.timePlayed));
    ui->mostPlayedGameTitle->setText("MOST PLAYED GAME( " + mostPlayedGameResult.gameName + " )");

    int timePlayedToday = db->totalTimePlayedToday();
    ui->timePlayedTodayText->setText(util->secondsToTime(timePlayedToday));

    int timePlayedForTheLastWeek = db->timePlayedFilter(7);
    ui->timePlayedThisWeek->setText(util->secondsToTime(timePlayedForTheLastWeek));

    int timePlayedForTheLastMonth = db->timePlayedFilter(30);
    ui->timePlayedThisMonth->setText(util->secondsToTime(timePlayedForTheLastMonth));

    int timePlayedForTheLastYear = db->timePlayedFilter(365);
    ui->timePlayedThForTheLastYear->setText(util->secondsToTime(timePlayedForTheLastYear));

    qDebug() << util->secondsToTime(db->totalTimePlayed());

    delete db;
}

void Stats::on_btnReload_clicked()
{
    Stats::refreshStats();
}

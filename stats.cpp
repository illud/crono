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

    Util util;
    QString totalTimePlayedString = util.SecondsToTime(totalTimePlayed);

    ui->totalTimePlayedText->setText(totalTimePlayedString);

    DbManager::MostPlayGame mostPlayedGameResult = db->mostPlayedGame();

    ui->mostPlayedGameText->setText(util.SecondsToTime(mostPlayedGameResult.timePlayed));
    ui->mostPlayedGameTitle->setText("MOST PLAYED GAME( " + mostPlayedGameResult.gameName + " )");

    int timePlayedToday = db->totalTimePlayedToday();
    ui->timePlayedTodayText->setText(util.SecondsToTime(timePlayedToday));

    int timePlayedForTheLastWeek = db->TimePlayedFilter(7);
    ui->timePlayedThisWeek->setText(util.SecondsToTime(timePlayedForTheLastWeek));

    int timePlayedForTheLastMonth = db->TimePlayedFilter(30);
    ui->timePlayedThisMonth->setText(util.SecondsToTime(timePlayedForTheLastMonth));

    int timePlayedForTheLastYear = db->TimePlayedFilter(365);
    ui->timePlayedThForTheLastYear->setText(util.SecondsToTime(timePlayedForTheLastYear));
}

Stats::~Stats()
{
    delete ui;
}

void Stats::on_reloadBtn_clicked()
{
    static const QString path = "crono.db";

    // Instance db conn
    DbManager *db = new DbManager(path);

    int totalTimePlayed = db->totalTimePlayed();

    Util util;
    QString totalTimePlayedString = util.SecondsToTime(totalTimePlayed);

    ui->totalTimePlayedText->setText(totalTimePlayedString);

    DbManager::MostPlayGame mostPlayedGameResult = db->mostPlayedGame();

    ui->mostPlayedGameText->setText(util.SecondsToTime(mostPlayedGameResult.timePlayed));
    ui->mostPlayedGameTitle->setText("MOST PLAYED GAME( " + mostPlayedGameResult.gameName + " )");

    int timePlayedToday = db->totalTimePlayedToday();
    ui->timePlayedTodayText->setText(util.SecondsToTime(timePlayedToday));

    int timePlayedForTheLastWeek = db->TimePlayedFilter(7);
    ui->timePlayedThisWeek->setText(util.SecondsToTime(timePlayedForTheLastWeek));

    int timePlayedForTheLastMonth = db->TimePlayedFilter(30);
    ui->timePlayedThisMonth->setText(util.SecondsToTime(timePlayedForTheLastMonth));

    int timePlayedForTheLastYear = db->TimePlayedFilter(365);
    ui->timePlayedThForTheLastYear->setText(util.SecondsToTime(timePlayedForTheLastYear));
}

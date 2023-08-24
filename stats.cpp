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
    QString totalTimePlayedString = util.secondsToTime(totalTimePlayed);

    ui->totalTimePlayedText->setText(totalTimePlayedString);
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
    QString totalTimePlayedString = util.secondsToTime(totalTimePlayed);

    ui->totalTimePlayedText->setText(totalTimePlayedString);
}

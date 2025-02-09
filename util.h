#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class Util : public QObject
{

    Q_OBJECT

public:
    Util();
    QVector<QString> removeDupWord(std::string str);
    QStringList getAllImageSrc(const QString &url, const QString &gameName);
    QString secondsToTime(int time);
    QString findLastBackSlashWord(std::string path);
    bool isProcessRunning(const QString &processName);
    QString removeDataFromLasBackSlash(QString filePath);
    bool createCronoRunnerBatFile(QString gameExePath, QString gameExe);
    size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp);
    bool checkInternetConn();
    QString getGameImage(QString gameName);
    QString dayNumberToWeekDay(int day);
};

#endif // UTIL_H

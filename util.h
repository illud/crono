#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class Util
{

public:
    Util();
    QVector<QString> removeDupWord(std::string str);
    QString secondsToTime(int time);
    QString findLastBackSlashWord(std::string path);
    bool isProcessRunning(const QString &processName);
};

#endif // UTIL_H

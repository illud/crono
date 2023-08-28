#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class Util
{

public:
    Util();
    QVector<QString> RemoveDupWord(std::string str);
    QString SecondsToTime(int time);
    QString FindLastBackSlashWord(std::string path);
    bool IsProcessRunning(const QString &processName);
    QString RemoveDataFromLasBackSlash(QString filePath);
    bool CreateCronoRunnerBatFile(QString gameExePath, QString gameExe);
    size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
};

#endif // UTIL_H

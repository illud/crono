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
};

#endif // UTIL_H

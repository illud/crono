#include "util.h"
#include <sstream>
#include <QProcess>

Util::Util()
{

}

QVector<QString> Util::removeDupWord(std::string str)
{
    QVector<QString> words;
    // Used to split string around spaces.
    std::istringstream  ss(str);

    std::string word; // for storing each word

    // Traverse through all wordsF
    // while loop till we get
    // strings to store in string word
    while (ss >> word)
    {
        // print the read word
        words.push_back(word.c_str());
    }

    return words;
}

// Converts secons to hours and minutes
QString Util::secondsToTime(int time){
    int h = time / 3600;
    int m = time % 3600 / 60;

    QString hours = QString::number(h);

    QString minutes = QString::number(m);

    return hours + "h " + minutes + "m";
}

// Finds game exe name (game.exe)
QString Util::findLastBackSlashWord(std::string path){
    auto const pos = path.find_last_of('/');
    const auto leaf = path.substr(pos + 1);

    return leaf.c_str();
}

// Check if game is running
bool Util::isProcessRunning(const QString &processName) {
    QProcess process;
#ifdef Q_OS_WIN
    process.start("tasklist");
#else
    process.start("ps", QStringList() << "aux");
#endif
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QString outputStr = QString::fromLocal8Bit(output);

    return outputStr.contains(processName, Qt::CaseInsensitive);
}


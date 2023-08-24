#include "util.h"
#include "qdebug.h"
#include <sstream>
#include <QProcess>
#include <QFile>
#include <QTextStream>

Util::Util()
{
}

QVector<QString> Util::removeDupWord(std::string str)
{
    QVector<QString> words;
    // Used to split string around spaces.
    std::istringstream ss(str);

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
QString Util::secondsToTime(int time)
{
    int h = time / 3600;
    int m = time % 3600 / 60;

    QString hours = QString::number(h);

    QString minutes = QString::number(m);

    return hours + "h " + minutes + "m";
}

// Finds game exe name (game.exe)
QString Util::findLastBackSlashWord(std::string path)
{
    auto const pos = path.find_last_of("\\");
    const auto leaf = path.substr(pos + 1);

    return leaf.c_str();
}

QString Util::removeDataFromLasBackSlash(QString filePath)
{

    // Find the index of the last backslash
    int lastIndex = filePath.lastIndexOf("\\");

    if (lastIndex != -1)
    {
        // Extract the part of the string before the last backslash
        QString newPath = filePath.left(lastIndex + 1);

        return newPath;
    }

    return "Error";
}

bool Util::createCronoRunnerBatFile(QString gameExePath, QString gameExe)
{
    // Specify the file name/path
    QString fileName = "crono_runner.bat";

    // Create a QFile instance
    QFile file(fileName);

    // Open the file in write mode
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Create a QTextStream to write to the file
        QTextStream out(&file);

        // Write text to the file
        out << "start /d "
               "\""
            << gameExePath << "\" " << gameExe << Qt::endl;

        // Close the file
        file.close();

        return true;
    }
    else
    {
        // Handle error if the file couldn't be opened
        qDebug() << "Failed to open the file for writing.";
        return false;
    }
}

// Check if game is running
bool Util::isProcessRunning(const QString &processName)
{
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

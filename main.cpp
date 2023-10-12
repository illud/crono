#include "mainwindow.h"
#include "utiltest.h"
//#include <Windows.h>
#include <QApplication>

//#include <QTest>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Disable console output in release mode on Windows
    //FreeConsole();

    // Tests config import
    //UtilTest utilTest;
    //QTest::qExec(&utilTest, argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}

#include "utiltest.h"
#include "util.h"

UtilTest::UtilTest(QObject *parent)
    : QObject{parent}
{
}

void UtilTest::testSecondsToTime()
{
    Util util; // Create an instance of your Util class
    // Test case 1: 3661 seconds should be converted to "1h 1m"
    QCOMPARE(util.secondsToTime(3661), QString("1h 1m"));

    // Test case 2: 7200 seconds should be converted to "2h 0m"
    QCOMPARE(util.secondsToTime(7200), QString("2h 0m"));

    // Test case 3: 150 seconds should be converted to "0h 2m"
    QCOMPARE(util.secondsToTime(150), QString("0h 2m"));

    // Test case 4: 0 seconds should be converted to "0h 0m"
    QCOMPARE(util.secondsToTime(0), QString("0h 0m"));
}

void UtilTest::testRemoveDupWord()
{
    Util util;
    // Test case: Test with a string containing duplicate words
    std::string input1 = "Hello world world";
    QVector<QString> result1 = util.removeDupWord(input1);
    QVector<QString> expected1 = {"Hello", "world", "world"};
    QCOMPARE(result1, expected1);
}

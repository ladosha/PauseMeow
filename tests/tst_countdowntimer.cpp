#include "CountdownTimer.h"

#include <QSignalSpy>
#include <QtTest>

class CountdownTimerTest final : public QObject
{
    Q_OBJECT

private slots:
    void workExpiresIntoEntering()
    {
        CountdownTimer timer;
        QSignalSpy expiredSpy(&timer, &CountdownTimer::workExpired);

        timer.startWork(1);
        QCOMPARE(timer.phase(), CountdownTimer::Phase::Work);
        QCOMPARE(timer.remainingSeconds(), 1);

        QTRY_COMPARE_WITH_TIMEOUT(expiredSpy.count(), 1, 1600);
        QCOMPARE(timer.phase(), CountdownTimer::Phase::Entering);
        QCOMPARE(timer.remainingSeconds(), 0);
    }

    void restExpiresIntoLeaving()
    {
        CountdownTimer timer;
        QSignalSpy expiredSpy(&timer, &CountdownTimer::restExpired);

        timer.startRest(1);
        QTRY_COMPARE_WITH_TIMEOUT(expiredSpy.count(), 1, 1600);
        QCOMPARE(timer.phase(), CountdownTimer::Phase::Leaving);
    }

    void workCanPauseAndResume()
    {
        CountdownTimer timer;
        timer.startWork(2);
        timer.togglePause();

        QVERIFY(timer.paused());
        const int pausedAt = timer.remainingSeconds();
        QTest::qWait(350);
        QCOMPARE(timer.remainingSeconds(), pausedAt);

        timer.togglePause();
        QVERIFY(!timer.paused());
        QTRY_VERIFY_WITH_TIMEOUT(timer.remainingSeconds() < pausedAt, 1400);
    }

    void restCannotBePaused()
    {
        CountdownTimer timer;
        timer.startRest(2);
        timer.togglePause();
        QVERIFY(!timer.paused());
    }
};

QTEST_GUILESS_MAIN(CountdownTimerTest)

#include "tst_countdowntimer.moc"

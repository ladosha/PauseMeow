#include "AppController.h"
#include "PlatformIntegration.h"

#include <QCoreApplication>
#include <QDir>
#include <QSignalSpy>
#include <QSettings>
#include <QtTest>

namespace {
const QByteArray TestOrganization = "PauseMeowIntegrationTests";
const QByteArray TestApplication = "Persistence";
const QByteArray TestAutostartValue = "PauseMeowIntegrationTest";

QSettings testSettings()
{
    return QSettings(
        QString::fromLatin1(TestOrganization),
        QString::fromLatin1(TestApplication));
}

QSettings runKey()
{
    return QSettings(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
}
}

class WindowsIntegrationTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qputenv("PAUSEMEOW_TEST_ORGANIZATION", TestOrganization);
        qputenv("PAUSEMEOW_TEST_APPLICATION", TestApplication);
        qputenv("PAUSEMEOW_TEST_AUTOSTART_VALUE", TestAutostartValue);
        clearTestState();
    }

    void cleanup()
    {
        clearTestState();
    }

    void cleanupTestCase()
    {
        clearTestState();
        qunsetenv("PAUSEMEOW_TEST_ORGANIZATION");
        qunsetenv("PAUSEMEOW_TEST_APPLICATION");
        qunsetenv("PAUSEMEOW_TEST_AUTOSTART_VALUE");
    }

    void settingsPersistAcrossControllers()
    {
        {
            AppController controller(false, true);
            controller.applySettings(
                33, 7, QStringLiteral("  stretch   and breathe  "),
                false, true, false, false, true, false);
            controller.setLanguage(QStringLiteral("ru"));
        }

        AppController restored(false, true);
        QVERIFY(restored.configured());
        QCOMPARE(restored.workMinutes(), 33);
        QCOMPARE(restored.restMinutes(), 7);
        QCOMPARE(restored.breakMessage(), QStringLiteral("stretch and breathe"));
        QVERIFY(!restored.dimBackground());
        QVERIFY(restored.strictMode());
        QVERIFY(!restored.autostart());
        QVERIFY(!restored.walkingSounds());
        QVERIFY(restored.walkingPurr());
        QVERIFY(!restored.restPurr());
        QCOMPARE(restored.language(), QStringLiteral("ru"));
    }

    void autostartEnableDisableAndRepair()
    {
        QVERIFY(PlatformIntegration::setAutostartEnabled(true));

        QSettings registry = runKey();
        const QString valueName = QString::fromLatin1(TestAutostartValue);
        const QString expected = QStringLiteral("\"%1\" --background").arg(
            QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        QCOMPARE(registry.value(valueName).toString(), expected);

        registry.setValue(valueName, QStringLiteral("\"C:\\Moved\\PauseMeow.exe\" --background"));
        registry.sync();
        QVERIFY(PlatformIntegration::reconcileAutostartEnabled(true));
        registry.sync();
        QCOMPARE(registry.value(valueName).toString(), expected);

        QVERIFY(PlatformIntegration::setAutostartEnabled(false));
        registry.sync();
        QVERIFY(!registry.contains(valueName));
    }

    void resetClearsSettingsAndAutostart()
    {
        QSettings settings = testSettings();
        settings.setValue(QStringLiteral("configured"), true);
        settings.setValue(QStringLiteral("workMinutes"), 99);
        settings.sync();
        QVERIFY(PlatformIntegration::setAutostartEnabled(true));

        PlatformIntegration::resetUserState();

        settings.sync();
        QVERIFY(settings.allKeys().isEmpty());
        QSettings registry = runKey();
        registry.sync();
        QVERIFY(!registry.contains(QString::fromLatin1(TestAutostartValue)));
    }

    void strictModeBlocksEarlyFinish()
    {
        AppController controller(false, true);
        controller.previewBreak(
            3, QString(), true, true, false, false, false);
        QCOMPARE(controller.phase(), QStringLiteral("entering"));
        controller.beginRest();
        QCOMPARE(controller.phase(), QStringLiteral("rest"));

        controller.finishBreakEarly();
        QCOMPARE(controller.phase(), QStringLiteral("rest"));
    }

    void gentleModeAllowsEarlyFinish()
    {
        AppController controller(false, true);
        controller.previewBreak(
            3, QString(), true, false, false, false, false);
        controller.beginRest();
        QCOMPARE(controller.phase(), QStringLiteral("rest"));

        controller.finishBreakEarly();
        QCOMPARE(controller.phase(), QStringLiteral("leaving"));
    }

    void fourClicksTriggerAnnoyedReaction()
    {
        AppController controller(false, true);
        controller.previewBreak(
            3, QString(), true, false, false, false, false);
        controller.beginRest();

        QSignalSpy pokeSpy(&controller, &AppController::catPokeRequested);
        QSignalSpy hissSpy(&controller, &AppController::catHissRequested);
        for (int click = 0; click < 4; ++click) {
            controller.registerCatClick();
        }

        QCOMPARE(pokeSpy.count(), 3);
        QCOMPARE(hissSpy.count(), 1);
    }

private:
    static void clearTestState()
    {
        QSettings settings = testSettings();
        settings.clear();
        settings.sync();

        QSettings registry = runKey();
        registry.remove(QString::fromLatin1(TestAutostartValue));
        registry.sync();
    }
};

QTEST_GUILESS_MAIN(WindowsIntegrationTest)

#include "tst_windowsintegration.moc"

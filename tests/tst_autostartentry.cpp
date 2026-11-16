#include "AutostartEntry.h"

#include <QtTest>

class AutostartEntryTest final : public QObject
{
    Q_OBJECT

private slots:
    void generatedEntryIsEnabled()
    {
        const QString executable = QStringLiteral(
            "/home/cat/My Apps/Pause%Meow.AppImage");
        const QString entry = AutostartEntry::build(executable);

        QCOMPARE(AutostartEntry::inspect(entry, executable),
            AutostartEntry::State::Enabled);
        QVERIFY(entry.contains(QStringLiteral(" --background\n")));
        QVERIFY(entry.contains(QStringLiteral("Pause%%Meow.AppImage")));
    }

    void movedAppImageIsStale()
    {
        const QString entry = AutostartEntry::build(
            QStringLiteral("/home/cat/Downloads/PauseMeow.AppImage"));

        QCOMPARE(AutostartEntry::inspect(
                     entry, QStringLiteral("/home/cat/Applications/PauseMeow.AppImage")),
            AutostartEntry::State::Stale);
    }

    void hiddenEntryIsDisabled()
    {
        QString entry = AutostartEntry::build(
            QStringLiteral("/home/cat/PauseMeow.AppImage"));
        entry.append(QStringLiteral("Hidden=true\n"));

        QCOMPARE(AutostartEntry::inspect(
                     entry, QStringLiteral("/home/cat/PauseMeow.AppImage")),
            AutostartEntry::State::Disabled);
    }

    void gnomeDisabledEntryIsDisabled()
    {
        QString entry = AutostartEntry::build(
            QStringLiteral("/home/cat/PauseMeow.AppImage"));
        entry.replace(QStringLiteral("X-GNOME-Autostart-enabled=true"),
            QStringLiteral("X-GNOME-Autostart-enabled=false"));

        QCOMPARE(AutostartEntry::inspect(
                     entry, QStringLiteral("/home/cat/PauseMeow.AppImage")),
            AutostartEntry::State::Disabled);
    }

    void malformedEntryIsInvalid()
    {
        const QString entry = QStringLiteral(
            "[Desktop Entry]\nType=Application\nName=PauseMeow\n");

        QCOMPARE(AutostartEntry::inspect(
                     entry, QStringLiteral("/home/cat/PauseMeow.AppImage")),
            AutostartEntry::State::Invalid);
    }
};

QTEST_GUILESS_MAIN(AutostartEntryTest)

#include "tst_autostartentry.moc"

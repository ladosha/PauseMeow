#include "AutostartEntry.h"

#include <QHash>
#include <QStringList>

namespace {
bool desktopBoolean(const QString &value, const bool fallback)
{
    if (value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0
        || value == QStringLiteral("1")) {
        return true;
    }
    if (value.compare(QStringLiteral("false"), Qt::CaseInsensitive) == 0
        || value == QStringLiteral("0")) {
        return false;
    }
    return fallback;
}

QHash<QString, QString> desktopEntryValues(QStringView contents)
{
    QHash<QString, QString> values;
    bool inDesktopEntry = false;

    for (QString line : contents.toString().split(QLatin1Char('\n'))) {
        if (line.endsWith(QLatin1Char('\r'))) {
            line.chop(1);
        }
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (trimmed.startsWith(QLatin1Char('['))
            && trimmed.endsWith(QLatin1Char(']'))) {
            inDesktopEntry = trimmed == QStringLiteral("[Desktop Entry]");
            continue;
        }
        if (!inDesktopEntry) {
            continue;
        }

        const qsizetype separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) {
            continue;
        }
        values.insert(
            line.left(separator).trimmed(), line.mid(separator + 1).trimmed());
    }
    return values;
}
}

QString AutostartEntry::quoteExecArgument(QString argument)
{
    argument.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    argument.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    argument.replace(QLatin1Char('`'), QStringLiteral("\\`"));
    argument.replace(QLatin1Char('$'), QStringLiteral("\\$"));
    argument.replace(QLatin1Char('%'), QStringLiteral("%%"));
    return QStringLiteral("\"%1\"").arg(argument);
}

QString AutostartEntry::expectedExec(const QString &executable)
{
    return QStringLiteral("%1 --background").arg(quoteExecArgument(executable));
}

QString AutostartEntry::build(const QString &executable)
{
    return QStringLiteral(
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Version=1.0\n"
        "Name=PauseMeow\n"
        "Comment=Work and break timer\n"
        "Exec=%1\n"
        "Icon=com.vladmas.PauseMeow\n"
        "Terminal=false\n"
        "StartupNotify=false\n"
        "X-GNOME-Autostart-enabled=true\n")
        .arg(expectedExec(executable));
}

AutostartEntry::State AutostartEntry::inspect(
    const QStringView contents, const QString &executable)
{
    const QHash<QString, QString> values = desktopEntryValues(contents);
    if (values.value(QStringLiteral("Type")) != QStringLiteral("Application")
        || !values.contains(QStringLiteral("Exec"))) {
        return State::Invalid;
    }
    if (desktopBoolean(values.value(QStringLiteral("Hidden")), false)
        || !desktopBoolean(
            values.value(QStringLiteral("X-GNOME-Autostart-enabled")), true)) {
        return State::Disabled;
    }
    return values.value(QStringLiteral("Exec")) == expectedExec(executable)
        ? State::Enabled
        : State::Stale;
}

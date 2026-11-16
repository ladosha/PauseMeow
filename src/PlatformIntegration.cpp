#include "PlatformIntegration.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QVariantMap>
#endif

namespace {
constexpr auto OrganizationName = "PauseMeow";
constexpr auto ApplicationName = "PauseMeow";
constexpr auto AutostartValueName = "PauseMeow";
constexpr auto DesktopId = "com.vladmas.PauseMeow";

#ifdef Q_OS_LINUX
constexpr auto PortalService = "org.freedesktop.portal.Desktop";
constexpr auto PortalPath = "/org/freedesktop/portal/desktop";
constexpr auto BackgroundPortalInterface = "org.freedesktop.portal.Background";
constexpr auto RequestPortalInterface = "org.freedesktop.portal.Request";

class PortalResponseWaiter final : public QObject
{
    Q_OBJECT

public:
    bool received = false;
    uint response = 2;
    QVariantMap results;

public slots:
    void handleResponse(const uint responseCode, const QVariantMap &responseResults)
    {
        received = true;
        response = responseCode;
        results = responseResults;
        emit finished();
    }

signals:
    void finished();
};

QString quoteDesktopExecArgument(QString argument)
{
    argument.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    argument.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    argument.replace(QLatin1Char('`'), QStringLiteral("\\`"));
    argument.replace(QLatin1Char('$'), QStringLiteral("\\$"));
    argument.replace(QLatin1Char('%'), QStringLiteral("%%"));
    return QStringLiteral("\"%1\"").arg(argument);
}

QString nativeAutostartPath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation))
        .filePath(QStringLiteral("autostart/%1.desktop").arg(
            QString::fromLatin1(DesktopId)));
}

QString linuxLaunchExecutable()
{
    const QString appImage = qEnvironmentVariable("APPIMAGE");
    if (!appImage.isEmpty() && QFileInfo::exists(appImage)) {
        return QFileInfo(appImage).absoluteFilePath();
    }
    return QCoreApplication::applicationFilePath();
}

bool setNativeLinuxAutostart(const bool enabled)
{
    const QString autostartPath = nativeAutostartPath();
    if (!enabled) {
        const bool removed = !QFileInfo::exists(autostartPath)
            || QFile::remove(autostartPath);
        const QString legacyPath = QDir(QFileInfo(autostartPath).absolutePath())
            .filePath(QStringLiteral("pausemeow.desktop"));
        if (QFileInfo::exists(legacyPath)) {
            QFile::remove(legacyPath);
        }
        return removed;
    }

    if (!QDir().mkpath(QFileInfo(autostartPath).absolutePath())) {
        return false;
    }

    const QString contents = QStringLiteral(
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Version=1.0\n"
        "Name=PauseMeow\n"
        "Comment=Work and break timer\n"
        "Exec=%1 --background\n"
        "Icon=%2\n"
        "Terminal=false\n"
        "StartupNotify=false\n"
        "X-GNOME-Autostart-enabled=true\n")
        .arg(quoteDesktopExecArgument(linuxLaunchExecutable()),
            QString::fromLatin1(DesktopId));

    QSaveFile desktopFile(autostartPath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)
        || desktopFile.write(contents.toUtf8()) < 0) {
        desktopFile.cancelWriting();
        return false;
    }
    return desktopFile.commit();
}

bool requestFlatpakAutostart(const bool enabled)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return false;
    }

    QDBusInterface background(
        QString::fromLatin1(PortalService),
        QString::fromLatin1(PortalPath),
        QString::fromLatin1(BackgroundPortalInterface),
        bus);
    if (!background.isValid()) {
        return false;
    }

    const QString token = QStringLiteral("pausemeow_%1")
        .arg(QUuid::createUuid().toString(QUuid::Id128));
    QString sender = bus.baseService();
    if (sender.startsWith(QLatin1Char(':'))) {
        sender.remove(0, 1);
    }
    sender.replace(QLatin1Char('.'), QLatin1Char('_'));
    const QString predictedRequestPath = QStringLiteral(
        "/org/freedesktop/portal/desktop/request/%1/%2").arg(sender, token);

    PortalResponseWaiter waiter;
    if (!bus.connect(
            QString::fromLatin1(PortalService),
            predictedRequestPath,
            QString::fromLatin1(RequestPortalInterface),
            QStringLiteral("Response"),
            &waiter,
            SLOT(handleResponse(uint,QVariantMap)))) {
        return false;
    }

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), token);
    options.insert(QStringLiteral("reason"),
        QStringLiteral("Start PauseMeow automatically and keep the break timer available in the background."));
    options.insert(QStringLiteral("autostart"), enabled);
    options.insert(QStringLiteral("commandline"), QStringList {
        QFileInfo(QCoreApplication::applicationFilePath()).fileName(),
        QStringLiteral("--background"),
    });

    const QDBusReply<QDBusObjectPath> reply = background.call(
        QStringLiteral("RequestBackground"), QString(), options);
    if (!reply.isValid()) {
        bus.disconnect(QString::fromLatin1(PortalService), predictedRequestPath,
            QString::fromLatin1(RequestPortalInterface), QStringLiteral("Response"),
            &waiter, SLOT(handleResponse(uint,QVariantMap)));
        return false;
    }

    const QString requestPath = reply.value().path();
    if (requestPath != predictedRequestPath) {
        bus.disconnect(QString::fromLatin1(PortalService), predictedRequestPath,
            QString::fromLatin1(RequestPortalInterface), QStringLiteral("Response"),
            &waiter, SLOT(handleResponse(uint,QVariantMap)));
        if (!bus.connect(QString::fromLatin1(PortalService), requestPath,
                QString::fromLatin1(RequestPortalInterface), QStringLiteral("Response"),
                &waiter, SLOT(handleResponse(uint,QVariantMap)))) {
            return false;
        }
    }

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&waiter, &PortalResponseWaiter::finished,
        &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(120000);
    if (!waiter.received) {
        loop.exec();
    }

    bus.disconnect(QString::fromLatin1(PortalService), requestPath,
        QString::fromLatin1(RequestPortalInterface), QStringLiteral("Response"),
        &waiter, SLOT(handleResponse(uint,QVariantMap)));

    if (!waiter.received || waiter.response != 0) {
        return false;
    }
    return !enabled || waiter.results.value(QStringLiteral("autostart")).toBool();
}
#endif
}

bool PlatformIntegration::setAutostartEnabled(const bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    if (enabled) {
        const QString executable = QDir::toNativeSeparators(
            QCoreApplication::applicationFilePath());
        runKey.setValue(QString::fromLatin1(AutostartValueName),
            QStringLiteral("\"%1\" --background").arg(executable));
    } else {
        runKey.remove(QString::fromLatin1(AutostartValueName));
    }
    runKey.sync();
    return runKey.status() == QSettings::NoError;
#elif defined(Q_OS_LINUX)
    if (qEnvironmentVariableIsSet("FLATPAK_ID")) {
        return requestFlatpakAutostart(enabled);
    }
    return setNativeLinuxAutostart(enabled);
#else
    return !enabled;
#endif
}

void PlatformIntegration::resetUserState()
{
    // Autostart is stored outside QSettings on every supported platform.
    (void)setAutostartEnabled(false);

    QSettings settings(
        QString::fromLatin1(OrganizationName),
        QString::fromLatin1(ApplicationName));
    settings.clear();
    settings.sync();
}

#ifdef Q_OS_LINUX
#include "PlatformIntegration.moc"
#endif

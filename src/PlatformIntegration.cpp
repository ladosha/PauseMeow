#include "PlatformIntegration.h"
#include "AutostartEntry.h"

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
constexpr auto DesktopId = "com.vladmas.PauseMeow";

QString settingsOrganization()
{
#ifdef PAUSEMEOW_TESTING
    const QString override = qEnvironmentVariable("PAUSEMEOW_TEST_ORGANIZATION");
    if (!override.isEmpty()) {
        return override;
    }
#endif
    return QStringLiteral("PauseMeow");
}

QString settingsApplication()
{
#ifdef PAUSEMEOW_TESTING
    const QString override = qEnvironmentVariable("PAUSEMEOW_TEST_APPLICATION");
    if (!override.isEmpty()) {
        return override;
    }
#endif
    return QStringLiteral("PauseMeow");
}

QString autostartValueName()
{
#ifdef PAUSEMEOW_TESTING
    const QString override = qEnvironmentVariable("PAUSEMEOW_TEST_AUTOSTART_VALUE");
    if (!override.isEmpty()) {
        return override;
    }
#endif
    return QStringLiteral("PauseMeow");
}

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

    const QString contents = AutostartEntry::build(linuxLaunchExecutable());

    QSaveFile desktopFile(autostartPath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)
        || desktopFile.write(contents.toUtf8()) < 0) {
        desktopFile.cancelWriting();
        return false;
    }
    return desktopFile.commit();
}

bool reconcileNativeLinuxAutostart()
{
    QFile desktopFile(nativeAutostartPath());
    if (!desktopFile.exists()) {
        return false;
    }
    if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const AutostartEntry::State state = AutostartEntry::inspect(
        QString::fromUtf8(desktopFile.readAll()), linuxLaunchExecutable());
    switch (state) {
    case AutostartEntry::State::Enabled:
        return true;
    case AutostartEntry::State::Stale:
        return setNativeLinuxAutostart(true);
    case AutostartEntry::State::Disabled:
    case AutostartEntry::State::Invalid:
        return false;
    }
    return false;
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
        runKey.setValue(autostartValueName(),
            QStringLiteral("\"%1\" --background").arg(executable));
    } else {
        runKey.remove(autostartValueName());
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

bool PlatformIntegration::reconcileAutostartEnabled(const bool savedEnabled)
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    const QString valueName = autostartValueName();
    if (!runKey.contains(valueName)) {
        return false;
    }

    const QString executable = QDir::toNativeSeparators(
        QCoreApplication::applicationFilePath());
    const QString expected = QStringLiteral("\"%1\" --background").arg(executable);
    if (runKey.value(valueName).toString() != expected) {
        runKey.setValue(valueName, expected);
        runKey.sync();
    }
    return runKey.status() == QSettings::NoError;
#elif defined(Q_OS_LINUX)
    if (qEnvironmentVariableIsSet("FLATPAK_ID")) {
        return savedEnabled;
    }
    return reconcileNativeLinuxAutostart();
#else
    Q_UNUSED(savedEnabled);
    return false;
#endif
}

void PlatformIntegration::resetUserState()
{
    // Autostart is stored outside QSettings on every supported platform.
    (void)setAutostartEnabled(false);

    QSettings settings(settingsOrganization(), settingsApplication());
    settings.clear();
    settings.sync();
}

#ifdef Q_OS_LINUX
#include "PlatformIntegration.moc"
#endif

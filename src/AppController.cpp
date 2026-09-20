#include "AppController.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QIcon>
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>

namespace {
constexpr auto OrganizationName = "DeskCat";
constexpr auto ApplicationName = "DeskCat";
}

AppController::AppController(const bool trayEnabled, QObject *parent)
    : QObject(parent)
    , m_trayEnabled(trayEnabled)
{
    loadSettings();

    connect(&m_timer, &CountdownTimer::phaseChanged, this, [this] {
        emit phaseChanged();
        updateTray();
    });
    connect(&m_timer, &CountdownTimer::remainingSecondsChanged, this, [this] {
        emit remainingSecondsChanged();
        updateTray();
    });
    connect(&m_timer, &CountdownTimer::pausedChanged, this, [this] {
        emit pausedChanged();
        updateTray();
    });

    if (m_trayEnabled) {
        createTrayIcon();
    }

    if (m_configured) {
        QTimer::singleShot(0, this, &AppController::startWork);
    } else {
        m_settingsVisible = true;
    }
}

AppController::~AppController() = default;

QString AppController::phase() const
{
    switch (m_timer.phase()) {
    case CountdownTimer::Phase::Stopped:
        return QStringLiteral("stopped");
    case CountdownTimer::Phase::Work:
        return QStringLiteral("work");
    case CountdownTimer::Phase::Entering:
        return QStringLiteral("entering");
    case CountdownTimer::Phase::Rest:
        return QStringLiteral("rest");
    case CountdownTimer::Phase::Leaving:
        return QStringLiteral("leaving");
    }
    return QStringLiteral("stopped");
}

int AppController::remainingSeconds() const
{
    return m_timer.remainingSeconds();
}

QString AppController::remainingText() const
{
    return formatSeconds(m_timer.remainingSeconds());
}

bool AppController::paused() const
{
    return m_timer.paused();
}

int AppController::workMinutes() const noexcept
{
    return m_workMinutes;
}

int AppController::restMinutes() const noexcept
{
    return m_restMinutes;
}

bool AppController::dimBackground() const noexcept
{
    return m_dimBackground;
}

bool AppController::strictMode() const noexcept
{
    return m_strictMode;
}

bool AppController::autostart() const noexcept
{
    return m_autostart;
}

bool AppController::walkingSounds() const noexcept
{
    return m_walkingSounds;
}

bool AppController::walkingPurr() const noexcept
{
    return m_walkingPurr;
}

bool AppController::restPurr() const noexcept
{
    return m_restPurr;
}

bool AppController::configured() const noexcept
{
    return m_configured;
}

bool AppController::settingsVisible() const noexcept
{
    return m_settingsVisible;
}

void AppController::applySettings(
    const int workMinutes,
    const int restMinutes,
    const bool dimBackground,
    const bool strictMode,
    const bool autostart,
    const bool walkingSounds,
    const bool walkingPurr,
    const bool restPurr)
{
    m_workMinutes = qBound(1, workMinutes, 240);
    m_restMinutes = qBound(1, restMinutes, 60);
    m_dimBackground = dimBackground;
    m_strictMode = strictMode;
    m_autostart = autostart;
    m_walkingSounds = walkingSounds;
    m_walkingPurr = walkingPurr;
    m_restPurr = restPurr;
    emit settingsChanged();

    if (!m_configured) {
        m_configured = true;
        emit configuredChanged();
    }

    saveSettings();
    setAutostartEnabled(m_autostart);
    hideSettings();
    startWork();
}

void AppController::showSettings()
{
    if (m_settingsVisible) {
        return;
    }
    m_settingsVisible = true;
    emit settingsVisibleChanged();
}

void AppController::hideSettings()
{
    if (!m_settingsVisible) {
        return;
    }
    m_settingsVisible = false;
    emit settingsVisibleChanged();
}

void AppController::previewBreak(const int seconds)
{
    m_previewRestSeconds = qBound(3, seconds, 3600);
    hideSettings();
    m_timer.beginEntering();
}

void AppController::beginRest()
{
    if (m_timer.phase() != CountdownTimer::Phase::Entering) {
        return;
    }

    const int duration = m_previewRestSeconds > 0
        ? m_previewRestSeconds
        : m_restMinutes * 60;
    m_previewRestSeconds = 0;
    m_timer.startRest(duration);
}

void AppController::completeLeaving()
{
    if (m_timer.phase() != CountdownTimer::Phase::Leaving) {
        return;
    }
    startWork();
}

void AppController::finishBreakEarly()
{
    if (m_strictMode) {
        return;
    }

    const auto currentPhase = m_timer.phase();
    if (currentPhase == CountdownTimer::Phase::Entering
        || currentPhase == CountdownTimer::Phase::Rest) {
        m_timer.beginLeaving();
    }
}

void AppController::registerCatClick()
{
    if (m_timer.phase() != CountdownTimer::Phase::Rest) {
        return;
    }

    if (!m_clickClock.isValid()) {
        m_clickClock.start();
    }
    const qint64 now = m_clickClock.elapsed();

    while (!m_recentClicks.isEmpty() && now - m_recentClicks.front() > 2500) {
        m_recentClicks.removeFirst();
    }
    m_recentClicks.append(now);

    if (m_recentClicks.size() >= 4) {
        m_recentClicks.clear();
        emit catHissRequested();
    } else {
        emit catPokeRequested();
    }
}

void AppController::togglePause()
{
    m_timer.togglePause();
}

void AppController::quitApplication()
{
    QCoreApplication::quit();
}

void AppController::loadSettings()
{
    QSettings settings(OrganizationName, ApplicationName);
    m_configured = settings.value(QStringLiteral("configured"), false).toBool();
    m_workMinutes = settings.value(QStringLiteral("workMinutes"), 45).toInt();
    m_restMinutes = settings.value(QStringLiteral("restMinutes"), 10).toInt();
    m_dimBackground = settings.value(QStringLiteral("dimBackground"), true).toBool();
    m_strictMode = settings.value(QStringLiteral("strictMode"), false).toBool();
    m_autostart = settings.value(QStringLiteral("autostart"), true).toBool();
    m_walkingSounds = settings.value(QStringLiteral("walkingSounds"), true).toBool();
    m_walkingPurr = settings.value(QStringLiteral("walkingPurr"), true).toBool();
    m_restPurr = settings.value(QStringLiteral("restPurr"), true).toBool();
}

void AppController::saveSettings()
{
    QSettings settings(OrganizationName, ApplicationName);
    settings.setValue(QStringLiteral("configured"), m_configured);
    settings.setValue(QStringLiteral("workMinutes"), m_workMinutes);
    settings.setValue(QStringLiteral("restMinutes"), m_restMinutes);
    settings.setValue(QStringLiteral("dimBackground"), m_dimBackground);
    settings.setValue(QStringLiteral("strictMode"), m_strictMode);
    settings.setValue(QStringLiteral("autostart"), m_autostart);
    settings.setValue(QStringLiteral("walkingSounds"), m_walkingSounds);
    settings.setValue(QStringLiteral("walkingPurr"), m_walkingPurr);
    settings.setValue(QStringLiteral("restPurr"), m_restPurr);
}

void AppController::startWork()
{
    m_recentClicks.clear();
    m_timer.startWork(m_workMinutes * 60);
}

void AppController::createTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayMenu = new QMenu();
    auto *settingsAction = m_trayMenu->addAction(QStringLiteral("Открыть настройки"));
    auto *breakAction = m_trayMenu->addAction(QStringLiteral("Начать перерыв сейчас"));
    m_pauseAction = m_trayMenu->addAction(QStringLiteral("Приостановить таймер"));
    m_trayMenu->addSeparator();
    auto *quitAction = m_trayMenu->addAction(QStringLiteral("Выйти"));

    connect(settingsAction, &QAction::triggered, this, &AppController::showSettings);
    connect(breakAction, &QAction::triggered, this, [this] {
        previewBreak(m_restMinutes * 60);
    });
    connect(m_pauseAction, &QAction::triggered, this, &AppController::togglePause);
    connect(quitAction, &QAction::triggered, this, &AppController::quitApplication);

    m_trayIcon = new QSystemTrayIcon(QApplication::windowIcon(), this);
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this,
        [this](const QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                showSettings();
            }
        });
    m_trayIcon->show();
    updateTray();
}

void AppController::updateTray()
{
    if (!m_trayIcon) {
        return;
    }

    const QString label = phaseLabel(m_timer.phase(), m_timer.paused());
    const QString tooltip = m_timer.phase() == CountdownTimer::Phase::Stopped
        ? QStringLiteral("DeskCat — настройте первый цикл")
        : QStringLiteral("DeskCat — %1 · %2").arg(label, remainingText());
    m_trayIcon->setToolTip(tooltip);

    if (m_pauseAction) {
        m_pauseAction->setEnabled(m_timer.phase() == CountdownTimer::Phase::Work);
        m_pauseAction->setText(m_timer.paused()
            ? QStringLiteral("Продолжить таймер")
            : QStringLiteral("Приостановить таймер"));
    }
}

void AppController::setAutostartEnabled(const bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    if (enabled) {
        const QString executable = QDir::toNativeSeparators(
            QCoreApplication::applicationFilePath());
        runKey.setValue(QStringLiteral("DeskCat"),
            QStringLiteral("\"%1\" --background").arg(executable));
    } else {
        runKey.remove(QStringLiteral("DeskCat"));
    }
#else
    Q_UNUSED(enabled)
#endif
}

QString AppController::formatSeconds(const int totalSeconds)
{
    const int safeSeconds = qMax(0, totalSeconds);
    const int hours = safeSeconds / 3600;
    const int minutes = (safeSeconds % 3600) / 60;
    const int seconds = safeSeconds % 60;

    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

QString AppController::phaseLabel(
    const CountdownTimer::Phase phase,
    const bool paused)
{
    if (paused) {
        return QStringLiteral("пауза");
    }
    switch (phase) {
    case CountdownTimer::Phase::Stopped:
        return QStringLiteral("не настроен");
    case CountdownTimer::Phase::Work:
        return QStringLiteral("работа");
    case CountdownTimer::Phase::Entering:
        return QStringLiteral("кот идёт");
    case CountdownTimer::Phase::Rest:
        return QStringLiteral("отдых");
    case CountdownTimer::Phase::Leaving:
        return QStringLiteral("кот просыпается");
    }
    return QString();
}

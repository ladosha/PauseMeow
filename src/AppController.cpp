#include "AppController.h"
#include "PlatformIntegration.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>

namespace {
constexpr auto OrganizationName = "DeskCat";
constexpr auto ApplicationName = "DeskCat";

const QVariantMap EnglishTranslations {
    {QStringLiteral("unit.minutes"), QStringLiteral("min")},
    {QStringLiteral("tagline"), QStringLiteral("Your personal break guardian")},
    {QStringLiteral("status.paused"), QStringLiteral("Timer paused")},
    {QStringLiteral("status.work"), QStringLiteral("Work session")},
    {QStringLiteral("status.rest"), QStringLiteral("The cat is resting")},
    {QStringLiteral("status.entering"), QStringLiteral("The cat is on its way")},
    {QStringLiteral("status.leaving"), QStringLiteral("Break finished")},
    {QStringLiteral("status.unconfigured"), QStringLiteral("Set up your first session")},
    {QStringLiteral("status.nextAutomatic"), QStringLiteral("The next mode change will happen automatically")},
    {QStringLiteral("status.afterSaveTray"), QStringLiteral("After you save, the app will stay in the system tray")},
    {QStringLiteral("section.cycle"), QStringLiteral("Session duration")},
    {QStringLiteral("work.title"), QStringLiteral("Work")},
    {QStringLiteral("work.subtitle"), QStringLiteral("How long the cat will leave you undisturbed")},
    {QStringLiteral("rest.title"), QStringLiteral("Rest")},
    {QStringLiteral("rest.subtitle"), QStringLiteral("How long the cat will sleep on your screen")},
    {QStringLiteral("section.behavior"), QStringLiteral("Behavior")},
    {QStringLiteral("behavior.dim"), QStringLiteral("Dim the background during breaks")},
    {QStringLiteral("behavior.strict"), QStringLiteral("Strict mode — for workaholics")},
    {QStringLiteral("behavior.autostart"), QStringLiteral("Start DeskCat when you sign in")},
    {QStringLiteral("section.sound"), QStringLiteral("Sound")},
    {QStringLiteral("sound.walkingSteps"), QStringLiteral("Footstep sounds while walking")},
    {QStringLiteral("sound.walkingPurr"), QStringLiteral("Purring while walking")},
    {QStringLiteral("sound.restPurr"), QStringLiteral("Purring during breaks")},
    {QStringLiteral("section.language"), QStringLiteral("Language")},
    {QStringLiteral("language.label"), QStringLiteral("App language")},
    {QStringLiteral("button.preview"), QStringLiteral("Preview break")},
    {QStringLiteral("button.resume"), QStringLiteral("Resume")},
    {QStringLiteral("button.pause"), QStringLiteral("Pause")},
    {QStringLiteral("button.saveRestart"), QStringLiteral("Save and restart")},
    {QStringLiteral("button.saveStart"), QStringLiteral("Save and start")},
    {QStringLiteral("overlay.entering.title"), QStringLiteral("Time to stop")},
    {QStringLiteral("overlay.leaving.title"), QStringLiteral("Well rested")},
    {QStringLiteral("overlay.rest.title"), QStringLiteral("The cat sleeps — you rest")},
    {QStringLiteral("overlay.rest.subtitle"), QStringLiteral("Step away from the screen, roll your shoulders back, and look into the distance")},
    {QStringLiteral("overlay.leaving.subtitle"), QStringLiteral("The cat is waking up — you can get back to work in a moment")},
    {QStringLiteral("overlay.entering.subtitle"), QStringLiteral("The cat is coming out to guard your break")},
    {QStringLiteral("overlay.hiss"), QStringLiteral("Shh… enough, let me sleep")},
    {QStringLiteral("overlay.untilWake"), QStringLiteral("until wake-up")},
    {QStringLiteral("overlay.wake"), QStringLiteral("Wake the cat")},
    {QStringLiteral("tray.settings"), QStringLiteral("Open settings")},
    {QStringLiteral("tray.breakNow"), QStringLiteral("Start a break now")},
    {QStringLiteral("tray.pause"), QStringLiteral("Pause timer")},
    {QStringLiteral("tray.resume"), QStringLiteral("Resume timer")},
    {QStringLiteral("tray.quit"), QStringLiteral("Quit")},
    {QStringLiteral("tray.firstCycle"), QStringLiteral("DeskCat — set up your first session")},
    {QStringLiteral("phase.paused"), QStringLiteral("paused")},
    {QStringLiteral("phase.stopped"), QStringLiteral("not configured")},
    {QStringLiteral("phase.work"), QStringLiteral("work")},
    {QStringLiteral("phase.entering"), QStringLiteral("cat is coming")},
    {QStringLiteral("phase.rest"), QStringLiteral("rest")},
    {QStringLiteral("phase.leaving"), QStringLiteral("cat is waking up")},
};

const QVariantMap RussianTranslations {
    {QStringLiteral("unit.minutes"), QStringLiteral("мин")},
    {QStringLiteral("tagline"), QStringLiteral("Ваш личный хранитель перерывов")},
    {QStringLiteral("status.paused"), QStringLiteral("Таймер приостановлен")},
    {QStringLiteral("status.work"), QStringLiteral("Рабочий цикл")},
    {QStringLiteral("status.rest"), QStringLiteral("Кот отдыхает")},
    {QStringLiteral("status.entering"), QStringLiteral("Кот уже идёт")},
    {QStringLiteral("status.leaving"), QStringLiteral("Перерыв закончен")},
    {QStringLiteral("status.unconfigured"), QStringLiteral("Настройте первый цикл")},
    {QStringLiteral("status.nextAutomatic"), QStringLiteral("Следующая смена режима произойдёт автоматически")},
    {QStringLiteral("status.afterSaveTray"), QStringLiteral("После сохранения приложение останется в системном трее")},
    {QStringLiteral("section.cycle"), QStringLiteral("Длительность цикла")},
    {QStringLiteral("work.title"), QStringLiteral("Работа")},
    {QStringLiteral("work.subtitle"), QStringLiteral("Сколько кот не будет вас беспокоить")},
    {QStringLiteral("rest.title"), QStringLiteral("Отдых")},
    {QStringLiteral("rest.subtitle"), QStringLiteral("Сколько кот будет спать на экране")},
    {QStringLiteral("section.behavior"), QStringLiteral("Поведение")},
    {QStringLiteral("behavior.dim"), QStringLiteral("Затемнять фон во время отдыха")},
    {QStringLiteral("behavior.strict"), QStringLiteral("Строгий режим — для трудоголиков")},
    {QStringLiteral("behavior.autostart"), QStringLiteral("Запускать DeskCat при входе в систему")},
    {QStringLiteral("section.sound"), QStringLiteral("Звук")},
    {QStringLiteral("sound.walkingSteps"), QStringLiteral("Звуки шагов во время ходьбы")},
    {QStringLiteral("sound.walkingPurr"), QStringLiteral("Мурчание во время ходьбы")},
    {QStringLiteral("sound.restPurr"), QStringLiteral("Мурчание во время перерыва")},
    {QStringLiteral("section.language"), QStringLiteral("Язык")},
    {QStringLiteral("language.label"), QStringLiteral("Язык приложения")},
    {QStringLiteral("button.preview"), QStringLiteral("Посмотреть перерыв")},
    {QStringLiteral("button.resume"), QStringLiteral("Продолжить")},
    {QStringLiteral("button.pause"), QStringLiteral("Пауза")},
    {QStringLiteral("button.saveRestart"), QStringLiteral("Сохранить и перезапустить")},
    {QStringLiteral("button.saveStart"), QStringLiteral("Сохранить и начать")},
    {QStringLiteral("overlay.entering.title"), QStringLiteral("Время остановиться")},
    {QStringLiteral("overlay.leaving.title"), QStringLiteral("Отлично отдохнули")},
    {QStringLiteral("overlay.rest.title"), QStringLiteral("Кот спит — вы отдыхаете")},
    {QStringLiteral("overlay.rest.subtitle"), QStringLiteral("Отойдите от экрана, расправьте плечи и посмотрите вдаль")},
    {QStringLiteral("overlay.leaving.subtitle"), QStringLiteral("Кот просыпается — через мгновение можно продолжать работу")},
    {QStringLiteral("overlay.entering.subtitle"), QStringLiteral("Кот выходит и готовится охранять ваш отдых")},
    {QStringLiteral("overlay.hiss"), QStringLiteral("Ш-ш-ш… хватит, дай поспать")},
    {QStringLiteral("overlay.untilWake"), QStringLiteral("до пробуждения")},
    {QStringLiteral("overlay.wake"), QStringLiteral("Разбудить")},
    {QStringLiteral("tray.settings"), QStringLiteral("Открыть настройки")},
    {QStringLiteral("tray.breakNow"), QStringLiteral("Начать перерыв сейчас")},
    {QStringLiteral("tray.pause"), QStringLiteral("Приостановить таймер")},
    {QStringLiteral("tray.resume"), QStringLiteral("Продолжить таймер")},
    {QStringLiteral("tray.quit"), QStringLiteral("Выйти")},
    {QStringLiteral("tray.firstCycle"), QStringLiteral("DeskCat — настройте первый цикл")},
    {QStringLiteral("phase.paused"), QStringLiteral("пауза")},
    {QStringLiteral("phase.stopped"), QStringLiteral("не настроен")},
    {QStringLiteral("phase.work"), QStringLiteral("работа")},
    {QStringLiteral("phase.entering"), QStringLiteral("кот идёт")},
    {QStringLiteral("phase.rest"), QStringLiteral("отдых")},
    {QStringLiteral("phase.leaving"), QStringLiteral("кот просыпается")},
};
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
    }
    if (!m_configured || (m_trayEnabled && !m_trayIcon)) {
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

bool AppController::trayAvailable() const noexcept
{
    return m_trayIcon != nullptr;
}

QString AppController::language() const
{
    return m_language;
}

QVariantMap AppController::translations() const
{
    return m_language == QStringLiteral("ru")
        ? RussianTranslations
        : EnglishTranslations;
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
    const bool previousAutostart = m_autostart;
    m_autostart = PlatformIntegration::setAutostartEnabled(autostart)
        ? autostart
        : previousAutostart;
    m_walkingSounds = walkingSounds;
    m_walkingPurr = walkingPurr;
    m_restPurr = restPurr;
    emit settingsChanged();

    if (!m_configured) {
        m_configured = true;
        emit configuredChanged();
    }

    saveSettings();
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

void AppController::setLanguage(const QString &language)
{
    const QString normalized = language.trimmed().toLower();
    if ((normalized != QStringLiteral("en") && normalized != QStringLiteral("ru"))
        || normalized == m_language) {
        return;
    }

    m_language = normalized;
    QSettings settings(OrganizationName, ApplicationName);
    settings.setValue(QStringLiteral("language"), m_language);
    emit languageChanged();
    updateTray();
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

void AppController::resetApplicationState()
{
    PlatformIntegration::resetUserState();
    m_timer.stop();
    m_recentClicks.clear();
    m_previewRestSeconds = 0;

    m_workMinutes = 45;
    m_restMinutes = 10;
    m_dimBackground = true;
    m_strictMode = false;
    m_autostart = false;
    m_walkingSounds = true;
    m_walkingPurr = true;
    m_restPurr = true;
    m_language = QStringLiteral("en");

    const bool wasConfigured = m_configured;
    m_configured = false;
    emit settingsChanged();
    emit languageChanged();
    if (wasConfigured) {
        emit configuredChanged();
    }
    updateTray();
    showSettings();
}

void AppController::loadSettings()
{
    QSettings settings(OrganizationName, ApplicationName);
    m_configured = settings.value(QStringLiteral("configured"), false).toBool();
    m_workMinutes = settings.value(QStringLiteral("workMinutes"), 45).toInt();
    m_restMinutes = settings.value(QStringLiteral("restMinutes"), 10).toInt();
    m_dimBackground = settings.value(QStringLiteral("dimBackground"), true).toBool();
    m_strictMode = settings.value(QStringLiteral("strictMode"), false).toBool();
    m_autostart = settings.value(QStringLiteral("autostart"), false).toBool();
    m_walkingSounds = settings.value(QStringLiteral("walkingSounds"), true).toBool();
    m_walkingPurr = settings.value(QStringLiteral("walkingPurr"), true).toBool();
    m_restPurr = settings.value(QStringLiteral("restPurr"), true).toBool();
    const QString savedLanguage = settings.value(
        QStringLiteral("language"), QStringLiteral("en")).toString().toLower();
    m_language = savedLanguage == QStringLiteral("ru")
        ? QStringLiteral("ru")
        : QStringLiteral("en");
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
    settings.setValue(QStringLiteral("language"), m_language);
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
    m_settingsAction = m_trayMenu->addAction(QString());
    m_breakAction = m_trayMenu->addAction(QString());
    m_pauseAction = m_trayMenu->addAction(QString());
    m_trayMenu->addSeparator();
    m_quitAction = m_trayMenu->addAction(QString());

    connect(m_settingsAction, &QAction::triggered, this, &AppController::showSettings);
    connect(m_breakAction, &QAction::triggered, this, [this] {
        previewBreak(m_restMinutes * 60);
    });
    connect(m_pauseAction, &QAction::triggered, this, &AppController::togglePause);
    connect(m_quitAction, &QAction::triggered, this, &AppController::quitApplication);

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

    m_settingsAction->setText(text(QStringLiteral("tray.settings")));
    m_breakAction->setText(text(QStringLiteral("tray.breakNow")));
    m_quitAction->setText(text(QStringLiteral("tray.quit")));

    const QString label = phaseLabel(m_timer.phase(), m_timer.paused());
    const QString tooltip = m_timer.phase() == CountdownTimer::Phase::Stopped
        ? text(QStringLiteral("tray.firstCycle"))
        : QStringLiteral("DeskCat — %1 · %2").arg(label, remainingText());
    m_trayIcon->setToolTip(tooltip);

    if (m_pauseAction) {
        m_pauseAction->setEnabled(m_timer.phase() == CountdownTimer::Phase::Work);
        m_pauseAction->setText(m_timer.paused()
            ? text(QStringLiteral("tray.resume"))
            : text(QStringLiteral("tray.pause")));
    }
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

QString AppController::text(const QString &key) const
{
    const QVariantMap &catalog = m_language == QStringLiteral("ru")
        ? RussianTranslations
        : EnglishTranslations;
    const auto entry = catalog.constFind(key);
    return entry == catalog.cend() ? key : entry.value().toString();
}

QString AppController::phaseLabel(
    const CountdownTimer::Phase phase,
    const bool paused) const
{
    if (paused) {
        return text(QStringLiteral("phase.paused"));
    }
    switch (phase) {
    case CountdownTimer::Phase::Stopped:
        return text(QStringLiteral("phase.stopped"));
    case CountdownTimer::Phase::Work:
        return text(QStringLiteral("phase.work"));
    case CountdownTimer::Phase::Entering:
        return text(QStringLiteral("phase.entering"));
    case CountdownTimer::Phase::Rest:
        return text(QStringLiteral("phase.rest"));
    case CountdownTimer::Phase::Leaving:
        return text(QStringLiteral("phase.leaving"));
    }
    return QString();
}

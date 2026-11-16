#pragma once

#include "CountdownTimer.h"

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QAction;
class QMenu;
class QSystemTrayIcon;

class AppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY remainingSecondsChanged)
    Q_PROPERTY(QString remainingText READ remainingText NOTIFY remainingSecondsChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(int workMinutes READ workMinutes NOTIFY settingsChanged)
    Q_PROPERTY(int restMinutes READ restMinutes NOTIFY settingsChanged)
    Q_PROPERTY(QString breakMessage READ breakMessage NOTIFY settingsChanged)
    Q_PROPERTY(int breakMessageMaxLength READ breakMessageMaxLength CONSTANT)
    Q_PROPERTY(QString overlayBreakMessage READ overlayBreakMessage NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool overlayDimBackground READ overlayDimBackground NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool overlayStrictMode READ overlayStrictMode NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool overlayWalkingSounds READ overlayWalkingSounds NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool overlayWalkingPurr READ overlayWalkingPurr NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool overlayRestPurr READ overlayRestPurr NOTIFY overlaySettingsChanged)
    Q_PROPERTY(bool dimBackground READ dimBackground NOTIFY settingsChanged)
    Q_PROPERTY(bool strictMode READ strictMode NOTIFY settingsChanged)
    Q_PROPERTY(bool autostart READ autostart NOTIFY settingsChanged)
    Q_PROPERTY(bool walkingSounds READ walkingSounds NOTIFY settingsChanged)
    Q_PROPERTY(bool walkingPurr READ walkingPurr NOTIFY settingsChanged)
    Q_PROPERTY(bool restPurr READ restPurr NOTIFY settingsChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY configuredChanged)
    Q_PROPERTY(bool settingsVisible READ settingsVisible NOTIFY settingsVisibleChanged)
    Q_PROPERTY(bool trayAvailable READ trayAvailable CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QVariantMap translations READ translations NOTIFY languageChanged)

public:
    explicit AppController(
        bool trayEnabled,
        bool startHidden = false,
        QObject *parent = nullptr);
    ~AppController() override;

    [[nodiscard]] QString phase() const;
    [[nodiscard]] int remainingSeconds() const;
    [[nodiscard]] QString remainingText() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] int workMinutes() const noexcept;
    [[nodiscard]] int restMinutes() const noexcept;
    [[nodiscard]] QString breakMessage() const;
    [[nodiscard]] int breakMessageMaxLength() const noexcept;
    [[nodiscard]] QString overlayBreakMessage() const;
    [[nodiscard]] bool overlayDimBackground() const noexcept;
    [[nodiscard]] bool overlayStrictMode() const noexcept;
    [[nodiscard]] bool overlayWalkingSounds() const noexcept;
    [[nodiscard]] bool overlayWalkingPurr() const noexcept;
    [[nodiscard]] bool overlayRestPurr() const noexcept;
    [[nodiscard]] bool dimBackground() const noexcept;
    [[nodiscard]] bool strictMode() const noexcept;
    [[nodiscard]] bool autostart() const noexcept;
    [[nodiscard]] bool walkingSounds() const noexcept;
    [[nodiscard]] bool walkingPurr() const noexcept;
    [[nodiscard]] bool restPurr() const noexcept;
    [[nodiscard]] bool configured() const noexcept;
    [[nodiscard]] bool settingsVisible() const noexcept;
    [[nodiscard]] bool trayAvailable() const noexcept;
    [[nodiscard]] QString language() const;
    [[nodiscard]] QVariantMap translations() const;

    Q_INVOKABLE void applySettings(
        int workMinutes,
        int restMinutes,
        const QString &breakMessage,
        bool dimBackground,
        bool strictMode,
        bool autostart,
        bool walkingSounds,
        bool walkingPurr,
        bool restPurr);
    Q_INVOKABLE void showSettings();
    Q_INVOKABLE void hideSettings();
    Q_INVOKABLE void setLanguage(const QString &language);
    Q_INVOKABLE void previewBreak(
        int seconds,
        const QString &breakMessage,
        bool dimBackground,
        bool strictMode,
        bool walkingSounds,
        bool walkingPurr,
        bool restPurr);
    Q_INVOKABLE void beginRest();
    Q_INVOKABLE void completeLeaving();
    Q_INVOKABLE void finishBreakEarly();
    Q_INVOKABLE void registerCatClick();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void quitApplication();
    void resetApplicationState();

signals:
    void phaseChanged();
    void remainingSecondsChanged();
    void pausedChanged();
    void settingsChanged();
    void overlaySettingsChanged();
    void configuredChanged();
    void settingsVisibleChanged();
    void settingsActivationRequested();
    void languageChanged();
    void catPokeRequested();
    void catHissRequested();

private:
    void loadSettings();
    void saveSettings();
    void startWork();
    void createTrayIcon();
    void updateTray();
    [[nodiscard]] QString text(const QString &key) const;
    static QString formatSeconds(int totalSeconds);
    [[nodiscard]] QString phaseLabel(CountdownTimer::Phase phase, bool paused) const;

    CountdownTimer m_timer;
    int m_workMinutes = 45;
    int m_restMinutes = 10;
    QString m_breakMessage;
    int m_previewRestSeconds = 0;
    QString m_previewBreakMessage;
    bool m_previewSettingsActive = false;
    bool m_previewDimBackground = true;
    bool m_previewStrictMode = false;
    bool m_previewWalkingSounds = true;
    bool m_previewWalkingPurr = true;
    bool m_previewRestPurr = true;
    bool m_dimBackground = true;
    bool m_strictMode = false;
    bool m_autostart = false;
    bool m_walkingSounds = true;
    bool m_walkingPurr = true;
    bool m_restPurr = true;
    bool m_configured = false;
    bool m_settingsVisible = false;
    bool m_trayEnabled = true;
    QString m_language = QStringLiteral("en");

    QElapsedTimer m_clickClock;
    QList<qint64> m_recentClicks;

    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_breakAction = nullptr;
    QAction *m_pauseAction = nullptr;
    QAction *m_quitAction = nullptr;
};

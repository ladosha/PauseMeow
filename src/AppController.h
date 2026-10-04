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
    explicit AppController(bool trayEnabled, QObject *parent = nullptr);
    ~AppController() override;

    [[nodiscard]] QString phase() const;
    [[nodiscard]] int remainingSeconds() const;
    [[nodiscard]] QString remainingText() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] int workMinutes() const noexcept;
    [[nodiscard]] int restMinutes() const noexcept;
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
        bool dimBackground,
        bool strictMode,
        bool autostart,
        bool walkingSounds,
        bool walkingPurr,
        bool restPurr);
    Q_INVOKABLE void showSettings();
    Q_INVOKABLE void hideSettings();
    Q_INVOKABLE void setLanguage(const QString &language);
    Q_INVOKABLE void previewBreak(int seconds);
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
    void configuredChanged();
    void settingsVisibleChanged();
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
    int m_previewRestSeconds = 0;
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

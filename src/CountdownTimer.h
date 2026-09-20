#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class CountdownTimer final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Phase phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY remainingSecondsChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

public:
    enum class Phase {
        Stopped,
        Work,
        Entering,
        Rest,
        Leaving
    };
    Q_ENUM(Phase)

    explicit CountdownTimer(QObject *parent = nullptr);

    [[nodiscard]] Phase phase() const noexcept;
    [[nodiscard]] int remainingSeconds() const noexcept;
    [[nodiscard]] bool paused() const noexcept;

    void startWork(int seconds);
    void beginEntering();
    void startRest(int seconds);
    void beginLeaving();
    void stop();

public slots:
    void togglePause();

signals:
    void phaseChanged();
    void remainingSecondsChanged();
    void pausedChanged();
    void workExpired();
    void restExpired();

private slots:
    void updateCountdown();

private:
    void startTimedPhase(Phase phase, int seconds);
    void setPhase(Phase phase);
    void setRemainingSeconds(int seconds);
    [[nodiscard]] qint64 currentRemainingMilliseconds() const;

    Phase m_phase = Phase::Stopped;
    QTimer m_tickTimer;
    QElapsedTimer m_elapsed;
    qint64 m_remainingMilliseconds = 0;
    int m_displayedSeconds = 0;
    bool m_paused = false;
};


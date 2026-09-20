#include "CountdownTimer.h"

#include <QtGlobal>

CountdownTimer::CountdownTimer(QObject *parent)
    : QObject(parent)
{
    m_tickTimer.setInterval(100);
    m_tickTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_tickTimer, &QTimer::timeout, this, &CountdownTimer::updateCountdown);
}

CountdownTimer::Phase CountdownTimer::phase() const noexcept
{
    return m_phase;
}

int CountdownTimer::remainingSeconds() const noexcept
{
    return m_displayedSeconds;
}

bool CountdownTimer::paused() const noexcept
{
    return m_paused;
}

void CountdownTimer::startWork(const int seconds)
{
    startTimedPhase(Phase::Work, seconds);
}

void CountdownTimer::beginEntering()
{
    m_tickTimer.stop();
    m_elapsed.invalidate();
    m_remainingMilliseconds = 0;
    setRemainingSeconds(0);
    setPhase(Phase::Entering);
}

void CountdownTimer::startRest(const int seconds)
{
    startTimedPhase(Phase::Rest, seconds);
}

void CountdownTimer::beginLeaving()
{
    m_tickTimer.stop();
    m_elapsed.invalidate();
    m_remainingMilliseconds = 0;
    setRemainingSeconds(0);
    setPhase(Phase::Leaving);
}

void CountdownTimer::stop()
{
    m_tickTimer.stop();
    m_elapsed.invalidate();
    m_remainingMilliseconds = 0;
    setRemainingSeconds(0);
    if (m_paused) {
        m_paused = false;
        emit pausedChanged();
    }
    setPhase(Phase::Stopped);
}

void CountdownTimer::togglePause()
{
    if (m_phase != Phase::Work) {
        return;
    }

    if (!m_paused) {
        m_remainingMilliseconds = currentRemainingMilliseconds();
        m_tickTimer.stop();
        m_elapsed.invalidate();
        m_paused = true;
    } else {
        m_elapsed.restart();
        m_tickTimer.start();
        m_paused = false;
    }

    emit pausedChanged();
}

void CountdownTimer::updateCountdown()
{
    const qint64 remaining = currentRemainingMilliseconds();
    const int seconds = static_cast<int>((remaining + 999) / 1000);
    setRemainingSeconds(seconds);

    if (remaining > 0) {
        return;
    }

    m_tickTimer.stop();
    m_elapsed.invalidate();
    m_remainingMilliseconds = 0;

    if (m_phase == Phase::Work) {
        setPhase(Phase::Entering);
        emit workExpired();
    } else if (m_phase == Phase::Rest) {
        setPhase(Phase::Leaving);
        emit restExpired();
    }
}

void CountdownTimer::startTimedPhase(const Phase phase, const int seconds)
{
    m_tickTimer.stop();
    m_paused = false;
    emit pausedChanged();

    m_remainingMilliseconds = qMax(1, seconds) * 1000LL;
    m_elapsed.restart();
    setRemainingSeconds(qMax(1, seconds));
    setPhase(phase);
    m_tickTimer.start();
}

void CountdownTimer::setPhase(const Phase phase)
{
    if (m_phase == phase) {
        return;
    }
    m_phase = phase;
    emit phaseChanged();
}

void CountdownTimer::setRemainingSeconds(const int seconds)
{
    if (m_displayedSeconds == seconds) {
        return;
    }
    m_displayedSeconds = seconds;
    emit remainingSecondsChanged();
}

qint64 CountdownTimer::currentRemainingMilliseconds() const
{
    if (!m_elapsed.isValid()) {
        return qMax<qint64>(0, m_remainingMilliseconds);
    }
    return qMax<qint64>(0, m_remainingMilliseconds - m_elapsed.elapsed());
}


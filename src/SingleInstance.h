#pragma once

#include <QLocalServer>
#include <QObject>

class SingleInstance final : public QObject
{
    Q_OBJECT

public:
    enum class StartResult {
        Primary,
        Secondary,
        Error,
    };

    explicit SingleInstance(QObject *parent = nullptr);

    [[nodiscard]] StartResult start();

signals:
    void activationRequested();

private:
    [[nodiscard]] bool notifyRunningInstance() const;
    void acceptConnections();

    QLocalServer m_server;
};

#pragma once

#include <QLocalServer>
#include <QObject>
#include <QString>

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

    [[nodiscard]] StartResult start(const QString &command = QStringLiteral("activate"));

signals:
    void activationRequested();
    void resetRequested();

private:
    [[nodiscard]] bool notifyRunningInstance(const QString &command) const;
    void acceptConnections();

    QLocalServer m_server;
};

#include "SingleInstance.h"

#include <QIODevice>
#include <QLocalSocket>

namespace {
constexpr auto ServerName = "PauseMeow.SingleInstance.v1";
constexpr int ConnectionTimeoutMs = 500;
}

SingleInstance::SingleInstance(QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QLocalServer::newConnection,
        this, &SingleInstance::acceptConnections);
}

SingleInstance::StartResult SingleInstance::start(const QString &command)
{
    if (notifyRunningInstance(command)) {
        return StartResult::Secondary;
    }

    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    if (m_server.listen(QString::fromLatin1(ServerName))) {
        return StartResult::Primary;
    }

    // Another process may have won the race between our connection and
    // listen attempts. Try it once more before treating the endpoint as stale.
    if (notifyRunningInstance(command)) {
        return StartResult::Secondary;
    }

    QLocalServer::removeServer(QString::fromLatin1(ServerName));
    return m_server.listen(QString::fromLatin1(ServerName))
        ? StartResult::Primary
        : StartResult::Error;
}

bool SingleInstance::notifyRunningInstance(const QString &command) const
{
    QLocalSocket socket;
    socket.connectToServer(
        QString::fromLatin1(ServerName), QIODevice::WriteOnly);
    if (!socket.waitForConnected(ConnectionTimeoutMs)) {
        return false;
    }

    socket.write(command.toUtf8());
    socket.write("\n");
    socket.waitForBytesWritten(ConnectionTimeoutMs);
    socket.disconnectFromServer();
    return true;
}

void SingleInstance::acceptConnections()
{
    while (m_server.hasPendingConnections()) {
        QLocalSocket *socket = m_server.nextPendingConnection();
        if (!socket) {
            continue;
        }

        if (socket->bytesAvailable() == 0) {
            socket->waitForReadyRead(ConnectionTimeoutMs);
        }
        const QString command = QString::fromUtf8(socket->readAll()).trimmed();
        connect(socket, &QLocalSocket::disconnected,
            socket, &QObject::deleteLater);
        socket->disconnectFromServer();

        if (command == QStringLiteral("reset")) {
            emit resetRequested();
        } else {
            emit activationRequested();
        }
    }
}

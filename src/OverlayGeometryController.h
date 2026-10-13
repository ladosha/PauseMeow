#pragma once

#include <QAbstractNativeEventFilter>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTimer>

class QScreen;
class QWindow;

class OverlayGeometryController final
    : public QObject
    , public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit OverlayGeometryController(QObject *parent = nullptr);
    ~OverlayGeometryController() override;

    Q_INVOKABLE void registerWindow(QObject *object);
    Q_INVOKABLE void unregisterWindow(QObject *object);

    bool nativeEventFilter(
        const QByteArray &eventType,
        void *message,
        qintptr *result) override;

private:
    void watchScreen(QScreen *screen);
    void scheduleRefresh();
    void refreshAll();
    void refreshWindow(QWindow *window);

    QList<QPointer<QWindow>> m_windows;
    QList<QPointer<QScreen>> m_watchedScreens;
    QTimer m_refreshTimer;
    QTimer m_settleTimer;
};

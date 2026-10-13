#include "OverlayGeometryController.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

namespace {
constexpr auto RefreshDelayMs = 100;
constexpr auto SettleDelayMs = 700;
}

OverlayGeometryController::OverlayGeometryController(QObject *parent)
    : QObject(parent)
{
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(RefreshDelayMs);
    m_settleTimer.setSingleShot(true);
    m_settleTimer.setInterval(SettleDelayMs);

    connect(&m_refreshTimer, &QTimer::timeout,
        this, &OverlayGeometryController::refreshAll);
    connect(&m_settleTimer, &QTimer::timeout,
        this, &OverlayGeometryController::refreshAll);
    connect(qGuiApp, &QGuiApplication::screenAdded,
        this, [this](QScreen *screen) {
            watchScreen(screen);
            scheduleRefresh();
        });
    connect(qGuiApp, &QGuiApplication::screenRemoved,
        this, [this] { scheduleRefresh(); });

    for (QScreen *screen : QGuiApplication::screens()) {
        watchScreen(screen);
    }
    qGuiApp->installNativeEventFilter(this);
}

OverlayGeometryController::~OverlayGeometryController()
{
    qGuiApp->removeNativeEventFilter(this);
}

void OverlayGeometryController::registerWindow(QObject *object)
{
    auto *window = qobject_cast<QWindow *>(object);
    if (!window) {
        return;
    }

    for (const auto &registeredWindow : std::as_const(m_windows)) {
        if (registeredWindow == window) {
            scheduleRefresh();
            return;
        }
    }

    m_windows.append(window);
    connect(window, &QWindow::screenChanged,
        this, [this](QScreen *screen) {
            watchScreen(screen);
            scheduleRefresh();
        });
    connect(window, &QWindow::visibleChanged,
        this, [this](bool visible) {
            if (visible) {
                scheduleRefresh();
            }
        });
    connect(window, &QObject::destroyed,
        this, [this] { scheduleRefresh(); });

    watchScreen(window->screen());
    scheduleRefresh();
}

void OverlayGeometryController::unregisterWindow(QObject *object)
{
    auto *window = qobject_cast<QWindow *>(object);
    m_windows.removeIf([window](const QPointer<QWindow> &candidate) {
        return candidate.isNull() || candidate == window;
    });
}

bool OverlayGeometryController::nativeEventFilter(
    const QByteArray &eventType,
    void *message,
    qintptr *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

#ifdef Q_OS_WIN
    const auto *nativeMessage = static_cast<MSG *>(message);
    if (nativeMessage
        && (nativeMessage->message == WM_DISPLAYCHANGE
            || nativeMessage->message == WM_DPICHANGED
            || nativeMessage->message == WM_SETTINGCHANGE)) {
        scheduleRefresh();
    }
#else
    Q_UNUSED(message)
#endif

    return false;
}

void OverlayGeometryController::watchScreen(QScreen *screen)
{
    if (!screen) {
        return;
    }

    for (const auto &watchedScreen : std::as_const(m_watchedScreens)) {
        if (watchedScreen == screen) {
            return;
        }
    }

    m_watchedScreens.append(screen);
    connect(screen, &QScreen::geometryChanged,
        this, [this] { scheduleRefresh(); });
    connect(screen, &QScreen::availableGeometryChanged,
        this, [this] { scheduleRefresh(); });
    connect(screen, &QScreen::virtualGeometryChanged,
        this, [this] { scheduleRefresh(); });
    connect(screen, &QScreen::logicalDotsPerInchChanged,
        this, [this] { scheduleRefresh(); });
    connect(screen, &QScreen::orientationChanged,
        this, [this] { scheduleRefresh(); });
    connect(screen, &QObject::destroyed,
        this, [this] {
            m_watchedScreens.removeIf([](const QPointer<QScreen> &candidate) {
                return candidate.isNull();
            });
            scheduleRefresh();
        });
}

void OverlayGeometryController::scheduleRefresh()
{
    m_refreshTimer.start();
    m_settleTimer.start();
}

void OverlayGeometryController::refreshAll()
{
    m_windows.removeIf([](const QPointer<QWindow> &window) {
        return window.isNull();
    });

    for (const auto &window : std::as_const(m_windows)) {
        if (window) {
            refreshWindow(window);
        }
    }
}

void OverlayGeometryController::refreshWindow(QWindow *window)
{
    QScreen *screen = window->screen();
    if (!screen) {
        return;
    }

    window->setGeometry(screen->geometry());

#ifdef Q_OS_WIN
    const HWND windowHandle = reinterpret_cast<HWND>(window->winId());
    const HMONITOR monitor = MonitorFromWindow(
        windowHandle,
        MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (monitor && GetMonitorInfoW(monitor, &monitorInfo)) {
        const RECT &bounds = monitorInfo.rcMonitor;
        SetWindowPos(
            windowHandle,
            HWND_TOPMOST,
            bounds.left,
            bounds.top,
            bounds.right - bounds.left,
            bounds.bottom - bounds.top,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
#endif
}

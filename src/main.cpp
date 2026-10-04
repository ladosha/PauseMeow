#include "AppController.h"
#include "SingleInstance.h"
#include "PlatformIntegration.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("DeskCat"));
    QApplication::setOrganizationName(QStringLiteral("DeskCat"));
    QApplication::setApplicationVersion(QStringLiteral(DESKCAT_VERSION));
#ifdef Q_OS_LINUX
    QApplication::setDesktopFileName(QStringLiteral("com.deskcat.DeskCat"));
#endif
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/app_icon.png")));
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("DeskCat focus and rest timer"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(
        QStringLiteral("demo"),
        QStringLiteral("Start an eight-second visual break preview.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("smoke-test"),
        QStringLiteral("Load QML and exit automatically.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("reset-settings"),
        QStringLiteral("Clear settings and remove DeskCat from autostart before launch.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("uninstall"),
        QStringLiteral("Remove per-user DeskCat settings and autostart registration, then exit.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("background"),
        QStringLiteral("Start quietly when already configured.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("show-settings"),
        QStringLiteral("Open the settings window after launch.")));
    parser.process(app);

    const bool uninstallRequested = parser.isSet(QStringLiteral("uninstall"));
    const bool resetRequested = parser.isSet(QStringLiteral("reset-settings"));
    if (uninstallRequested || resetRequested) {
        PlatformIntegration::resetUserState();
    }

    const bool smokeTest = parser.isSet(QStringLiteral("smoke-test"));
    QPointer<AppController> controllerInstance;
    SingleInstance singleInstance;
    QObject::connect(&singleInstance, &SingleInstance::activationRequested,
        &app, [&controllerInstance] {
            if (controllerInstance) {
                controllerInstance->showSettings();
            }
        });
    QObject::connect(&singleInstance, &SingleInstance::resetRequested,
        &app, [&controllerInstance] {
            if (controllerInstance) {
                controllerInstance->resetApplicationState();
            }
        });
    QObject::connect(&singleInstance, &SingleInstance::uninstallRequested,
        &app, [&app] {
            PlatformIntegration::resetUserState();
            app.quit();
        });

    if (!smokeTest) {
        const QString instanceCommand = uninstallRequested
            ? QStringLiteral("uninstall")
            : resetRequested ? QStringLiteral("reset") : QStringLiteral("activate");
        switch (singleInstance.start(instanceCommand)) {
        case SingleInstance::StartResult::Secondary:
            return 0;
        case SingleInstance::StartResult::Error:
            qCritical() << "Could not establish the DeskCat single-instance endpoint.";
            return 3;
        case SingleInstance::StartResult::Primary:
            break;
        }
    }

    if (uninstallRequested) {
        return 0;
    }

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller(!smokeTest);
    controllerInstance = &controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(
        QStringLiteral("appController"),
        &controller);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [] {
            QCoreApplication::exit(2);
        }, Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("DeskCat"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return 2;
    }

    if (parser.isSet(QStringLiteral("show-settings"))) {
        controller.showSettings();
    }

    if (parser.isSet(QStringLiteral("demo"))) {
        QTimer::singleShot(700, &controller, [&controller] {
            controller.previewBreak(8);
        });
    }

    if (smokeTest) {
        QTimer::singleShot(700, &app, &QCoreApplication::quit);
    }

    return app.exec();
}

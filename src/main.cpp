#include "AppController.h"
#include "OverlayGeometryController.h"
#include "SingleInstance.h"
#include "PlatformIntegration.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStringList>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PauseMeow"));
    QApplication::setOrganizationName(QStringLiteral("PauseMeow"));
    QApplication::setApplicationVersion(QStringLiteral(PAUSEMEOW_VERSION));
#ifdef Q_OS_LINUX
    QApplication::setDesktopFileName(QStringLiteral("com.vladmas.PauseMeow"));
#endif
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/app_icon.png")));
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("PauseMeow focus and rest timer"));
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
        QStringLiteral("Clear settings and remove PauseMeow from autostart before launch.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("background"),
        QStringLiteral("Start quietly when already configured.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("show-settings"),
        QStringLiteral("Open the settings window after launch.")));
    parser.process(app);

    const bool resetRequested = parser.isSet(QStringLiteral("reset-settings"));
    if (resetRequested) {
        PlatformIntegration::resetUserState();
    }

    const bool smokeTest = parser.isSet(QStringLiteral("smoke-test"));
    if (smokeTest) {
        const QStringList requiredResources {
            QStringLiteral(":/assets/app_icon.png"),
            QStringLiteral(":/assets/audio/cat_steps.ogg"),
            QStringLiteral(":/assets/audio/cat_purr.ogg"),
            QStringLiteral(":/assets/audio/cat_meow.ogg"),
            QStringLiteral(":/assets/sprites/cat_walk.png"),
            QStringLiteral(":/assets/sprites/cat_lie_down.png"),
            QStringLiteral(":/assets/sprites/cat_sleep.png"),
            QStringLiteral(":/assets/sprites/cat_poke.png"),
            QStringLiteral(":/assets/sprites/cat_hiss.png"),
            QStringLiteral(":/assets/sprites/cat_settle.png"),
            QStringLiteral(":/assets/sprites/cat_wake_yawn.png"),
            QStringLiteral(":/assets/sprites/cat_turn.png"),
        };
        for (const QString &resource : requiredResources) {
            if (!QFile::exists(resource)) {
                qCritical() << "Required resource is missing:" << resource;
                return 4;
            }
        }
    }
    const bool backgroundRequested = parser.isSet(QStringLiteral("background"));
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
    if (!smokeTest) {
        const QString instanceCommand = resetRequested
            ? QStringLiteral("reset") : QStringLiteral("activate");
        switch (singleInstance.start(instanceCommand)) {
        case SingleInstance::StartResult::Secondary:
            return 0;
        case SingleInstance::StartResult::Error:
            qCritical() << "Could not establish the PauseMeow single-instance endpoint.";
            return 3;
        case SingleInstance::StartResult::Primary:
            break;
        }
    }

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller(!smokeTest, backgroundRequested || smokeTest);
    OverlayGeometryController overlayGeometryController;
    controllerInstance = &controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(
        QStringLiteral("appController"),
        &controller);
    engine.rootContext()->setContextProperty(
        QStringLiteral("overlayGeometryController"),
        &overlayGeometryController);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [] {
            QCoreApplication::exit(2);
        }, Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("PauseMeow"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return 2;
    }

    if (parser.isSet(QStringLiteral("show-settings"))) {
        controller.showSettings();
    }

    if (parser.isSet(QStringLiteral("demo"))) {
        QTimer::singleShot(700, &controller, [&controller] {
            controller.previewBreak(
                8,
                controller.breakMessage(),
                controller.dimBackground(),
                controller.strictMode(),
                controller.walkingSounds(),
                controller.walkingPurr(),
                controller.restPurr());
        });
    }

    if (smokeTest) {
        QTimer::singleShot(700, &app, &QCoreApplication::quit);
    }

    return app.exec();
}

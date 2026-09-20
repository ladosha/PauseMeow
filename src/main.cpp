#include "AppController.h"
#include "SingleInstance.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("DeskCat"));
    QApplication::setOrganizationName(QStringLiteral("DeskCat"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
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
        QStringLiteral("Clear DeskCat application settings before launch.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("background"),
        QStringLiteral("Start quietly when already configured.")));
    parser.process(app);

    const bool smokeTest = parser.isSet(QStringLiteral("smoke-test"));
    QPointer<AppController> controllerInstance;
    SingleInstance singleInstance;
    QObject::connect(&singleInstance, &SingleInstance::activationRequested,
        &app, [&controllerInstance] {
            if (controllerInstance) {
                controllerInstance->showSettings();
            }
        });

    if (!smokeTest) {
        switch (singleInstance.start()) {
        case SingleInstance::StartResult::Secondary:
            return 0;
        case SingleInstance::StartResult::Error:
            qCritical() << "Could not establish the DeskCat single-instance endpoint.";
            return 3;
        case SingleInstance::StartResult::Primary:
            break;
        }
    }

    if (parser.isSet(QStringLiteral("reset-settings"))) {
        QSettings settings(QStringLiteral("DeskCat"), QStringLiteral("DeskCat"));
        settings.clear();
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

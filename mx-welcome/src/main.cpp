/**********************************************************************
 * Copyright (C) 2015-2026 MX Authors
 *
 * This file is part of mx-welcome and is distributed under GPL-3.0-or-later.
 **********************************************************************/

#include "backend.h"
#include "version.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>
#include <QQuickStyle>
#include <QTranslator>
#include <QUrl>
#include <QtGlobal>

#include <cstdlib>
#include <unistd.h>

static QString bundledIconPath()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return QStringLiteral(":/qt/qml/MxWelcome/icons/mx-welcome.svg");
#else
    return QStringLiteral(":/MxWelcome/icons/mx-welcome.svg");
#endif
}

class ThemeIconProvider final : public QQuickImageProvider
{
public:
    ThemeIconProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        const QSize target = requestedSize.isValid() ? requestedSize : QSize(64, 64);
        const QIcon fallback(bundledIconPath());
        const QPixmap pixmap = QIcon::fromTheme(id, fallback).pixmap(target);
        if (size) {
            *size = pixmap.size();
        }
        return pixmap;
    }
};

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")
        && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qWarning("mx-welcome: no display available (DISPLAY and WAYLAND_DISPLAY are both unset); "
                "a graphical session is required to run this program.");
        return EXIT_FAILURE;
    }

    QApplication::setOrganizationName(QStringLiteral("MX-Linux"));
    QApplication::setApplicationName(QStringLiteral("mx-welcome"));
    QApplication::setApplicationDisplayName(QStringLiteral("MX Welcome"));
    QApplication::setApplicationVersion(QStringLiteral(VERSION));

    QApplication app(argc, argv);
    QApplication::setWindowIcon(
        QIcon::fromTheme(QStringLiteral("mx-welcome"), QIcon(bundledIconPath())));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("This tool displays the MX Linux welcome screen."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"a", "about"},
                      QObject::tr("Start with About selected. The About page provides basic information about the "
                                  "current MX Linux version, the user's hardware, and access to a full system report.")});
    parser.addOption({{"t", "test"}, QObject::tr("Run a test mode.")});
    parser.process(app);

    QTranslator qtTranslator;
    if (qtTranslator.load(QStringLiteral("qt_") + QLocale().name(),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTranslator);
    }
    QTranslator qtBaseTranslator;
    if (qtBaseTranslator.load(QStringLiteral("qtbase_") + QLocale::system().name(),
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTranslator);
    }
    QTranslator appTranslator;
    if (appTranslator.load(QApplication::applicationName() + QLatin1Char('_') + QLocale::system().name(),
                           QStringLiteral("/usr/share/mx-welcome/locale"))) {
        QApplication::installTranslator(&appTranslator);
    }

    if (getuid() == 0) {
        QApplication::beep();
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("You must run this program as normal user."));
        return EXIT_FAILURE;
    }

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")
        && qgetenv("QT_STYLE_OVERRIDE").toLower() == "gtk2") {
        // QT_STYLE_OVERRIDE=gtk2 is a widget-only style name with no corresponding Qt Quick
        // Controls style module. Left alone, the platform theme's style hint propagates it to
        // Quick Controls, which then fails to load. Only override in that specific case, so
        // every other environment still gets its native platform style (see README.md).
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
    }

    Backend backend(parser);
    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("icons"), new ThemeIconProvider);
    engine.setInitialProperties({{QStringLiteral("backend"), QVariant::fromValue(&backend)}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    engine.loadFromModule(QStringLiteral("MxWelcome"), QStringLiteral("Main"));
#else
    engine.load(QUrl(QStringLiteral("qrc:/MxWelcome/qml/Main.qml")));
#endif

    return QApplication::exec();
}

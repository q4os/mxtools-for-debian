#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "core/app_init.h"
#include "core/cmd.h"
#include "cli/controller.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setProperty("cliMode", true);
    app.setProperty("cliQuietTerminal", true); // don't mirror logs to terminal; still write to file
    QCoreApplication::setApplicationName("mx-boot-repair");
    QCoreApplication::setOrganizationName("MX-Linux");

    AppInit::setupRootEnv();
    AppInit::installTranslations();
    AppInit::setupLogging();

    CliController controller;
    const int code = controller.run();
    if (QFile::exists("/usr/bin/pkexec")) {
        Cmd().run("pkexec /usr/lib/mx-boot-repair/mxbr-lib copy_log", nullptr, nullptr, QuietMode::Yes);
    } else {
        Cmd().runAsRoot("/usr/lib/mx-boot-repair/mxbr-lib copy_log", nullptr, nullptr, QuietMode::Yes);
    }
    return code;
}

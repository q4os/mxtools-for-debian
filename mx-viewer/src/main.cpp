/*****************************************************************************
 * main.cpp
 *****************************************************************************
 * Copyright (C) 2022-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Viewer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "historystore.h"
#include "mainwindow.h"
#include "singleinstance.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QStandardPaths>
#include <QTranslator>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <grp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

// Avoid the cost of Qt Quick's OpenGL rendering on software drivers.
bool openGLIsSoftwareRendered()
{
    QOpenGLContext context;
    QOffscreenSurface surface;
    surface.create();
    if (!context.create() || !context.makeCurrent(&surface)) {
        return false;
    }
    const auto *renderer = reinterpret_cast<const char *>(context.functions()->glGetString(GL_RENDERER));
    const QString name = QString::fromLatin1(renderer ? renderer : "");
    context.doneCurrent();
    return name.contains("llvmpipe") || name.contains("softpipe") || name.contains("SwiftShader")
           || name.contains("Software Rasterizer");
}

namespace
{
constexpr uid_t nobodyId = 65534; // nobody (uid 65534), nogroup (gid 65534)

// The account to run as after starting as root; the defaults are 'nobody'.
struct Account {
    uid_t uid {nobodyId};
    gid_t gid {nobodyId};
    QByteArray name;
    QByteArray home;
};

// What the root process could use to reach the display but 'nobody' cannot reach by itself: the X
// authority cookie and a connection to the Wayland compositor, both taken before dropping privileges.
struct DisplayAccess {
    QByteArray xauthority;
    int waylandFd {-1};
};

// The private home made for 'nobody', removed at exit.
QByteArray temporaryHome;

// -n/--force-nobody, read before QApplication and its parser exist. Short options may be combined
// ("-jn"), as QCommandLineParser accepts them; a single-dash word with any other letter is one of Qt's
// own options ("-session", "-platformpluginpath") and does not count.
bool forceNobodyRequested(int argc, char *argv[])
{
    constexpr std::string_view shortOptions {"fijnshv?"};
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--") {
            break;
        }
        if (arg == "--force-nobody") {
            return true;
        }
        if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-'
            && arg.find_first_not_of(shortOptions, 1) == std::string_view::npos && arg.find('n') != std::string_view::npos) {
            return true;
        }
    }
    return false;
}

// The user who started this root process, as sudo or pkexec record it, or else the session's login name.
Account invokingAccount()
{
    const passwd *pw {nullptr};
    for (const char *variable : {"SUDO_UID", "PKEXEC_UID"}) {
        bool ok {false};
        const uint uid = qEnvironmentVariable(variable).toUInt(&ok);
        if (ok && (pw = getpwuid(uid))) {
            break;
        }
    }
    if (!pw) {
        if (const char *login = getlogin()) {
            pw = getpwnam(login);
        }
    }
    if (!pw || pw->pw_uid == 0 || pw->pw_gid == 0) {
        qDebug() << "Could not find the user who started the program, dropping privileges to nobody";
        return {};
    }
    return {pw->pw_uid, pw->pw_gid, pw->pw_name, pw->pw_dir};
}

// Reads the X cookie and connects to the Wayland compositor for 'nobody'. XAUTHORITY and WAYLAND_DISPLAY
// come from the environment of whoever started the program, so this runs with that user's rights: root
// must not be made to read a file or open a socket the user could not, then hand it to 'nobody'.
// Without a known user, only root's own cookie file is used.
bool collectDisplayAccess(const Account &invoker, DisplayAccess &access)
{
    constexpr qint64 maxCookieSize = 64 * 1024;
    if (invoker.name.isEmpty()) {
        const passwd *root = getpwuid(0);
        if (!qEnvironmentVariableIsEmpty("DISPLAY") && root) {
            QFile file(QFile::decodeName(root->pw_dir) + "/.Xauthority");
            if (file.open(QIODevice::ReadOnly)) {
                access.xauthority = file.read(maxCookieSize);
            }
        }
        return true;
    }
    if (initgroups(invoker.name.constData(), invoker.gid) != 0 || setegid(invoker.gid) != 0
        || seteuid(invoker.uid) != 0) {
        return false;
    }
    if (!qEnvironmentVariableIsEmpty("DISPLAY")) {
        QString path = qEnvironmentVariable("XAUTHORITY");
        if (path.isEmpty()) {
            path = QFile::decodeName(invoker.home) + "/.Xauthority";
        }
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            access.xauthority = file.read(maxCookieSize);
        }
    }
    const QString wayland = qEnvironmentVariable("WAYLAND_DISPLAY");
    if (!wayland.isEmpty()) {
        const QByteArray path = QFile::encodeName(
            wayland.startsWith('/') ? wayland : qEnvironmentVariable("XDG_RUNTIME_DIR") + '/' + wayland);
        sockaddr_un address {};
        address.sun_family = AF_UNIX;
        if (static_cast<size_t>(path.size()) < sizeof(address.sun_path)) {
            std::memcpy(address.sun_path, path.constData(), path.size());
            const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
            if (fd >= 0 && connect(fd, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) == 0) {
                access.waylandFd = fd;
            } else if (fd >= 0) {
                close(fd);
            }
        }
    }
    // Back to root for the rest of the drop.
    return seteuid(0) == 0 && setegid(0) == 0;
}

// After the drop, HOME, USER and the XDG directories still name root's; point them at the new user's.
bool setUpEnvironment(Account account, const DisplayAccess &display)
{
    if (account.name.isEmpty()) {
        // 'nobody' has no home of its own, so it gets a private one for this run.
        char path[] = "/tmp/mx-viewer-XXXXXX";
        if (!mkdtemp(path)) {
            return false;
        }
        temporaryHome = path;
        // At process exit, after QtWebEngine has shut down and stopped writing into it.
        std::atexit([] { QDir(QString::fromLocal8Bit(temporaryHome)).removeRecursively(); });
        account.name = "nobody";
        account.home = path;
    }
    qputenv("HOME", account.home);
    qputenv("USER", account.name);
    qputenv("LOGNAME", account.name);
    for (const char *variable : {"XDG_CONFIG_HOME", "XDG_DATA_HOME", "XDG_CACHE_HOME", "XDG_STATE_HOME"}) {
        qunsetenv(variable);
    }
    const QByteArray runtimeDir = "/run/user/" + QByteArray::number(account.uid);
    struct stat info {};
    if (stat(runtimeDir.constData(), &info) == 0 && S_ISDIR(info.st_mode) && info.st_uid == account.uid) {
        qputenv("XDG_RUNTIME_DIR", runtimeDir);
    } else {
        qunsetenv("XDG_RUNTIME_DIR");
    }
    if (!display.xauthority.isEmpty()) {
        QFile cookie(QString::fromLocal8Bit(account.home) + "/.Xauthority");
        if (!cookie.open(QIODevice::WriteOnly) || cookie.write(display.xauthority) != display.xauthority.size()) {
            return false;
        }
        cookie.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        qputenv("XAUTHORITY", QFile::encodeName(cookie.fileName()));
    } else if (!qEnvironmentVariableIsEmpty("XAUTHORITY")
               && access(qgetenv("XAUTHORITY").constData(), R_OK) != 0) {
        // Root's cookie file; without it Xlib uses the one in the new HOME.
        qunsetenv("XAUTHORITY");
    }
    if (display.waylandFd >= 0) {
        qputenv("WAYLAND_SOCKET", QByteArray::number(display.waylandFd));
    }
    return true;
}

// Drop rights of the program to the user who started it, or to 'nobody' if that user is unknown or
// when asked. Done before QApplication exists, so no Qt plugin and no display connection is set up as root.
bool dropElevatedPrivileges(bool forceNobody)
{
    if (getuid() != 0 && geteuid() != 0) {
        return true;
    }

    // ref:
    // https://www.safaribooksonline.com/library/view/secure-programming-cookbook/0596003943/ch01s03.html#secureprgckbk-CHP-1-SECT-3.3
    const Account invoker = invokingAccount();
    const Account account = forceNobody ? Account {} : invoker;
    const bool nobody = account.name.isEmpty();
    DisplayAccess display;
    if (nobody && !collectDisplayAccess(invoker, display)) {
        return false;
    }

    // Replace root's supplementary groups with the target user's groups (or none for 'nobody')
    // before changing the primary gid, while we still have the privilege to do so.
    if (nobody ? setgroups(0, nullptr) != 0 : initgroups(account.name.constData(), account.gid) != 0) {
        return false;
    }
    if (setgid(account.gid) != 0) {
        return false;
    }
    if (setuid(account.uid) != 0) {
        return false;
    }

    // On systems with defined _POSIX_SAVED_IDS in the unistd.h file, it should be
    // impossible to regain elevated privs after the setuid() call, above.  Test, try to regain elev priv:
    if (setuid(0) != -1 || seteuid(0) != -1 || setgid(0) != -1 || setegid(0) != -1) {
        return false; // and the calling fn should EXIT/abort the program
    }

    // Change working directory to /tmp for security and compatibility reasons.
    // After dropping privileges, the original cwd might not be accessible to the new user,
    // potentially causing issues with file operations or signals. /tmp is world-writable
    // and a standard location for temporary operations.
    if (chdir("/tmp") != 0) {
        qDebug() << "Can't change working directory to /tmp";
        return false;
    }
    return setUpEnvironment(account, display);
}
} // namespace

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")
        && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qWarning("mx-viewer: no display available (DISPLAY and WAYLAND_DISPLAY are both unset); "
                "a graphical session is required to run this program.");
        return EXIT_FAILURE;
    }

    // Before QApplication, so its plugins and the display connection are never set up as root.
    const bool startedAsRoot = getuid() == 0 || geteuid() == 0;
    if (!dropElevatedPrivileges(startedAsRoot && forceNobodyRequested(argc, argv))) {
        qWarning("mx-viewer: could not drop elevated privileges");
        return EXIT_FAILURE;
    }

    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QGuiApplication::setQuitOnLastWindowClosed(true);
    // Set Qt platform to XCB (X11) if not already set and we're in X11 environment
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        if (!qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
            qputenv("QT_QPA_PLATFORM", "xcb");
        }
    }

    // Debian's hunspell-* packages ship Chromium-format dictionaries here; QtWebEngine
    // only looks next to the binary or in the Qt data path unless told otherwise.
    if (qEnvironmentVariableIsEmpty("QTWEBENGINE_DICTIONARIES_PATH") && QDir("/usr/share/hunspell-bdic").exists()) {
        qputenv("QTWEBENGINE_DICTIONARIES_PATH", "/usr/share/hunspell-bdic");
    }

    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setApplicationVersion(VERSION);
    QApplication::setOrganizationName("MX-Linux");

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("This tool will display the URL content in a window, window title is optional"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"f", "full-screen"}, QObject::tr("Start program in full-screen mode")});
    parser.addOption({{"i", "disable-images"}, QObject::tr("Disable load images automatically from websites")});
    parser.addOption({{"j", "disable-js"}, QObject::tr("Disable JavaScript")});
    if (startedAsRoot) {
        parser.addOption(
            {{"n", "force-nobody"},
             QObject::tr("Drop program's rights to 'nobody'. By default, if run as root, the rights are "
                         "dropped to normal user. This option might provide additional protection, but the program "
                         "would not be able to write its cache and cookies to the user directory, so it might break "
                         "some functionality.")});
    }
    parser.addOption({{"s", "enable-spatial-navigation"}, QObject::tr("Enable spatial navigation with keyboard")});
    parser.addPositionalArgument(QObject::tr("URL"),
                                 QObject::tr("URL of the page you want to load")
                                     + "\ne.g., https://google.com, google.com, file:///home/user/file.html");
    parser.addPositionalArgument(QObject::tr("Title"), QObject::tr("Window title for the viewer"), "[title]");
    parser.process(app);

    // A plain launch (how links from other applications arrive) opens in the running browser.
    // Help-viewer style calls with a title or options always get their own window, and so does a
    // launch as root.
    const bool plainLaunch
        = !startedAsRoot && parser.optionNames().isEmpty() && parser.positionalArguments().size() <= 1;
    if (plainLaunch) {
        if (SingleInstance::forward(parser.positionalArguments().value(0))) {
            return EXIT_SUCCESS;
        }
        SingleInstance::listen(&app, &MainWindow::openFromOtherInstance);
    }

    // Only for a window this process shows itself, not for a link passed on to the running browser.
    // Probe before the first Qt Quick window is created.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_BACKEND") && openGLIsSoftwareRendered()) {
        qputenv("QT_QUICK_BACKEND", "software");
    }

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    // The packages install to /usr/share/mx-viewer/locale; a copy in the user's data directory comes first.
    QTranslator appTran;
    const QStringList localeDirs = QStandardPaths::locateAll(
        QStandardPaths::GenericDataLocation, QApplication::applicationName() + "/locale", QStandardPaths::LocateDirectory);
    for (const QString &localeDir : localeDirs) {
        if (appTran.load(QApplication::applicationName() + "_" + QLocale::system().name(), localeDir)) {
            QApplication::installTranslator(&appTran);
            break;
        }
    }

    auto *window = new MainWindow(parser);
    window->show();

    // Ensure proper cleanup on application exit
    // Close every browser window (including private ones) so each saves its state
    QObject::connect(&app, &QApplication::aboutToQuit, [] {
        MainWindow::setQuitting();
        const auto widgets = QApplication::topLevelWidgets();
        for (auto *widget : widgets) {
            if (qobject_cast<MainWindow *>(widget) && !widget->isHidden()) {
                widget->close();
            }
        }
    });

    const int result = QApplication::exec();
    MainWindow::releaseSharedProfile();
    HistoryStore::close();
    return result;
}

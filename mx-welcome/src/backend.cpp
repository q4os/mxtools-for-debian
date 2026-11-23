#include "backend.h"

#include "version.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTimer>
#include <QTextStream>

#include <functional>
#include <optional>

namespace
{
QString configuredValue(const QSettings &system, const QSettings &defaults, const QString &key,
                        const QVariant &fallback = {})
{
    return system.value(key, defaults.value(key, fallback)).toString();
}

void runInxi(QObject *owner, const QStringList &arguments, const std::function<void(const QString &)> &onResult)
{
    auto *process = new QProcess(owner);
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process->setProcessEnvironment(environment);
    const auto finish = [process, onResult](const QString &output) {
        process->deleteLater();
        onResult(output);
    };
    QObject::connect(process, &QProcess::errorOccurred, owner, [finish](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            finish({});
        }
    });
    QObject::connect(process, &QProcess::finished, owner, [process, finish](int code, QProcess::ExitStatus status) {
        finish(status == QProcess::NormalExit && code == 0
                   ? QString::fromUtf8(process->readAllStandardOutput()).trimmed() : QString());
    });
    QTimer::singleShot(10000, process, &QProcess::kill);
    process->start(QStringLiteral("inxi"), arguments);
}

QString autostartDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart");
}

// Preserve desktop-entry escapes, localized keys and unrelated groups verbatim.
struct LoginEntry {
    QByteArray contents;
    bool disabled = false;
    bool hasLaunchCommand = false;
};

std::optional<LoginEntry> readLoginEntry(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    LoginEntry entry;
    bool desktopGroup = false;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine();
        const QByteArray trimmed = line.trimmed();
        if (trimmed.startsWith('[')) {
            desktopGroup = trimmed == "[Desktop Entry]";
        } else if (desktopGroup) {
            const auto separator = trimmed.indexOf('=');
            const QByteArray key = trimmed.left(separator).trimmed();
            const QByteArray value = trimmed.mid(separator + 1).trimmed();
            if ((key == "Hidden" && value == "true") || (key == "X-GNOME-Autostart-enabled" && value == "false")) {
                entry.disabled = true;
                continue;
            }
            if ((key == "Exec" && !value.isEmpty()) || (key == "DBusActivatable" && value == "true")) {
                entry.hasLaunchCommand = true;
            }
        }
        entry.contents += line;
    }
    if (file.error() != QFileDevice::NoError) {
        return std::nullopt;
    }
    return entry;
}

bool enableLoginEntry(const QString &target)
{
    const QString source = QStringLiteral("/usr/share/mx-welcome/mx-welcome.desktop");
    if (!QFileInfo::exists(target)) {
        return QFile::copy(source, target);
    }
    auto entry = readLoginEntry(target);
    if (!entry) {
        return false;
    }
    if (!entry->hasLaunchCommand) {
        // A desktop session can leave just a Hidden=true mask behind.
        entry = readLoginEntry(source);
        if (!entry || !entry->hasLaunchCommand) {
            return false;
        }
    } else if (!entry->disabled) {
        return true;
    }
    QSaveFile file(target);
    return file.open(QIODevice::WriteOnly) && file.write(entry->contents) == entry->contents.size() && file.commit();
}

}

Backend::Backend(const QCommandLineParser &parser, QObject *parent)
    : QObject(parent)
{
    m_startOnAbout = parser.isSet(QStringLiteral("about"));
    QSettings settings;
    m_initialWindowSize = QSize(settings.value(QStringLiteral("Window/windowWidth"), 760).toInt(),
                                settings.value(QStringLiteral("Window/windowHeight"), 720).toInt());
    loadConfiguration(parser.isSet(QStringLiteral("test")));
    rebuildActions();
    runInxi(this, {QStringLiteral("-c"), QStringLiteral("0"), QStringLiteral("-S")},
            [this](const QString &output) { updateDesktopVersion(output); });
}

ActionModel *Backend::actions()
{
    return &m_actions;
}

QString Backend::distroTitle() const { return m_distroTitle; }
QString Backend::distroVersion() const { return m_distroVersion; }
QString Backend::debianVersion() const { return m_debianVersion; }
QString Backend::desktopVersion() const { return m_desktopVersion; }
QString Backend::supportedUntil() const { return m_supportedUntil; }
QString Backend::termsSummary() const { return m_termsSummary; }
QString Backend::liveUserInfo() const { return m_liveUserInfo; }
QUrl Backend::headerSource() const { return m_headerSource; }
QUrl Backend::logoSource() const { return m_logoSource; }
QString Backend::systemInfo() const { return m_systemInfo; }
bool Backend::autoStartup() const { return m_autoStartup; }
bool Backend::liveSession() const { return m_liveSession; }
bool Backend::showLiveUserInfo() const { return m_showLiveUserInfo; }
bool Backend::startOnAbout() const { return m_startOnAbout; }
QSize Backend::initialWindowSize() const { return m_initialWindowSize; }
QString Backend::version() const { return QStringLiteral(VERSION); }

void Backend::saveWindowSize(const QSize &size)
{
    QSettings settings;
    settings.setValue(QStringLiteral("Window/windowWidth"), size.width());
    settings.setValue(QStringLiteral("Window/windowHeight"), size.height());
}

void Backend::setAutoStartup(bool enabled)
{
    if (m_autoStartup == enabled) {
        return;
    }

    const QString targetDir = autostartDirectory();
    const QString target = targetDir + QStringLiteral("/mx-welcome.desktop");
    bool succeeded = true;
    if (enabled) {
        succeeded = QDir().mkpath(targetDir) && enableLoginEntry(target);
    } else {
        succeeded = !QFileInfo::exists(target) || QFile::remove(target);
    }

    if (!succeeded) {
        emit errorOccurred(tr("Error"), tr("Could not update the login startup setting."));
        return;
    }

    m_autoStartup = enabled;
    emit autoStartupChanged();
}

void Backend::activate(const QString &identifier)
{
    const auto *definition = findAction(identifier);
    if (!definition || !definition->visible || !definition->action.enabled) {
        return;
    }

    if (!definition->command.isEmpty()) {
        launchCommand(definition->command);
    } else if (!definition->defaultCommand.isEmpty()) {
        const bool manual = identifier == QLatin1String("manual");
        if (manual && m_desktopProbePending) {
            m_manualRequested = true;
            return;
        }
        const QString program = manual && m_isFluxbox ? QStringLiteral("mxfb-help") : definition->defaultCommand;
        if (!QProcess::startDetached(program)) {
            emit errorOccurred(tr("Error"), tr("Could not start the requested action."));
        }
    } else if (definition->defaultUrl.isValid() && !QDesktopServices::openUrl(definition->defaultUrl)) {
        emit errorOccurred(tr("Error"), tr("Could not open %1").arg(definition->defaultUrl.toString()));
    }
}

void Backend::requestSystemInfo()
{
    if (m_reportRequested) {
        return;
    }
    m_reportRequested = true;
    runInxi(this, {QStringLiteral("-c"), QStringLiteral("0")}, [this](const QString &output) {
        m_systemInfo = output.isEmpty() ? tr("System information is unavailable.") : output;
        emit systemInfoChanged();
    });
}

void Backend::openFullSystemReport()
{
    if (m_debianVersion.section(QLatin1Char('.'), 0, 0) == QLatin1String("10")) {
        launchCommand(QStringLiteral("x-terminal-emulator -e bash -c \"/usr/bin/quick-system-info-mx\""));
        return;
    }
    if (!QProcess::startDetached(QStringLiteral("/usr/bin/quick-system-info-gui"))) {
        emit errorOccurred(tr("Error"), tr("Could not start Quick System Info."));
    }
}

QString Backend::termsText()
{
    if (!m_termsCommand.isEmpty()) {
        launchCommand(m_termsCommand);
        return {};
    }
    const QString text = runCommand(QStringLiteral("zless"), {QStringLiteral("/usr/share/mx-welcome/TOS")});
    return text.isEmpty() ? tr("Could not load the Terms of Use.") : text;
}

QString Backend::licenseHtml() const
{
    QFile file(QStringLiteral("/usr/share/doc/mx-welcome/license.html"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("Could not load %1").arg(file.fileName());
    }
    return QString::fromUtf8(file.readAll());
}

QString Backend::changelogText() const
{
    const QString path = QStringLiteral("/usr/share/doc/") + QCoreApplication::applicationName()
                         + QStringLiteral("/changelog.gz");
    const QString text = runCommand(QStringLiteral("zcat"), {path}, 3000);
    return text.isEmpty() ? tr("Could not load changelog.") : text;
}

QString Backend::runCommand(const QString &program, const QStringList &arguments, int timeoutMs)
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(timeoutMs)) {
        return {};
    }
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        return {};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

QUrl Backend::localSource(const QString &path)
{
    return QFileInfo::exists(path) ? QUrl::fromLocalFile(path) : QUrl();
}

void Backend::loadConfiguration(bool testMode)
{
    const QString oldConfig = QDir::homePath() + QStringLiteral("/.config/")
                              + QCoreApplication::applicationName() + QStringLiteral(".conf");
    QFile::remove(oldConfig);

    const QDir autostartDir(autostartDirectory());
    // /etc/skel seeds this marker so new accounts see the welcome dialog once on
    // first login; remove it after that single launch without touching the
    // user's own autostart entry, which may have been enabled independently
    // (e.g. via the desktop environment's session settings).
    QFile::remove(autostartDir.filePath(QStringLiteral("mx-welcome-once.desktop")));
    const auto loginEntry = readLoginEntry(autostartDir.filePath(QStringLiteral("mx-welcome.desktop")));
    m_autoStartup = loginEntry && loginEntry->hasLaunchCommand && !loginEntry->disabled;

    QSettings defaults(QStringLiteral("/usr/share/mx-welcome/mx-welcome.conf"), QSettings::NativeFormat);
    QSettings system(QStringLiteral("/etc/mx-welcome/mx-welcome.conf"), QSettings::NativeFormat);

    QString distro = configuredValue(system, defaults, QStringLiteral("DISTRO"));
    QString codename = configuredValue(system, defaults, QStringLiteral("CODENAME"));
    m_supportedUntil = configuredValue(system, defaults, QStringLiteral("SUPPORTED"));
    m_termsSummary = configuredValue(system, defaults, QStringLiteral("TOSTEXT"));
    if (m_termsSummary.isEmpty()) {
        m_termsSummary = tr("The name “MX Linux” is covered by Linux Foundation Sublicense No. 20140605-0483. "
                            "We develop software that is covered by a free license that can be examined in the Wiki list. "
                            "We also include software developed by others that is under a free license.");
    }
    m_termsCommand = configuredValue(system, defaults, QStringLiteral("TOSCMD"));
    m_headerSource = localSource(configuredValue(system, defaults, QStringLiteral("HEADER")));
    m_logoSource = localSource(configuredValue(system, defaults, QStringLiteral("LOGO")));

    const QString showLive = configuredValue(system, defaults, QStringLiteral("SHOWLIVEUSERINFO"), QStringLiteral("true"));
    m_liveUserInfo = configuredValue(system, defaults, QStringLiteral("LIVEUSERINFOTEXT"));
    if (m_liveUserInfo.isEmpty()) {
        m_liveUserInfo = tr("User demo, password: demo. Superuser root, password: root.");
    }

    const QByteArray rootFileSystem = QStorageInfo::root().fileSystemType();
    m_liveSession = rootFileSystem == "aufs" || rootFileSystem == "overlay";
    //hide password information when using root persistence or force-passwd is present
    const bool persistRoot = QFileInfo::exists(QStringLiteral("/live/config/persist-root"));
    const bool newPasswords = QFileInfo::exists(QStringLiteral("/live/config/force-passwd"));
    m_showLiveUserInfo = testMode || (m_liveSession && !newPasswords && !persistRoot && showLive != QLatin1String("false"));

    const QString checkRelease = configuredValue(system, defaults, QStringLiteral("CHECKLSB_RELEASE"),
                                                  QStringLiteral("true"));
    if (checkRelease != QLatin1String("false") && QFileInfo::exists(QStringLiteral("/etc/lsb-release"))) {
        QSettings release(QStringLiteral("/etc/lsb-release"), QSettings::NativeFormat);
        distro = release.value(QStringLiteral("DISTRIB_ID")).toString() + QLatin1Char('-')
                 + release.value(QStringLiteral("DISTRIB_RELEASE")).toString();
        codename = release.value(QStringLiteral("DISTRIB_CODENAME")).toString();
    }
    m_distroVersion = distro;
    m_distroTitle = codename.isEmpty() ? distro : tr("%1 “%2”").arg(distro, codename);

    QFile debianFile(QStringLiteral("/etc/debian_version"));
    if (debianFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_debianVersion = QString::fromUtf8(debianFile.readLine()).trimmed();
    }

    struct Seed {
        const char *identifier;
        int configIndex;
        QString title;
        QString description;
        const char *themeIcon;
        QString defaultCommand;
        const char *defaultUrl;
    };
    const QVector<Seed> seeds = {
        {"setup", 1, tr("Install MX Linux"), tr("Install MX Linux on this computer."), "system-software-install", QStringLiteral("minstall-launcher"), nullptr},
        {"faq", 2, tr("FAQ"), tr("Find answers to frequently asked questions."), "help-faq", QStringLiteral("mx-faq"), nullptr},
        {"forum", 3, tr("Forums"), tr("Ask questions and join the MX Linux community."), "system-users", {}, "http://forum.mxlinux.org/index.php"},
        {"manual", 4, tr("Users Manual"), tr("Read the documentation for your MX Linux release."), "help-contents", QStringLiteral("mx-manual"), nullptr},
        {"video", 5, tr("Videos"), tr("Watch MX Linux tutorials and demonstrations."), "video-x-generic", {}, "http://www.mxlinux.org/videos/"},
        {"wiki", 6, tr("Wiki"), tr("Browse community-maintained guides and reference material."), "internet-web-browser", {}, "http://www.mxlinux.org/wiki"},
        {"contribute", 7, tr("Contribute"), tr("Learn how to support and contribute to MX Linux."), "help-donate", {}, "http://www.mxlinux.org/donate"},
        {"tools", 8, tr("Tools"), tr("Open the collection of MX system utilities."), "preferences-system", QStringLiteral("mx-tools"), nullptr},
        {"packages", 9, tr("Popular Apps"), tr("Discover and install popular applications."), "system-software-install", QStringLiteral("mx-packageinstaller"), nullptr},
        {"tweak", 10, tr("Tweak (Panel, etc...)"), tr("Adjust desktop, panel, and system preferences."), "preferences-desktop", QStringLiteral("mx-tweak"), nullptr},
        {"tour", 11, tr("Tour"), tr("Take a guided tour of MX Linux."), "start-here", QStringLiteral("mx-tour"), nullptr},
    };

    for (const auto &seed : seeds) {
        const QString prefix = QString::number(seed.configIndex);
        const QString configuredTitle = configuredValue(system, defaults, prefix + QStringLiteral("text"));
        const QString configuredIcon = configuredValue(system, defaults, prefix + QStringLiteral("icon"));
        const QString configuredDescription = configuredValue(system, defaults, prefix + QStringLiteral("description"));
        ActionDefinition definition;
        definition.action.identifier = QString::fromLatin1(seed.identifier);
        definition.action.title = configuredTitle.isEmpty() ? seed.title : configuredTitle;
        definition.action.description = configuredDescription.isEmpty() ? seed.description : configuredDescription;
        definition.action.iconSource = localSource(configuredIcon);
        if (definition.action.iconSource.isEmpty()) {
            definition.action.iconSource = QUrl(QStringLiteral("image://icons/") + QString::fromLatin1(seed.themeIcon));
        }
        definition.command = configuredValue(system, defaults, prefix + QStringLiteral("command"));
        definition.defaultCommand = seed.defaultCommand;
        definition.defaultUrl = seed.defaultUrl ? QUrl(QString::fromLatin1(seed.defaultUrl)) : QUrl();
        if (definition.action.identifier == QLatin1String("setup")) {
            definition.visible = m_liveSession || testMode;
        } else if (definition.action.identifier == QLatin1String("tour")) {
            definition.visible = !configuredTitle.isEmpty() || QFileInfo::exists(QStringLiteral("/usr/bin/mx-tour"));
        }
        m_definitions.append(std::move(definition));
    }
}

void Backend::updateDesktopVersion(const QString &sessionInfo)
{
    m_desktopProbePending = false;
    const QRegularExpression desktopPattern(QStringLiteral(R"(Desktop:\s*(.+?)\s+v:\s*([\d\.]+))"));
    const auto match = desktopPattern.match(sessionInfo);
    if (match.hasMatch()) {
        m_desktopVersion = match.captured(1) + QLatin1Char(' ') + match.captured(2);
        m_isFluxbox = m_desktopVersion.contains(QStringLiteral("Fluxbox"));
        if (m_isFluxbox) {
            QFile file(QStringLiteral("/etc/mxfb_version"));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString version = QString::fromUtf8(file.readLine()).trimmed();
                if (!version.isEmpty()) {
                    m_desktopVersion += QLatin1Char(' ') + version;
                }
            }
        }
        emit desktopVersionChanged();
    }
    if (m_manualRequested) {
        activate(QStringLiteral("manual"));
    }
}

void Backend::rebuildActions()
{
    QVector<WelcomeAction> filtered;
    for (const auto &definition : m_definitions) {
        if (!definition.visible) {
            continue;
        }
        filtered.append(definition.action);
    }
    m_actions.setActions(std::move(filtered));
}

void Backend::launchCommand(const QString &command)
{
    if (!QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command})) {
        emit errorOccurred(tr("Error"), tr("Could not start the requested action."));
    }
}

const Backend::ActionDefinition *Backend::findAction(const QString &identifier) const
{
    for (const auto &definition : m_definitions) {
        if (definition.action.identifier == identifier) {
            return &definition;
        }
    }
    return nullptr;
}

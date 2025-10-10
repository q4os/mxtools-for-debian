/**********************************************************************
 *  ConkyManager.cpp
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include "cmd.h"
#include "conkymanager.h"
#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

ConkyManager::ConkyManager(QObject *parent)
    : QObject(parent),
      m_statusProcess(new QProcess(this)),
      m_autostartTimer(new QTimer(this)),
      m_statusTimer(new QTimer(this)),
      m_statusCheckRunning(false),
      m_startupDelay(5)
{
    m_statusTimer->setInterval(2s);
    connect(m_statusTimer, &QTimer::timeout, this, &ConkyManager::updateRunningStatus);
    m_statusTimer->start();

    m_autostartTimer->setSingleShot(true);
    connect(m_autostartTimer, &QTimer::timeout, this, &ConkyManager::onAutostartTimer);

    // Setup async status process
    connect(m_statusProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &ConkyManager::onStatusProcessFinished);

    loadSettings();
}

ConkyManager::~ConkyManager()
{
    saveSettings();
    clearConkyItems();
}

void ConkyManager::addSearchPath(const QString &path)
{
    if (!m_searchPaths.contains(path)) {
        m_searchPaths.append(path);
        scanForConkies();
        emit conkyItemsChanged();
    }
}

void ConkyManager::removeSearchPath(const QString &path)
{
    if (m_searchPaths.removeOne(path)) {
        scanForConkies();
        emit conkyItemsChanged();
    }
}

QStringList ConkyManager::searchPaths() const
{
    return m_searchPaths;
}

void ConkyManager::scanForConkies()
{
    clearConkyItems();

    QMap<QString, QStringList> conkyFolders; // folderName -> list of fullPaths
    QString userConkyPath = QDir::homePath() + "/.conky";

    // First, collect all conky folders from all search paths
    for (const QString &path : m_searchPaths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
        for (const QString &subdir : subdirs) {
            QString fullPath = dir.absoluteFilePath(subdir);

            // Collect all paths for each folder name
            if (!conkyFolders.contains(subdir)) {
                conkyFolders[subdir] = QStringList();
            }
            conkyFolders[subdir].append(fullPath);
        }
    }

    // Now scan folders with priority logic for "All" view
    // but keep all versions for filtering
    for (auto it = conkyFolders.begin(); it != conkyFolders.end(); ++it) {
        const QStringList &paths = it.value();

        // Sort paths to prioritize ~/.conky first
        QStringList sortedPaths = paths;
        std::sort(sortedPaths.begin(), sortedPaths.end(), [&userConkyPath](const QString &a, const QString &b) {
            bool aIsUser = a.startsWith(userConkyPath);
            bool bIsUser = b.startsWith(userConkyPath);

            // ~/.conky paths come first
            if (aIsUser && !bIsUser) {
                return true;
            }
            if (!aIsUser && bIsUser) {
                return false;
            }
            return a < b; // Alphabetical for same type
        });

        // Scan all versions - the filtering will be done at display level
        for (const QString &path : sortedPaths) {
            scanConkyDirectory(path);
        }
    }

    // Sort conky items by folder name, then by filename within each folder
    std::sort(m_conkyItems.begin(), m_conkyItems.end(), [](const ConkyItem *a, const ConkyItem *b) {
        QString folderA = QFileInfo(a->directory()).fileName().toLower();
        QString folderB = QFileInfo(b->directory()).fileName().toLower();

        if (folderA != folderB) {
            return folderA < folderB;
        }

        // Same folder, sort by filename
        QString fileA = QFileInfo(a->filePath()).fileName().toLower();
        QString fileB = QFileInfo(b->filePath()).fileName().toLower();
        return fileA < fileB;
    });

    applyAutostartFromStartupScript();

    emit conkyItemsChanged();
}

void ConkyManager::addConkyItemsFromDirectory(const QString &directoryPath)
{
    if (!QFileInfo::exists(directoryPath) || !QFileInfo(directoryPath).isDir()) {
        return;
    }

    // Scan the specific directory and add any new items
    scanConkyDirectory(directoryPath);

    // Sort conky items by folder name, then by filename within each folder (maintain consistent ordering)
    std::sort(m_conkyItems.begin(), m_conkyItems.end(), [](const ConkyItem *a, const ConkyItem *b) {
        QString folderA = QFileInfo(a->directory()).fileName().toLower();
        QString folderB = QFileInfo(b->directory()).fileName().toLower();

        if (folderA != folderB) {
            return folderA < folderB;
        }

        // Same folder, sort by filename
        QString fileA = QFileInfo(a->filePath()).fileName().toLower();
        QString fileB = QFileInfo(b->filePath()).fileName().toLower();
        return fileA < fileB;
    });

    applyAutostartFromStartupScript();

    emit conkyItemsChanged();
}

QList<ConkyItem *> ConkyManager::conkyItems() const
{
    return m_conkyItems;
}

void ConkyManager::startConky(ConkyItem *item)
{
    if (!item || item->isRunning()) {
        return;
    }

    // Use startDetached to ensure conky remains running after app closes
    QString program = "conky";
    QStringList arguments;
    arguments << "-c" << item->filePath();

    bool started = QProcess::startDetached(program, arguments, item->directory());

    if (!started) {
        return;
    }
    item->setRunning(true);
    emit conkyStarted(item);
}

void ConkyManager::stopConky(ConkyItem *item)
{
    if (!item || !item->isRunning()) {
        return;
    }

    QString pid = getConkyProcess(item->filePath());
    if (!pid.isEmpty()) {
        // Use synchronous approach to avoid lingering processes
        QProcess process;
        process.setProgram("kill");
        process.setArguments(QStringList() << pid);
        process.start();

        if (process.waitForFinished(3000)) {
            // Process killed successfully or failed
        } else {
            // Timeout - force kill the kill process
            process.kill();
            process.waitForFinished(1000);
        }

        item->setRunning(false);
        emit conkyStopped(item);
    }
}

void ConkyManager::removeConkyItem(ConkyItem *item)
{
    if (!item) {
        return;
    }

    // Stop the conky if it's running
    if (item->isRunning()) {
        stopConky(item);
    }

    // Remove from the list
    m_conkyItems.removeAll(item);

    // Delete the item
    item->deleteLater();

    // Emit signal to update UI
    emit conkyItemsChanged();
}

void ConkyManager::startAutostart()
{
    // Start all enabled conkies immediately
    for (ConkyItem *item : m_conkyItems) {
        if (item->isAutostart()) {
            startConky(item);
        }
    }
}

void ConkyManager::stopAllRunning()
{
    for (ConkyItem *item : m_conkyItems) {
        if (item->isRunning()) {
            stopConky(item);
        }
    }
}

void ConkyManager::saveSettings()
{
    m_settings.beginGroup("ConkyManager");
    m_settings.setValue("searchPaths", m_searchPaths);
    m_settings.setValue("startupDelay", m_startupDelay);
    m_settings.endGroup();

    // Update startup script whenever settings are saved
    updateStartupScript();
}

void ConkyManager::loadSettings()
{
    m_settings.beginGroup("ConkyManager");

    m_searchPaths
        = m_settings
              .value("searchPaths", QStringList() << QDir::homePath() + "/.conky" << "/usr/share/mx-conky-data/themes")
              .toStringList();

    m_startupDelay = m_settings.value("startupDelay", 5).toInt();
    m_settings.endGroup();

    scanForConkies();
}

void ConkyManager::updateRunningStatus()
{
    // Skip if previous status check is still running
    if (m_statusCheckRunning || m_statusProcess->state() != QProcess::NotRunning) {
        return;
    }

    m_statusCheckRunning = true;

    // Use pattern matching to find conky processes (handles /usr/bin/conky, conky, etc.)
    m_statusProcess->start("pgrep", QStringList() << "conky");
}

void ConkyManager::onStatusProcessFinished()
{
    m_statusCheckRunning = false;

    if (m_statusProcess->exitCode() != 0 && m_statusProcess->exitCode() != 1) {
        // pgrep returns 1 when no processes found, which is normal
        // Only worry about other exit codes
        return;
    }

    QString output = m_statusProcess->readAllStandardOutput();
    QStringList pids = output.split('\n', Qt::SkipEmptyParts);

    // Build set of running config files by checking each PID
    QSet<QString> runningConfigs;
    for (const QString &pid : pids) {
        // Get command line for this PID
        QProcess cmdlineProcess;
        cmdlineProcess.start("ps", QStringList() << "-p" << pid << "-o"
                                                 << "command="
                                                 << "--no-headers");
        if (cmdlineProcess.waitForFinished(1000)) {
            QString cmdline = cmdlineProcess.readAllStandardOutput().trimmed();

            // Skip if this is not actually a conky process (could be grep, mx-conky, etc.)
            // Look for conky executable (conky, /usr/bin/conky, etc.) but not mx-conky application itself
            if (!cmdline.contains("conky") || cmdline.contains("./mx-conky") || cmdline.endsWith("mx-conky")) {
                continue;
            }

            // Find -c flag position in the command line
            int cFlagIndex = cmdline.indexOf(" -c ");
            if (cFlagIndex != -1) {
                // Extract everything after " -c " as the config path
                QString configPath = cmdline.mid(cFlagIndex + 4); // +4 for " -c "
                QString absolutePath = QFileInfo(configPath).absoluteFilePath();
                runningConfigs.insert(absolutePath);
            }
        }
    }

    // Check each item against the running configs
    for (ConkyItem *item : m_conkyItems) {
        bool wasRunning = item->isRunning();
        QString configFilePath = QFileInfo(item->filePath()).absoluteFilePath();
        bool isRunning = runningConfigs.contains(configFilePath);

        if (wasRunning != isRunning) {
            item->setRunning(isRunning);
            if (isRunning) {
                emit conkyStarted(item);
            } else {
                emit conkyStopped(item);
            }
        }
    }
}

void ConkyManager::onAutostartTimer()
{
    if (m_autostartQueue.isEmpty()) {
        return;
    }

    ConkyItem *item = m_autostartQueue.takeFirst();
    startConky(item);

    if (!m_autostartQueue.isEmpty()) {
        ConkyItem *nextItem = m_autostartQueue.first();
        int delay = nextItem->autostartDelay() * 1000;
        if (delay <= 0) {
            delay = 1000;
        }
        m_autostartTimer->start(delay);
    }
}

void ConkyManager::clearConkyItems()
{
    qDeleteAll(m_conkyItems);
    m_conkyItems.clear();
}

QString ConkyManager::getConkyProcess(const QString &configPath) const
{
    QString absolutePath = QFileInfo(configPath).absoluteFilePath();

    // Get all conky PIDs using pattern matching (handles /usr/bin/conky, conky, etc.)
    QProcess pgrepProcess;
    pgrepProcess.start("pgrep", QStringList() << "conky");
    pgrepProcess.waitForFinished(3000);

    QString pgrepOutput = pgrepProcess.readAllStandardOutput();
    QStringList pids = pgrepOutput.split('\n', Qt::SkipEmptyParts);

    // Check each PID to find the one with matching config path
    for (const QString &pid : pids) {
        QProcess cmdlineProcess;
        cmdlineProcess.start("ps", QStringList() << "-p" << pid << "-o"
                                                 << "command="
                                                 << "--no-headers");
        if (cmdlineProcess.waitForFinished(1000)) {
            QString cmdline = cmdlineProcess.readAllStandardOutput().trimmed();

            // Skip if this is not actually a conky process (could be grep, mx-conky, etc.)
            // Look for conky executable (conky, /usr/bin/conky, etc.) but not mx-conky application itself
            if (!cmdline.contains("conky") || cmdline.contains("./mx-conky") || cmdline.endsWith("mx-conky")) {
                continue;
            }

            // Find -c flag position in the command line
            int cFlagIndex = cmdline.indexOf(" -c ");
            if (cFlagIndex != -1) {
                // Extract everything after " -c " as the config path
                QString configPath = cmdline.mid(cFlagIndex + 4); // +4 for " -c "
                QString foundAbsolutePath = QFileInfo(configPath).absoluteFilePath();

                // Check if this matches our target config path
                if (foundAbsolutePath == absolutePath) {
                    return pid;
                }
            }
        }
    }

    return QString(); // Not found
}

bool ConkyManager::isConkyRunning(const QString &configPath) const
{
    return !getConkyProcess(configPath).isEmpty();
}

void ConkyManager::scanDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    // Only get first-level subdirectories (no recursion)
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);

    for (const QString &subdir : subdirs) {
        scanConkyDirectory(dir.absoluteFilePath(subdir));
    }
}

void ConkyManager::scanConkyDirectory(const QString &path)
{
    static const QStringList skipNames = {"Changelog", "Notes", "README", "README!!", "OPTIONS"};

    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    dir.setFilter(QDir::Files | QDir::Readable);
    QFileInfoList allFiles = dir.entryInfoList();

    // Build a set of existing file paths for fast lookup
    QSet<QString> existingPaths;
    for (const ConkyItem *item : std::as_const(m_conkyItems)) {
        existingPaths.insert(item->filePath());
    }

    for (const QFileInfo &fileInfo : allFiles) {
        const QString &fileName = fileInfo.fileName();
        const QString &filePath = fileInfo.absoluteFilePath();

        if (fileName.startsWith('.')) {
            continue;
        }

        if (fileInfo.isDir()) {
            continue;
        }

        // Skip known non-conky files
        bool skipFile = false;
        for (const QString &skipName : skipNames) {
            if (fileName.compare(skipName, Qt::CaseInsensitive) == 0) {
                skipFile = true;
                break;
            }
        }
        if (skipFile) {
            continue;
        }

        // Check if file is binary first
        if (isBinaryFile(filePath)) {
            continue;
        }

        // Use the same logic as conky-file-parser.sh to verify it's actually a conky file
        // This replaces extension filtering - we now rely purely on content detection
        if (!isValidConkyFile(filePath)) {
            continue;
        }

        if (existingPaths.contains(filePath)) {
            continue;
        }

        m_conkyItems.append(new ConkyItem(filePath, this));
    }
}

void ConkyManager::applyAutostartFromStartupScript()
{
    QString home = QDir::homePath();
    QString scriptPath = home + "/.conky/conky-startup.sh";
    QFile scriptFile(scriptPath);

    if (!scriptFile.exists()) {
        scriptPath = "/usr/share/mx-conky-data/conky-startup.sh";
        scriptFile.setFileName(scriptPath);
    }

    if (!scriptFile.exists() || !scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QSet<QString> autostartPaths;
    QTextStream in(&scriptFile);
    QRegularExpression pattern(QStringLiteral("^\\s*conky\\s+-c\\s+(?:\"([^\"]+)\"|(\\S+))"),
                               QRegularExpression::CaseInsensitiveOption);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch match = pattern.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        QString path = match.captured(1);
        if (path.isEmpty()) {
            path = match.captured(2);
        }

        path = path.trimmed();
        if (path.endsWith('&')) {
            path.chop(1);
            path = path.trimmed();
        }

        path.replace(QStringLiteral("${HOME}"), home);
        path.replace(QStringLiteral("$HOME"), home);
        if (path.startsWith("~/")) {
            path.replace(0, 1, home);
        }

        QFileInfo info(path);
        QString absolutePath = info.absoluteFilePath();

        if (!absolutePath.isEmpty()) {
            autostartPaths.insert(QDir::cleanPath(absolutePath));
        }
    }

    scriptFile.close();

    for (ConkyItem *item : m_conkyItems) {
        QString itemPath = QDir::cleanPath(QFileInfo(item->filePath()).absoluteFilePath());
        bool enabled = autostartPaths.contains(itemPath);
        item->setEnabled(enabled);
        item->setAutostart(enabled);
    }
}

void ConkyManager::updateStartupScript()
{
    QString home = QDir::homePath();
    QString conkyDir = home + "/.conky";
    QString startupScript = conkyDir + "/conky-startup.sh";

    // Ensure ~/.conky directory exists
    QDir dir;
    if (!dir.exists(conkyDir)) {
        dir.mkpath(conkyDir);
    }

    QString scriptContent;
    scriptContent += "#!/bin/sh\n\n";
    scriptContent += QString("sleep %1s\n").arg(m_startupDelay);
    scriptContent += "killall -u $(id -nu) conky 2>/dev/null\n";

    // Add conky start commands
    for (ConkyItem *item : m_conkyItems) {
        if (item->isAutostart()) {
            QString relativeDir = QFileInfo(item->filePath()).absolutePath().replace(home, "$HOME");
            QString relativePath = item->filePath().replace(home, "$HOME");

            scriptContent += QString("cd \"%1\"\n").arg(relativeDir);
            scriptContent += QString("conky -c \"%1\" &\n").arg(relativePath);
        }
    }

    scriptContent += "exit 0\n";

    // Write the script
    QFile file(startupScript);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << scriptContent;
        file.close();

        // Make script executable
        QFile::setPermissions(startupScript, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup
                                                 | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther);
    }
}

void ConkyManager::setAutostart(bool enabled)
{
    QString home = QDir::homePath();

    // Check desktop environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString desktop = env.value("XDG_CURRENT_DESKTOP", "").toLower();

    // Lambda to create desktop file content
    auto createDesktopContent = []() {
        QString startupScript = "/usr/share/mx-conky-data/conky-startup.sh";
        QString content;
        content += "[Desktop Entry]\n";
        content += "Type=Application\n";
        content += QString("Exec=sh \"%1\"\n").arg(startupScript);
        content += "Hidden=false\n";
        content += "NoDisplay=false\n";
        content += "X-GNOME-Autostart-enabled=true\n";
        content += "Name=Conky\n";
        return content;
    };

    // Lambda to write content to a file
    auto writeToFile = [](const QString &filePath, const QString &content) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return true;
        }
        return false;
    };

    if (desktop.contains("enlightenment") || desktop.contains("e17") || desktop.contains("e18")
        || desktop.contains("e19") || desktop.contains("e20") || desktop.contains("e21") || desktop.contains("e22")
        || desktop.contains("e23") || desktop.contains("e24") || desktop.contains("e25") || desktop.contains("e26")) {
        // Enlightenment-specific handling
        QString applicationsDir = home + "/.local/share/applications";
        QString desktopFile = applicationsDir + "/conky.desktop";
        QString e17AutostartDir = home + "/.e/e/applications/startup";
        QString e17DesktopFile = e17AutostartDir + "/.order";

        QDir dir;
        // Ensure applications directory exists
        if (!dir.exists(applicationsDir)) {
            dir.mkpath(applicationsDir);
        }
        // Ensure E17 autostart directory exists
        if (!dir.exists(e17AutostartDir)) {
            dir.mkpath(e17AutostartDir);
        }

        if (enabled) {
            // Create and write desktop file
            writeToFile(desktopFile, createDesktopContent());

            // Update .order file with desktop filename only
            QStringList orderEntries;
            QFile orderFile(e17DesktopFile);
            if (orderFile.exists() && orderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&orderFile);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (!line.isEmpty() && line != "conky.desktop") {
                        orderEntries.append(line);
                    }
                }
                orderFile.close();
            }

            // Add conky.desktop if not already present
            orderEntries.append("conky.desktop");

            // Write updated .order file
            if (orderFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&orderFile);
                for (const QString &entry : orderEntries) {
                    out << entry << "\n";
                }
                orderFile.close();
            }
        } else {
            // Remove desktop file and from .order file
            QFile::remove(desktopFile);

            // Update .order file to remove conky.desktop
            QStringList orderEntries;
            QFile orderFile(e17DesktopFile);
            if (orderFile.exists() && orderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&orderFile);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (!line.isEmpty() && line != "conky.desktop") {
                        orderEntries.append(line);
                    }
                }
                orderFile.close();

                // Write updated .order file
                if (orderFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&orderFile);
                    for (const QString &entry : orderEntries) {
                        out << entry << "\n";
                    }
                    orderFile.close();
                }
            }
        }
    } else {
        // Standard XDG autostart handling for other desktop environments
        QString autostartDir = home + "/.config/autostart";
        QString desktopFile = autostartDir + "/conky.desktop";

        // Ensure autostart directory exists
        QDir dir;
        if (!dir.exists(autostartDir)) {
            dir.mkpath(autostartDir);
        }

        if (enabled) {
            // Create and write desktop file
            writeToFile(desktopFile, createDesktopContent());
        } else {
            QFile::remove(desktopFile);
        }
    }
}

bool ConkyManager::isAutostartEnabled() const
{
    QString home = QDir::homePath();

    // Check desktop environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString desktop = env.value("XDG_CURRENT_DESKTOP", "").toLower();

    if (desktop.contains("enlightenment") || desktop.contains("e17") || desktop.contains("e18")
        || desktop.contains("e19") || desktop.contains("e20") || desktop.contains("e21") || desktop.contains("e22")
        || desktop.contains("e23") || desktop.contains("e24") || desktop.contains("e25") || desktop.contains("e26")) {
        // Check Enlightenment autostart
        QString applicationsDir = home + "/.local/share/applications";
        QString desktopFile = applicationsDir + "/conky.desktop";
        QString e17AutostartDir = home + "/.e/e/applications/startup";
        QString e17DesktopFile = e17AutostartDir + "/.order";

        if (!QFile::exists(desktopFile)) {
            return false;
        }

        // Read the desktop file and check if Hidden=true
        QFile file(desktopFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("Hidden=", Qt::CaseInsensitive)) {
                QString value = line.mid(7).trimmed(); // Remove "Hidden=" prefix
                if (value.compare("true", Qt::CaseInsensitive) == 0) {
                    return false; // Autostart is disabled
                }
            }
        }
        file.close();

        // Check if conky.desktop is listed in .order file
        QFile orderFile(e17DesktopFile);
        if (!orderFile.exists() || !orderFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream orderIn(&orderFile);
        while (!orderIn.atEnd()) {
            QString line = orderIn.readLine().trimmed();
            if (line == "conky.desktop") {
                return true;
            }
        }

        return false; // Not found in .order file
    } else {
        // Standard XDG autostart check
        QString desktopFile = home + "/.config/autostart/conky.desktop";

        if (!QFile::exists(desktopFile)) {
            return false;
        }

        // Read the desktop file and check if Hidden=true
        QFile file(desktopFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("Hidden=", Qt::CaseInsensitive)) {
                QString value = line.mid(7).trimmed(); // Remove "Hidden=" prefix
                if (value.compare("true", Qt::CaseInsensitive) == 0) {
                    return false; // Autostart is disabled
                }
            }
        }

        return true; // File exists and Hidden is not true
    }
}

QString ConkyManager::copyFolderToUserConky(const QString &sourcePath)
{
    QFileInfo sourceInfo(sourcePath);
    QString folderName = sourceInfo.fileName();
    return copyFolderToUserConkyWithName(sourcePath, folderName);
}

QString ConkyManager::copyFolderToUserConkyWithName(const QString &sourcePath, const QString &newName)
{
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isDir()) {
        qDebug() << "ConkyManager: Source path does not exist or is not a directory:" << sourcePath;
        return QString();
    }

    QString userConkyPath = QDir::homePath() + "/.conky";
    QString destPath = userConkyPath + "/" + newName;

    // Create ~/.conky directory if it doesn't exist
    QDir().mkpath(userConkyPath);

    // Remove existing destination if it exists
    if (QFileInfo::exists(destPath)) {
        QDir(destPath).removeRecursively();
    }

    // Copy the folder
    if (copyDirectoryRecursively(sourcePath, destPath)) {
        qDebug() << "ConkyManager: Successfully copied" << sourcePath << "to" << destPath;
        return destPath;
    } else {
        qDebug() << "ConkyManager: Failed to copy" << sourcePath << "to" << destPath;
        return QString();
    }
}

bool ConkyManager::copyDirectoryRecursively(const QString &sourceDir, const QString &destDir)
{
    QDir sourceDirectory(sourceDir);
    if (!sourceDirectory.exists()) {
        return false;
    }

    QDir destDirectory(destDir);
    if (!destDirectory.exists()) {
        destDirectory.mkpath(".");
    }

    // Copy all files
    QFileInfoList fileInfoList = sourceDirectory.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : fileInfoList) {
        QString srcFilePath = fileInfo.absoluteFilePath();
        QString destFilePath = destDir + "/" + fileInfo.fileName();

        if (!QFile::copy(srcFilePath, destFilePath)) {
            qDebug() << "ConkyManager: Failed to copy file" << srcFilePath << "to" << destFilePath;
            return false;
        }
    }

    // Copy all subdirectories recursively
    QFileInfoList subdirInfoList = sourceDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &subdirInfo : subdirInfoList) {
        QString srcSubdirPath = subdirInfo.absoluteFilePath();
        QString destSubdirPath = destDir + "/" + subdirInfo.fileName();

        if (!copyDirectoryRecursively(srcSubdirPath, destSubdirPath)) {
            return false;
        }
    }

    return true;
}

bool ConkyManager::isBinaryFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return true;
    }

    // Read first 8KB to check for binary content
    const int bufferSize = 8192;
    QByteArray buffer = file.read(bufferSize);
    file.close();

    if (buffer.isEmpty()) {
        return true;
    }

    // Check for null bytes which indicate binary content
    if (buffer.contains('\0')) {
        return true;
    }

    // Check for high percentage of non-printable characters
    int nonPrintableCount = 0;
    for (char c : buffer) {
        unsigned char uc = static_cast<unsigned char>(c);
        // Only count actual control characters as non-printable
        // Allow common whitespace characters and all high-ASCII (UTF-8) characters
        if (uc < 32 && uc != '\t' && uc != '\n' && uc != '\r') {
            nonPrintableCount++;
        }
    }

    // If more than 30% non-printable control characters, consider it binary
    return (nonPrintableCount * 100 / buffer.size()) > 30;
}

bool ConkyManager::isValidConkyFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString content = file.readAll();
    file.close();

    if (content.isEmpty()) {
        return false;
    }

    // Check for new-style conky format (conky.config and conky.text)
    QRegularExpression configPattern(R"(^\s*conky\.config\s*=\s*\{)", QRegularExpression::MultilineOption);
    bool hasConfig = configPattern.match(content).hasMatch();

    QRegularExpression textPattern(R"(^\s*conky\.text\s*=\s*\[\[)", QRegularExpression::MultilineOption);
    bool hasText = textPattern.match(content).hasMatch();

    // New-style conky: both patterns must be present
    if (hasConfig && hasText) {
        return true;
    }

    // Check for old-style conky format (TEXT section without conky.config)
    QRegularExpression oldTextPattern(R"(^\s*TEXT\s*$)", QRegularExpression::MultilineOption);
    bool hasOldText = oldTextPattern.match(content).hasMatch();

    QRegularExpression newConfigPattern(R"(^\s*conky\.config\s*=)", QRegularExpression::MultilineOption);
    bool hasNewConfig = newConfigPattern.match(content).hasMatch();

    // Old-style conky: has TEXT section but no conky.config
    return hasOldText && !hasNewConfig;
}

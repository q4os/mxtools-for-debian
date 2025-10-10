/**********************************************************************
 *  ConkyManager.h
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
#pragma once

#include "cmd.h"
#include "conkyitem.h"
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QTimer>

class ConkyManager : public QObject
{
    Q_OBJECT

public:
    explicit ConkyManager(QObject *parent = nullptr);
    ~ConkyManager();

    void addSearchPath(const QString &path);
    void removeSearchPath(const QString &path);
    QStringList searchPaths() const;

    void scanForConkies();
    void addConkyItemsFromDirectory(const QString &directoryPath);
    QList<ConkyItem *> conkyItems() const;

    void startConky(ConkyItem *item);
    void stopConky(ConkyItem *item);
    void removeConkyItem(ConkyItem *item);
    QString copyFolderToUserConky(const QString &sourcePath);
    QString copyFolderToUserConkyWithName(const QString &sourcePath, const QString &newName);

    void startAutostart();
    void stopAllRunning();

    // Autostart management
    void updateStartupScript();
    void setAutostart(bool enabled);
    bool isAutostartEnabled() const;

    // Global startup delay
    int startupDelay() const
    {
        return m_startupDelay;
    }
    void setStartupDelay(int delay)
    {
        m_startupDelay = delay;
    }

    void saveSettings();
    void loadSettings();

signals:
    void conkyItemsChanged();
    void conkyStarted(ConkyItem *item);
    void conkyStopped(ConkyItem *item);

private slots:
    void updateRunningStatus();
    void onStatusProcessFinished();
    void onAutostartTimer();

private:
    QList<ConkyItem *> m_autostartQueue;
    QList<ConkyItem *> m_conkyItems;
    QProcess *m_statusProcess;
    QSettings m_settings;
    QStringList m_searchPaths;
    QTimer *m_autostartTimer;
    QTimer *m_statusTimer;
    bool m_statusCheckRunning;
    int m_startupDelay;
    mutable Cmd m_cmd; // Reusable Cmd object for process checking

    QString getConkyProcess(const QString &configPath) const;
    bool isConkyRunning(const QString &configPath) const;
    void clearConkyItems();
    void scanConkyDirectory(const QString &path);
    void scanDirectory(const QString &path);
    void applyAutostartFromStartupScript();
    bool copyDirectoryRecursively(const QString &sourceDir, const QString &destDir);
    bool isBinaryFile(const QString &filePath) const;
    bool isValidConkyFile(const QString &filePath) const;
};

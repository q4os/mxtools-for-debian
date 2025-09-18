/**********************************************************************
 *  ConkyItem.cpp
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

#include "conkyitem.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

ConkyItem::ConkyItem(QObject *parent)
    : QObject(parent)
{
}

ConkyItem::ConkyItem(const QString &filePath, QObject *parent)
    : QObject(parent),
      m_filePath(filePath)
{
    updateFromFile();
}

QString ConkyItem::filePath() const
{
    return m_filePath;
}

void ConkyItem::setFilePath(const QString &path)
{
    if (m_filePath != path) {
        m_filePath = path;
        updateFromFile();
        emit dataChanged();
    }
}

QString ConkyItem::name() const
{
    return m_name;
}

void ConkyItem::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit dataChanged();
    }
}

QString ConkyItem::description() const
{
    return m_description;
}

void ConkyItem::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        emit dataChanged();
    }
}

QString ConkyItem::previewImage() const
{
    // Lazy load preview image only when requested
    if (m_previewImage.isEmpty() && !m_filePath.isEmpty()) {
        // Cast away constness to allow lazy loading
        ConkyItem *that = const_cast<ConkyItem *>(this);
        that->m_previewImage = that->findPreviewImage();
    }
    return m_previewImage;
}

void ConkyItem::setPreviewImage(const QString &imagePath)
{
    if (m_previewImage != imagePath) {
        m_previewImage = imagePath;
        emit dataChanged();
    }
}

bool ConkyItem::isEnabled() const
{
    return m_enabled;
}

void ConkyItem::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit dataChanged();
    }
}

bool ConkyItem::isAutostart() const
{
    return m_autostart;
}

void ConkyItem::setAutostart(bool autostart)
{
    if (m_autostart != autostart) {
        m_autostart = autostart;
        emit dataChanged();
    }
}

int ConkyItem::autostartDelay() const
{
    return m_autostartDelay;
}

void ConkyItem::setAutostartDelay(int seconds)
{
    if (m_autostartDelay != seconds) {
        m_autostartDelay = seconds;
        emit dataChanged();
    }
}

bool ConkyItem::isRunning() const
{
    return m_running;
}

void ConkyItem::setRunning(bool running)
{
    if (m_running != running) {
        m_running = running;
        emit dataChanged();
    }
}

QString ConkyItem::directory() const
{
    return QFileInfo(m_filePath).absolutePath();
}

void ConkyItem::updateFromFile()
{
    if (m_filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) {
        return;
    }

    if (m_name.isEmpty()) {
        // Include parent directory name in the display name
        QString parentDirName = fileInfo.absoluteDir().dirName();
        QString fileName = fileInfo.completeBaseName();
        m_name = QString("%1: %2").arg(parentDirName, fileName);
    }

    // Don't search for preview image during initial scan - defer until user selects this conky
    m_previewImage.clear();
}

QString ConkyItem::findPreviewImage() const
{
    if (m_filePath.isEmpty()) {
        return {};
    }

    QFileInfo fileInfo(m_filePath);
    QDir dir = fileInfo.absoluteDir();

    QStringList imageExtensions = {"png", "jpg", "jpeg", "gif", "bmp"};
    QString baseName = fileInfo.completeBaseName();

    for (const QString &ext : imageExtensions) {
        QString imagePath = dir.absoluteFilePath(baseName + "." + ext);
        if (QFile::exists(imagePath)) {
            return imagePath;
        }
    }

    for (const QString &ext : imageExtensions) {
        QString imagePath = dir.absoluteFilePath("preview." + ext);
        if (QFile::exists(imagePath)) {
            return imagePath;
        }
    }

    return {};
}

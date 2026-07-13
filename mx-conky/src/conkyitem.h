/**********************************************************************
 *  ConkyItem.h
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

#include <QObject>
#include <QString>
#include <QStringList>

class ConkyItem : public QObject
{
    Q_OBJECT

public:
    explicit ConkyItem(QObject *parent = nullptr);
    explicit ConkyItem(const QString &filePath, QObject *parent = nullptr);

    [[nodiscard]] QString filePath() const;
    void setFilePath(const QString &path);

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] QString description() const;
    void setDescription(const QString &description);

    [[nodiscard]] QString previewImage() const;
    void setPreviewImage(const QString &imagePath);

    [[nodiscard]] bool isEnabled() const;
    void setEnabled(bool enabled);

    [[nodiscard]] bool isAutostart() const;
    void setAutostart(bool autostart);

    [[nodiscard]] int autostartDelay() const;
    void setAutostartDelay(int seconds);

    [[nodiscard]] bool isRunning() const;
    void setRunning(bool running);

    [[nodiscard]] QString directory() const;

signals:
    void dataChanged();

private:
    QString m_description;
    QString m_filePath;
    QString m_name;
    mutable QString m_previewImage;
    bool m_autostart = false;
    bool m_enabled = false;
    bool m_running = false;
    int m_autostartDelay = 0;

    void updateFromFile();
    [[nodiscard]] QString findPreviewImage() const;
};

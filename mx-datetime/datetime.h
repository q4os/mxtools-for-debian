/**********************************************************************
 *  MX Date/Time application.
 **********************************************************************
 *   Copyright (C) 2019 by AK-47
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#ifndef DATETIME_H
#define DATETIME_H

#include <QByteArray>
#include <QDateTime>
#include <QDialog>
#include <QString>
#include <QTableWidgetItem>
#include <QTimeEdit>
#include <QTimer>

// QTimeEdit subclassing just to stop the cursor and selection jumping every second.
class MTimeEdit : public QTimeEdit
{
    Q_OBJECT
public:
    explicit MTimeEdit(QWidget *parent = nullptr)
        : QTimeEdit(parent)
    {
    }
    void updateDateTime(const QDateTime &dateTime);
};

// This #include must come after the MTimeEdit class.
#include "ui_datetime.h"

class MXDateTime : public QDialog, private Ui::MXDateTime
{
    Q_OBJECT

public:
    explicit MXDateTime(QWidget *parent = nullptr);

private:
    QTimer updater;
    bool clockLock = false;
    bool changedServers = false;
    unsigned int loadedTabs = 0;
    int dateDelta = 0;
    qint64 timeDelta = 0;
    int zoneDelta = 0;
    bool zoneIdChanged = false;
    enum InitSystem { SystemV, OpenRC, SystemD } sysInit = SystemV;
    QList<QByteArray> zones;
    bool enabledNTP {};
    bool isHardwareUTC {};
    bool updating = false;

    void startup();
    void setClockLock(bool locked);
    bool shell(const QString &cmd, QByteArray *output = nullptr, bool elevate = false);
    bool execute(const QString &program, const QStringList &arguments = QStringList(), QByteArray *output = nullptr,
                 QByteArray *error = nullptr, bool elevate = false);
    bool executeAsRoot(const QString &program, const QStringList &arguments = QStringList(),
                       QByteArray *output = nullptr, QByteArray *error = nullptr);
    void loadTab(int index);
    void update();
    void loadDateTime();
    void saveDateTime(const QDateTime &driftStart);
    void transferTime(const QStringList &params, const QString &from, const QString &to);
    void readHardwareClock();
    void saveHardwareClock();
    QTableWidgetItem *addServerRow(bool enabled, const QString &type, const QString &address, const QString &options);
    void moveServerRow(int movement);
    bool validateServerList();
    void loadNetworkTime();
    void saveNetworkTime();
    bool loadSources(const QString &filename);
    bool clearSources(const QString &filename);

    // Slots

    void timeAreaIndexChanged(int index);
    void timeZoneIndexChanged(int index);
    void calendarSelectionChanged();
    void timeEditDateTimeChanged(const QDateTime &dateTime);

    void hardwareAdjustClicked();
    void systemToHardwareClicked();
    void hardwareToSystemClicked();

    void syncNowClicked();
    void serversItemSelectionChanged();
    void serverAddClicked();
    void serverRemoveClicked();
    void serverRowChanged();

    void aboutClicked();
    static void helpClicked();
    void applyClicked();
};

#endif // DATETIME_H

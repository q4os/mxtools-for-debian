/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2024 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QButtonGroup>
#include <QCommandLineParser>
#include <QFile>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

#include <cmd.h>

namespace Ui
{
class MainWindow;
}

namespace ButtonID
{
enum {
    Lang,
    Address,
    Collate,
    CType,
    Identification,
    Measurement,
    Messages,
    Monetary,
    Name,
    Numeric,
    Paper,
    Telephone,
    Time
};
}

namespace Tab
{
enum { Language, Subvariables, Management };
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &args, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void aboutClicked();
    void helpClicked();
    void listItemChanged(QListWidgetItem *item);
    void onFilterChanged(const QString &text);
    void onGroupButton(int button_id);
    void resetSubvariables();
    void tabWidgetCurrentChanged();
    void textSearch_textChanged();

private:
    Ui::MainWindow *ui;
    Cmd *cmd;
    QButtonGroup *buttonGroup;
    QHash<QString, QString> hashLocale;
    bool localeGenChanged {false};
    int countEnabled {};

    [[nodiscard]] QString getCurrentLang() const;
    [[nodiscard]] QString getCurrentSessionLang() const;
    [[nodiscard]] QString getLocaleDescription(const QString &locale) const;
    [[nodiscard]] bool anyDifferentSubvars() const;
    void disableGUI(bool disable);
    void disableAllButCurrent();
    void displayLocalesGen();
    void localeGen();
    void readLocaleFile(QFile &file, const QStringList &enabledLocale);
    void resetLocaleGen();
    void setButtons();
    void setConnections();
    void setSubvariables();
    void setup();
};

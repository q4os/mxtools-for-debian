/**********************************************************************
 *  mxconky.h
 **********************************************************************
 * Copyright (C) 2017 MX Authors
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

#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTimer>

#include "cmd.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &file = QLatin1String(""));
    ~MainWindow() override;

    void parseContent();
    void detectConkyFormat();
    void pickColor(QWidget *widget);
    void refresh();
    void saveBackup();
    void setColor(QWidget *widget, const QColor &color);
    void writeColor(QWidget *widget, const QColor &color);
    void writeFile(QFile file, const QString &content);

    bool checkConkyRunning();
    bool readFile(const QString &file_name);

    static QColor strToColor(const QString &colorstr);

public slots:

private slots:
    static void on_pushHelp_clicked();
    void cleanup();
    void closeEvent(QCloseEvent *event) override;
    void on_pushAbout_clicked();
    void on_pushCM_clicked();
    void on_pushChange_clicked();
    void on_pushColor0_clicked();
    void on_pushColor1_clicked();
    void on_pushColor2_clicked();
    void on_pushColor3_clicked();
    void on_pushColor4_clicked();
    void on_pushColor5_clicked();
    void on_pushColor6_clicked();
    void on_pushColor7_clicked();
    void on_pushColor8_clicked();
    void on_pushColor9_clicked();
    void on_pushDefaultColor_clicked();
    void on_pushEdit_clicked();
    void on_pushRestore_clicked();
    void on_pushToggleOn_clicked();
    void on_radioAllDesktops_clicked();
    void on_radioDayLong_clicked();
    void on_radioDayShort_clicked();
    void on_radioDesktop1_clicked();
    void on_radioMonthLong_clicked();
    void on_radioMonthShort_clicked();

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QTimer *timer {};
    QSettings settings;
    QString file_content;
    QString file_name;
    bool modified {};

    bool is_lua_format {};
    bool conky_format_detected = false;
    bool debug = false;

    // Regexp pattern
    QString capture_lua_color;
    QString capture_old_color;
    QString lua_comment_end;
    QString lua_comment_line;
    QString lua_comment_start;
    QString lua_config;
    QString lua_format;
    QString old_comment_line;
    QString old_format;

    QString capture_lua_owh = QStringLiteral("^(?<before>(?:.*\\]\\])?\\s*(?<item>own_window_hints)(?:\\s*=\\s*[\\\"\\'"
                                             "]))(?<value>[[:alnum:],_]*)(?<after>(?:[\\\"\\']\\s*,).*)");
    QString capture_old_owh = QStringLiteral(
        "^(?<before>(?:.*\\]\\])?\\s*(?<item>own_window_hints)(?:\\s+))(?<value>[[:alnum:],_]*)(?<after>.*)");

    QString block_comment_start = QStringLiteral("--[[");
    QString block_comment_end = QStringLiteral("]]");
    QRegularExpression regexp_lua_color(QString);
};

/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle, AK-47, Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of Quick System Info.
 *
 * mx-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCommandLineParser>
#include <QDialog>
#include <QAction>
#include <QEvent>
#include <QTextDocument>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &arg_parser, QWidget *parent = nullptr) noexcept;
    ~MainWindow();

    int run(const char *program, const QStringList &args = QStringList(),
        QByteArray *output = nullptr, const QByteArray *input = nullptr) noexcept;
    inline int shell(const QString &cmd,
        QByteArray *output = nullptr, const QByteArray *input = nullptr) noexcept
    {
        return run("/bin/bash", {"-c", cmd}, output, input);
    }

    void setup() noexcept;

private slots:
    void on_pushSave_clicked() noexcept;
    void on_pushSaveText_clicked() noexcept;
    void on_buttonAbout_clicked() noexcept;
    void on_ButtonHelp_clicked() noexcept;
    void on_listInfo_itemSelectionChanged() noexcept;
    void on_listInfo_itemChanged() noexcept;

    void on_tabWidget_currentChanged(int index);

    void on_comboBoxJournaldListBoots_activated(int index);

    void on_comboBoxJournaldPriority_activated(int index);

    void on_comboBoxJournaldSystemUser_activated(int index);

    void on_toolButtonReloadSearch_clicked();

private:
    Ui::MainWindow *ui;
    QStringList defaultMatches;
    QAction *actionSave = nullptr;
    QString searchText;
    bool journald_setup_done = false;
    QTextDocument::FindFlags searchFlags;
    void lockGUI(bool lock) noexcept;
    void forumcopy() noexcept;
    void plaincopy() noexcept;
    void showSavedMessage(const QString &filename, const QString &errmsg) noexcept;
    QByteArray readReport(int row) noexcept(false);
    void buildInfoList() noexcept;
    void listSelectAll() noexcept;
    void listSelectDefault() noexcept;
    void showFindDialog() noexcept;
    void findNext() noexcept;
    bool eventFilter(QObject *watched, QEvent *event) noexcept;
    void autoFitSplitter() noexcept;
    void systemd_check();
    void journald_setup();
    void run_journalctl_report();
};

#endif // MAINWINDOW_H

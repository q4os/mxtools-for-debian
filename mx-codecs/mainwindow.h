/*****************************************************************************
 * mxcodecs.h
 *****************************************************************************
 * Copyright (C) 2014 MX Authors
 *
 * Authors: Jerry 3904
 *          Anticaptilista
 *          Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Codecs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Codecs.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryDir>

#include "cmd.h"
#include "lockfile.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool arch_flag = true;
    bool i386_flag = true;

    bool isOnline();
    void installDebs(const QString &path);
    void updateStatus(const QString &msg, int val);

    QString downloadDebs();

public slots:
    void buttonHelp_clicked();
    void buttonAbout_clicked();
    void buttonOk_clicked();

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    LockFile lock_file;
    QString arch;
    QTemporaryDir tempdir;

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    bool downloadDeb(const QString &url, const QString &filepath);
    bool downloadFile(const QString &url, QFile &file);
    bool downloadInfoAndPackage(const QString &url, const QString &release, const QString &repo, const QString &arch,
                                QFile &file, const QStringList &search_terms, int progress);
};

#endif // MAINWINDOW_H

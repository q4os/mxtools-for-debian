/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QTimer>
#include <QWidget>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QLabel;

class Crontab;
class TCommand;

class TCommandEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TCommandEdit(QWidget *parent = nullptr);

public slots:
    void changeCurrent(Crontab *cron, TCommand *cmnd);

private slots:
    void commandEdited(const QString &str);
    void timeEdited(const QString &str);
    void userChanged(int index);
    void commentEdited();
    void resetExeTime();
    void doTimeDialog();

signals:
    void dataChanged();

private:
    void setExecuteList(const QString &time);
    QLineEdit *timeEdit;
    QComboBox *userCombo;
    QLineEdit *commandEdit;
    QTextEdit *commentEdit;
    QLabel *exeLabel;
    QLabel *userLabel;
    bool viewChanging;

    TCommand *tCommand {};
    QTimer timer;
};

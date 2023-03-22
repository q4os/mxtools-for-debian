/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <QDialog>
#include <QPushButton>

class QButtonGroup;
class QLineEdit;
class QCheckBox;

class TimeButton : public QPushButton
{
    Q_OBJECT
public:
    TimeButton(const QString &label, QWidget *parent = nullptr);
    TimeButton(int label, QWidget *parent = nullptr);
    void init();

public slots:
    void buttonToggled(bool chk);
};

class TimeDialog : public QDialog
{
    Q_OBJECT
public:
    TimeDialog(QString time, QWidget *parent = nullptr);
    QString time() { return outTime; }

private slots:
    void minuteButtonClicked(QAbstractButton *b);
    void hourButtonClicked(QAbstractButton *b);
    void dayButtonClicked(QAbstractButton *b);
    void monthButtonClicked(QAbstractButton *b);
    void weekButtonClicked(QAbstractButton *b);
    void simpleButtonClicked(QAbstractButton *b);
    void litCheckBoxChanged(int state);
    void resetClicked();

private:
    void initButtons(const QString &time);

    QString inTime;
    QString outTime;
    QButtonGroup *minuteBGroup;
    QButtonGroup *hourBGroup;
    QButtonGroup *dayBGroup;
    QButtonGroup *monthBGroup;
    QButtonGroup *weekBGroup;
    QButtonGroup *simpleBGroup;
    QCheckBox *litCheckBox;

    bool useLiteral;

    QLineEdit *timeEdit;
};

#endif

/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QWidget>

class QVBoxLayout;
class QTextEdit;
class QRadioButton;
class QComboBox;
class QLineEdit;
class VariableView;
class Crontab;
class TCommand;
class Variable;
class VariableModel;

class VariableEdit : public QWidget
{
    Q_OBJECT
public:
    explicit VariableEdit(QWidget *parent = nullptr);

public slots:
    void changeCurrent(Crontab *cron, TCommand *cmnd);
    void varViewSelected(Variable *var);

private slots:
    void commentChanged();
    void varEdited(const QString &str);
    void valEdited(const QString &str);
    void varCommentChanged();
    void deleteClicked();
    void newClicked();
    void mailOnClicked(bool state);
    void mailOffClicked(bool state);
    void mailToClicked(bool state);
    void userActivated(int index);

signals:
    void dataChanged();

private:
    void setMailVar(int flag);
    void setMailCombo(const QList<Variable *> &var);

    QVBoxLayout *varInputLayout {};
    QTextEdit *commentEdit;
    QRadioButton *mailOffRadio;
    QRadioButton *mailOnRadio;
    QRadioButton *mailToRadio;
    QComboBox *userCombo;
    VariableView *variableView;
    QLineEdit *nameEdit;
    QLineEdit *valueEdit;
    QTextEdit *varCommentEdit;
    bool viewChanging;
    bool varViewChanging;

    Crontab *crontab {};
    Variable *variable {};
    VariableModel *variableModel;
};

/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QDialog>
#include <QString>
#include <QTextEdit>

class QTextEdit;

class SaveDialog : public QDialog
{
    Q_OBJECT

public:
    SaveDialog(const QString &user, const QString &text);
    QString getText() { return cronText->toPlainText(); }

private:
    QTextEdit *cronText;
};

#endif

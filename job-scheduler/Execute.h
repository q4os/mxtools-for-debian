/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef EXECUTE_H
#define EXECUTE_H

#include <QString>

class TCommand;

class Execute
{
public:
    Execute(TCommand *cd, const QString &dt, int fl = 0, int sl = 0)
        : tCommands(cd)
        , exeTime(dt)
        , flag(fl)
        , sel(sl)
    {
    }

    ~Execute() { }

    TCommand *tCommands;
    QString exeTime;
    int flag; // 0:Normal, -1:Time format error,
    int sel;  // 1:Selecting(Cron) 2:Selecting(Command)
};
#endif

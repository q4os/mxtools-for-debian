/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QString>
#include <utility>

class TCommand;

class Execute
{
public:
    Execute(TCommand *cd, QString dt, int fl = 0, int sl = 0)
        : tCommands(cd),
          exeTime(std::move(dt)),
          flag(fl),
          sel(sl)
    {
    }

    ~Execute() = default;

    TCommand *tCommands = nullptr;
    QString exeTime{};
    int flag = 0; // 0:Normal, -1:Time format error,
    int sel = 0;  // 1:Selecting(Cron) 2:Selecting(Command)
};

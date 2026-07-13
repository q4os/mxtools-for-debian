/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QDir>
#include <QStringList>

#include <pwd.h>
#include <unistd.h>

class Clib
{
public:
    inline static uid_t uId()
    {
        return getuid();
    }
    inline static QString uName()
    {
        struct passwd *pw = getpwuid(uId());
        return pw ? QString(pw->pw_name) : QString::number(uId());
    }
    inline static QString uHome()
    {
        struct passwd *pw = getpwuid(uId());
        return pw ? QString(pw->pw_dir) : QDir::homePath();
    }
    inline static QString uShell()
    {
        struct passwd *pw = getpwuid(uId());
        return pw ? QString(pw->pw_shell) : QStringLiteral("/bin/sh");
    }
    inline static QString getEnv(const char *name)
    {
        return qEnvironmentVariable(name);
    }
    inline static QStringList allUsers()
    {
        QStringList ulist;
        struct passwd *pw = nullptr;
        while ((pw = getpwent()) != nullptr) {
            ulist << pw->pw_name;
        }
        endpwent();
        return ulist;
    }
};

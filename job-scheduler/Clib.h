/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef CLIB_H
#define CLIB_H

#include <QStringList>

#include <pwd.h>
#include <unistd.h>

class Clib
{
public:
    inline static uid_t uId() { return getuid(); }
    inline static QString uName() { return getpwuid(uId())->pw_name; }
    inline static QString uHome() { return getpwuid(uId())->pw_dir; }
    inline static QString uShell() { return getpwuid(uId())->pw_shell; }
    inline static QString getEnv(const char *name) { return qEnvironmentVariable(name); }
    inline static QStringList allUsers() {
        QStringList ulist;
        struct passwd *pw;
        while (( pw = getpwent()) != nullptr ) ulist << pw->pw_name;
        endpwent();
        return ulist;
    }
};

#endif

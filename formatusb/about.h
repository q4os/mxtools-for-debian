#pragma once

#include <QString>
#include <cmd.h>

void displayDoc(QString url, QString title, bool runned_as_root = false);
void displayAboutMsgBox(QString title, QString message, QString licence_url, QString license_title,
                        bool runned_as_root = false);

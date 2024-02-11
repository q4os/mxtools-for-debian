#pragma once

#include <QString>
#include <cmd.h>

void displayDoc(const QString &url, const QString &title, bool runned_as_root = false);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title, bool runned_as_root = false);

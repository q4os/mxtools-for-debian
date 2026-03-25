#pragma once

class QString;

void displayDoc(const QString &fileName, const QString &title, bool largeWindow = false);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title);

#pragma once

class QString;
class QWidget;

void displayDoc(const QString &url, const QString &title);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title, QWidget *parent = nullptr);

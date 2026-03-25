#pragma once

class QString;
class QWidget;

QString docPath(const QString &fileName);
void displayDoc(QWidget *parent, const QString &path, const QString &title, bool largeWindow = false);
void displayAboutMsgBox(QWidget *parent, const QString &title, const QString &message,
                        const QString &licensePath, const QString &licenseTitle);

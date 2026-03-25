#pragma once

class QWidget;
class QString;

void displayDoc(QWidget *parent, const QString &source, const QString &title, bool largeWindow = false);
void displayAboutMsgBox(QWidget *parent, const QString &title, const QString &message, const QString &licenceSource,
                        const QString &licenseTitle);

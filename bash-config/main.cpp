#include "global.h"
#include "window.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QApplication::setOrganizationName("MX-Linux");
	QApplication::setOrganizationDomain("MX-Linux");
	QApplication::setApplicationName(NAME_BIN);
	QApplication::setApplicationVersion("1.0.0");

	QTranslator qtTran;
	qtTran.load(QStringLiteral("bash-config_") + QLocale::system().name(), "/usr/share/bash-config/locale");
	a.installTranslator(&qtTran);

	/*
    QTranslator prgmTran;
    prgmTran.load(QString(NAME_BIN) + QLocale::system().name(), "filepath"); //CHANGE PATH IF USING
    a.installTranslator(prgmTran);
    */

	Window w;
	w.show();

	return a.exec();
}

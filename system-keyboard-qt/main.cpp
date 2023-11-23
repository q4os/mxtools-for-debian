#include "window.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("system-keyboard-qt_") + QLocale::system().name(), "/usr/share/system-keyboard-qt/locale");
    a.installTranslator(&appTran);


    Window w;
    w.show();

    return a.exec();


}

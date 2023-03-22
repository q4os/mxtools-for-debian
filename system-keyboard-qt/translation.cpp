
#include "translation.h"

QString keyboardtr(QString input)
{
    return QString::fromStdString(dgettext("xkeyboard-config", input.toStdString().c_str()));
}

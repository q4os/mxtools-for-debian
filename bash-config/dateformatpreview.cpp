#include "dateformatpreview.h"

#include <ctime>

DateFormatPreview::DateFormatPreview(QWidget *parent) : QLabel(parent), m_dateFormat()
{
    updateDisplayText();
}

QString DateFormatPreview::dateFormat() const
{
    return m_dateFormat;
}

void DateFormatPreview::setDateFormat(QString dateString)
{
    m_dateFormat = dateString;
    updateDisplayText();
}

void DateFormatPreview::updateDisplayText()
{
    if(m_dateFormat.isEmpty())
    {
        setText(" ");
        return;
    }
    struct tm* timeinfo;
    time_t rawtime;
    char buffer[100];
    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    size_t length = std::strftime(buffer, 100, m_dateFormat.toUtf8().data(), timeinfo);
    std::string str{buffer, length};
    setText(QString::fromStdString(str));
}

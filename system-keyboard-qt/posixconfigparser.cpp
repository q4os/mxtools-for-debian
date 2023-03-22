#include "posixconfigparser.h"

PosixConfigParser::PosixConfigParser(QString input)
    : m_input(input)
{
    parse();
}

void PosixConfigParser::set(QString key, QString value)
{
    QStringList lines = m_input.split('\n');
    bool complete = false;
    for(int i = 0; i < lines.size(); i++)
    {
        QString line = lines[i];
        if(line.indexOf('#') == 0) continue;
        int e = line.indexOf('=');
        if(e < 0) continue;
        QString k = line.mid(0, e);
        if(key == k)
        {
            lines[i] = QString("%1=\"%2\"").arg(key).arg(value);
            complete = true;
            break;
        }
    }
    if(!complete)
    {
        lines.append(QString("%1=\"%2\"").arg(key).arg(value));
    }
    m_input = lines.join('\n');
    parse();
}

void PosixConfigParser::parse()
{
    config.clear();
    QStringList lines = m_input.split('\n');
    for(auto line : lines)
    {
        if(line.indexOf('#') == 0) continue;
        int e = line.indexOf('=');
        if(e < 0) continue;
        QString key = line.mid(0, e);
        QString value = line.mid(e + 1);
        if(value.indexOf('"') == 0 || value.indexOf('\'') == 0)
        {
            value = value.mid(1);
            value = value.mid(0, value.length() - 1);
        }
        config[key] = value;
    }
}

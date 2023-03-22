#ifndef POSIXCONFIGPARSER_H
#define POSIXCONFIGPARSER_H

#include <QString>
#include <QMap>

class PosixConfigParser
{
public:
    PosixConfigParser(QString input);
    QMap<QString, QString> config;
    void set(QString key, QString value);
    QString source() const { return m_input; }
private:
    void parse();
    QString m_input;
};

#endif // POSIXCONFIGPARSER_H

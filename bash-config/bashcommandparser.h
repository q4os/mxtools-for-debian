#pragma once

#include "fuzzybashstream.h"

class BashCommandParser
{
public:
	BashCommandParser(const QString& command);
	QString command() const { return m_command; }
	QStringList rawArgs() const { return m_args; }

    bool hasOption(const QString& option);
    QString optionValue(const QString& option);
private:
	QString m_command;
	QStringList m_args;
};

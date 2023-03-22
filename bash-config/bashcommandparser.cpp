#include "bashcommandparser.h"

BashCommandParser::BashCommandParser(QString command)
{
	FuzzyBashStream stream{command};
    auto tokens = stream.tokens();
    if(tokens.empty())
    {
        m_command = QString();
        m_args = QStringList();
    }
}

bool BashCommandParser::hasOption(QString option)
{
    return m_args.contains(option);
}

QString BashCommandParser::optionValue(QString option)
{
    if(!hasOption(option)) return {};
    auto pos = option.indexOf(option);
    if((pos + 1) == option.length()) return {};
    return m_args.at(pos + 1);
}

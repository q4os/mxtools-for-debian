#include "bashcommandparser.h"

BashCommandParser::BashCommandParser(const QString& command)
{
	FuzzyBashStream stream{command};
    auto tokens = stream.tokens();
    if(tokens.empty())
    {
        m_command = QString();
        m_args = QStringList();
    }
}

bool BashCommandParser::hasOption(const QString& option)
{
    return m_args.contains(option);
}

QString BashCommandParser::optionValue(const QString& option)
{
    if(!hasOption(option)) return {};
    auto pos = option.indexOf(option);
    if((pos + 1) == option.length()) return {};
    return m_args.at(pos + 1);
}

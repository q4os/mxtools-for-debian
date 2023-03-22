#ifndef ALIASSTREAM_H
#define ALIASSTREAM_H

#include "fuzzybashstream.h"
#include <QString>

class AliasStream;
class AliasTab;

struct AliasTokenData
{
    bool standalone = false;
    FuzzyBashStream::TokenRef aliasCommandToken;
    FuzzyBashStream::TokenRef spaceToken;
};

Q_DECLARE_METATYPE(AliasTokenData);

enum AliasTokenType
{
    TokenAliasDefinition = FuzzyBashStream::TokenAliasReserved,
    TokenAliasCommand
};

void aliasGrouper(FuzzyBashStream* stream, QList<FuzzyBashStream::Token>& tokens);

struct ParsedAlias
{
    QString name;
    QString command;
    bool good = false;
};

struct ParsedAliasTokens
{
    FuzzyBashStream::TokenRef name, command, equal;
    bool good = false;
};

ParsedAlias parseAlias(QString input); // input should be like: ls='ls -l' not: alias ls='ls -l'
ParsedAliasTokens parseAliasTokens(QList<FuzzyBashStream::TokenRef> refs);

class Alias // Is only good while FuzzyBashStream* in TokenRef is good
{
  public:
    Alias() {}
    Alias(QString name, QString command, FuzzyBashStream::TokenRef token = {})
        : m_name(name), m_command(command), m_token(token) {}
    QString name() const { return m_name; }
    QString command() const { return m_command; }
    void setName(QString name) { m_name = name; }
    void setCommand(QString command) { m_command = command; }
    FuzzyBashStream::TokenRef token() const { return m_token; }
    bool operator==(const Alias& other)
    {
        return m_name == other.m_name && m_command == other.m_command;
    }
    bool operator!=(const Alias& other)
    {
        return !(*this == other);
    }
  protected:
    QString m_name, m_command;
    FuzzyBashStream::TokenRef m_token;
};

class AliasStream
{
  public:
	AliasStream(QString* str = nullptr, bool isOfBashrc = false);
    AliasStream& operator<<(const Alias& alias);		  //DEPRECATED
    AliasStream& operator<<(const QList<Alias>& aliases); //DEPRECATED
    AliasStream& operator>>(QList<Alias>& aliases);		  //DEPRECATED
    QList<Alias> get();
	void set(const Alias& alias);
	void set(const QList<Alias>& alias);
	void remove(const Alias& alias);
    void remove(const QList<Alias>& alias);
    bool contains(const Alias& alias);
    bool containsNamed(const QString& name);
	QString* source() const;
	void setSource(QString* source);
	bool getIsOfBashrc() const;
    void setIsOfBashrc(bool isOfBashrc);

    QList<Alias> aliasList(const QList<FuzzyBashStream::TokenRef>& refs, bool validRefs = true);

  protected:
	QString* m_source;
	bool m_isOfBashrc;
};

#endif // ALIASSTREAM_H

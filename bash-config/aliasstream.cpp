#include "aliasstream.h"

#include "global.h"
#include <QRegularExpression>
#include <algorithm>

AliasStream::AliasStream(QString* str, bool isOfBashrc)
{
	SCOPE_TRACKER;
	m_source = str;
	m_isOfBashrc = isOfBashrc;
}

AliasStream& AliasStream::operator<<(const Alias& alias)
{
    Q_ASSERT(false);
//	SCOPE_TRACKER;
//	if (m_source == nullptr)
//		return *this;
//	QString templateAliasRegex("(alias) (%1)=(\"|')(%2)(\\3)");
//	QString templateAliasText("alias %1='%2'");
//	QList<Alias> aliases;
//	*this >> aliases;
//	bool resolved = false;
//	for (Alias a : aliases)
//	{
//		if (a.alias() != alias.alias())
//			continue;
//		if (a.command() == alias.command())
//		{
//			resolved = true;
//			break;
//		}
//		m_source->replace(QRegularExpression(templateAliasRegex.arg(a.alias()).arg(a.command())), templateAliasText.arg(alias.alias()).arg(alias.command()));
//		resolved = true;
//		break;
//	}
//	if (!resolved)
//	{
//		m_source->append(QObject::tr("\n") + templateAliasText.arg(alias.alias()).arg(alias.command()));
//	}
    return *this;
}

Q_DECL_DEPRECATED
AliasStream& AliasStream::operator<<(const QList<Alias>& aliases)
{
    Q_ASSERT(false);
//	SCOPE_TRACKER;
//	if (m_source == nullptr)
//		return *this;
//	for (Alias alias : aliases)
//	{
//		*this << alias;
//	}
    return *this;
}

Q_DECL_DEPRECATED
AliasStream& AliasStream::operator>>(QList<Alias>& aliases)
{
    Q_ASSERT(false);
//	SCOPE_TRACKER;
//	if (m_source == nullptr)
//		return *this;
//	QList<Alias> rtn;
//	QRegularExpression regex("(alias) ([\\w-]+)=(\"|')([^\\n]{0,})(\\3)");
//	auto iter = regex.globalMatch(*m_source);
//	while (iter.hasNext())
//	{
//		auto match = iter.next();
//		Alias alias(match.captured(2), match.captured(4));
//		alias.setStart(match.capturedStart());
//		alias.setEnd(match.capturedEnd());
//		alias.setLength(match.capturedLength());
//		alias.setInBashrc(m_isOfBashrc);
//		/*
//        if(match.captured(3).contains('"'))
//            alias.surrondedInDoubleQuotes = true;
//        else
//            alias.surrondedInDoubleQuotes = false;
//         */
//		rtn.append(alias);
//	}
//	aliases = rtn;
	return *this;
}

QList<Alias> AliasStream::get()
{
	SCOPE_TRACKER;
	if (m_source == nullptr)
        return QList<Alias>{};
    QList<Alias> rtn;
    FuzzyBashStream stream{*m_source, FuzzyBashStream::ParseNormal, {aliasGrouper}};
    return aliasList(stream.tokenRefs(), false);
}

void AliasStream::set(const Alias& alias)
{
	SCOPE_TRACKER;
    FuzzyBashStream parser{*m_source, FuzzyBashStream::ParseNormal, {aliasGrouper}};
    auto tokens = parser.tokenRefs();
    auto aliases = aliasList(tokens);
    if(contains(alias))
        return;
    auto iter = std::find_if(aliases.begin(), aliases.end(), [alias](const Alias& a){
        return a.name() == alias.name();
    });
    if(iter == aliases.end())
    {
        (*m_source).append(QString("\nalias %1='%2'").arg(alias.name(), alias.command()));
    }
    else
    {
        FuzzyBashStream stream{(*iter).token().content(), FuzzyBashStream::ParseDisableCommandGrouping};
        auto pat = parseAliasTokens(stream.tokenRefs());
        if(!pat.good) // this shouldn't happen
        {
            Q_ASSERT(false);
        }
        pat.command.setQuoted(alias.command());
        *m_source = parser.source();
    }
}

void AliasStream::set(const QList<Alias>& alias)
{
    SCOPE_TRACKER;
    for(auto a : alias)
    {
        set(a);
    }
}

void AliasStream::remove(const Alias& alias)
{
//	SCOPE_TRACKER;
//	QString templateAliasRegex("(alias) (%1)=(\"|')(%2)(\\3)");
//	QString templateAliasText("alias %1='%2'");
//#define E(i) escapeRegexCharacters(i)
//	if (m_source->contains(QRegularExpression(templateAliasRegex.arg(E(alias.alias())).arg(E(alias.command())))))
//	{
//		m_source->remove(QRegularExpression(templateAliasRegex.arg(E(alias.alias())).arg(E(alias.command()))));
//		DEBUG << "Did detect and remove alias" << alias.alias();
//	}
//#undef E
    if(!contains(alias)) return;
    FuzzyBashStream parser{*m_source, FuzzyBashStream::ParseNormal, {aliasGrouper}};
    auto tokens = parser.tokenRefs();
    auto aliases = aliasList(tokens);
    auto iter = std::find(aliases.begin(), aliases.end(), alias);
    Q_ASSERT(iter != aliases.end());
    auto a = *iter;
    auto t = a.token();
    AliasTokenData data = t.data().value<AliasTokenData>();
    if(data.standalone)
        data.aliasCommandToken.setContent("");
    *m_source = parser.source();
}

void AliasStream::remove(const QList<Alias>& alias)
{
	SCOPE_TRACKER;
	for (auto a : alias)
        remove(a);
}

bool AliasStream::contains(const Alias &alias)
{
    Q_ASSERT(m_source);
    FuzzyBashStream stream{*m_source, FuzzyBashStream::ParseNormal, {aliasGrouper}};
    auto aliases = aliasList(stream.tokenRefs());
    return std::any_of(aliases.begin(), aliases.end(), [alias](const Alias& a){
        return a.name() == alias.name() && a.command() == alias.command();
    });
}

bool AliasStream::containsNamed(const QString &name)
{
    Q_ASSERT(m_source != nullptr);
    FuzzyBashStream stream{*m_source, FuzzyBashStream::ParseNormal, {aliasGrouper}};
    auto aliases = aliasList(stream.tokenRefs());
    return std::any_of(aliases.begin(), aliases.end(), [name](const Alias& a){
        return a.name() == name;
    });
}

QString* AliasStream::source() const
{
	SCOPE_TRACKER;
	return m_source;
}

void AliasStream::setSource(QString* source)
{
	SCOPE_TRACKER;
	m_source = source;
}

bool AliasStream::getIsOfBashrc() const
{
	SCOPE_TRACKER;
	return m_isOfBashrc;
}

void AliasStream::setIsOfBashrc(bool isOfBashrc)
{
	SCOPE_TRACKER;
    m_isOfBashrc = isOfBashrc;
}

QList<Alias> AliasStream::aliasList(const QList<FuzzyBashStream::TokenRef> &refs, bool validRef)
{
    QList<Alias> rtn;
    for(auto tr : refs)
    {
        if(tr.type() == AliasTokenType::TokenAliasDefinition)
        {
            auto pa = parseAlias(tr.content());
            if(!pa.good) continue;
            auto t = tr;
            if(!validRef)
                t = t.downgrade().upgrade(nullptr); // make a TokenRef without a valid stream causing it to assert failure when you try to modifiy
            rtn.append(Alias(pa.name, pa.command, t));
        }
    }
    return rtn;
}

void aliasGrouper(FuzzyBashStream* stream, QList<FuzzyBashStream::Token> &tokens)
{
    QList<FuzzyBashStream::Token> result;
    while(tokens.size() > 0)
    {
        FuzzyBashStream::Token token = tokens.first();
        tokens.pop_front();
        if(token.type() == FuzzyBashStream::TokenCommand && token.content().indexOf("alias") == 0)
        {
            auto split = FuzzyBashStream(token.content(), FuzzyBashStream::ParseDisableCommandGrouping, {}, token.start()).tokens();
            if(split.isEmpty() || split.first().content() != "alias" || split.first().type() != FuzzyBashStream::TokenWord)
            {
                goto normal;
            }
            auto aliasToken = FuzzyBashStream::Token::join({split.first()}, TokenAliasCommand); // Set type of token to TokenAliasCommand
            result.append(aliasToken);
            auto aliasTokenRef = aliasToken.upgrade(stream);
            split.pop_front(); // remove 'alias'
            int count = 0;
            while(!split.isEmpty())
            {
                if(split.isEmpty() || split.first().type() != FuzzyBashStream::TokenWhitespace)
                {
                    qDebug() << "Failed at finding whitespace: " << ((split.isEmpty()) ? "No Token Found" : split.first().content());
                    break;
                }
                auto spaceToken = split.first().upgrade(stream);
                result.append(split.first());
                split.pop_front(); // remove whitespace
                if(split.isEmpty() || split.first().type() != FuzzyBashStream::TokenWord)
                {
                    qDebug() << "Failed at finding word: " << ((split.isEmpty()) ? "No Token Found" : split.first().content());
                    break;
                }
                auto aliasNameToken = split.first();
                split.pop_front();
                if(split.isEmpty() || split.first().content() != "=")
                {
                    qDebug() << "Failed at finding equal sign: " << ((split.isEmpty()) ? "No Token Found" : split.first().content());
                    break;
                }
                auto aliasEqualToken = split.first();
                split.pop_front();
                if(split.isEmpty() || (split.first().type() != FuzzyBashStream::TokenWord && split.first().type() != FuzzyBashStream::TokenQuoted))
                {
                    qDebug() << "Failed at finding word or quoted: " << ((split.isEmpty()) ? "No Token Found" : split.first().content());
                    break;
                }
                auto aliasCommandToken = split.first();
                split.pop_front();
                AliasTokenData data;
                data.aliasCommandToken = aliasTokenRef;
                data.spaceToken = spaceToken;
                data.standalone = false;
                auto aliasDefAllToken = FuzzyBashStream::Token::join({aliasNameToken, aliasEqualToken, aliasCommandToken}, TokenAliasDefinition);
                result.append(aliasDefAllToken);
                count++;
            }
            if(count == 1) // if there was only one alias set standalone to true
            {
                AliasTokenData oldData = result.last().data().value<AliasTokenData>();
                oldData.standalone = true;
                QVariant newData;
                newData.setValue(oldData);
                result.last().setData(newData);
            }
        }
        else
        {
            normal:
            result.append(token);
        }
    }
    tokens = result;
}

ParsedAlias parseAlias(QString input)
{
    auto tokens = parseAliasTokens(FuzzyBashStream(input, FuzzyBashStream::ParseDisableCommandGrouping).tokenRefs());
    // WARNING tokens contains invalid references to stream, can't modify
    ParsedAlias pa;
    pa.good = tokens.good;
    pa.name = tokens.name.content();
    if(tokens.command.type() == FuzzyBashStream::TokenQuoted)
        pa.command = tokens.command.quoted();
    else
        pa.command = tokens.command.content();
    return pa;
}

ParsedAliasTokens parseAliasTokens(QList<FuzzyBashStream::TokenRef> refs)
{
    auto tokens{std::move(refs)};
    ParsedAliasTokens result;
    if(tokens.isEmpty() || tokens.first().type() != FuzzyBashStream::TokenWord)
    {
        return {};
    }
    result.name = tokens.first();
    tokens.pop_front();
    if(tokens.isEmpty() || tokens.first().content() != "=")
    {
        return {};
    }
    tokens.pop_front();
    if(tokens.isEmpty() || (tokens.first().type() != FuzzyBashStream::TokenQuoted && tokens.first().type() != FuzzyBashStream::TokenWord))
    {
        return {};
    }
    result.command = tokens.first();
    tokens.pop_front();
    result.good = true;
    return result;
}

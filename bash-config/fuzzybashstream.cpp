#include "fuzzybashstream.h"

FuzzyBashStream::PrivateToken FuzzyBashStream::joinTokens(FuzzyBashStream::TokenList tokens, int type)
{
	if (tokens.size() == 0)
		return PrivateToken();
	int start = tokens.first().start;
	int end = tokens.last().end;
	QString totalContent;
	for (auto token : tokens)
	{
		totalContent += token.content;
	}
	PrivateToken result;
	result.start = start;
	result.end = end;
	result.content = totalContent;
	result.tokenType = type;
	return result;
}

FuzzyBashStream::PrivateToken FuzzyBashStream::parseToken(FuzzyBashStream::InputStream& stream, int& mode)
{
	QList<QChar> acceptableForWord = {'@', '$', '%', '^', '*', '-', '+', '_', ':', '.', ',', '?', '~', '/'};
	if (mode & JustParsedWord)
	{
		acceptableForWord.append('=');
		acceptableForWord.append('!');
		acceptableForWord.append('{');
		acceptableForWord.append('}');
	}
	if (stream.eof())
	{
		return PrivateToken();
	}
	int start = stream.pos(), end = -1;
	TokenType type;
	QString content;
	if (stream.peek() == '\n' || stream.peek() == ';')
	{
		content += stream.next();
		type = TokenEOL;
	}
	else if (stream.peek() == '=' && mode & InsideSquareBrackets)
	{
		content += stream.next();
		if (!stream.eof() && stream.peek() == '=')
		{
			content += stream.next();
			type = TokenEqualSign;
		}
		else if (!stream.eof() && stream.peek() == '~')
		{
			content += stream.next();
			type = TokenRegexMatchSign;
		}
		else
			type = TokenEqualSign;
	}
	else if (stream.peek() == '!' && mode & InsideSquareBrackets)
	{
		content += stream.next();
		if (!stream.eof() && stream.peek() == '=')
		{
			content += stream.next();
			type = TokenNotEqualSign;
		}
		else
		{
			type = TokenUnknown;
		}
	}
	else if (stream.peek() == '&')
	{
		content += stream.next();
		if (!stream.eof() && stream.peek() == '&')
		{
			content += stream.next();
			type = TokenAnd;
		}
		else
		{
			type = TokenFork;
		}
	}
	else if (stream.peek() == '|')
	{
		content += stream.next();
		if (!stream.eof() && stream.peek() == '|')
		{
			content += stream.next();
			type = TokenOr;
		}
		else
		{
			type = TokenPipe;
		}
	}
	else if (stream.peek() == '{')
	{
		content += stream.next();
		type = TokenOpeningCurlyBracket;
	}
	else if (stream.peek() == '}')
	{
		content += stream.next();
		type = TokenClosingCurlyBracket;
	}
	else if (stream.peek() == '(')
	{
		content += stream.next();
		type = TokenOpeningParentheses;
	}
	else if (stream.peek() == ')')
	{
		content += stream.next();
		type = TokenClosingParentheses;
	}
	else if (stream.peek() == '[')
	{
		content += stream.next();
		type = TokenOpeningSquareBracket;
		mode = InsideSquareBrackets;
	}
	else if (stream.peek() == ']')
	{
		content += stream.next();
		type = TokenClosingSquareBracket;
		mode = Normal;
	}
	else if (stream.peek() == '#')
	{
		content += stream.next();
		while (!stream.eof() && stream.peek() != '\n')
			content += stream.next();
		type = TokenComment;
	}
	else if (stream.peek() == '"') // QUOTED STRING ex: "hello"
	{
		content += stream.next();
		while (!stream.eof() && !(stream.peek() == '"' && stream.prev() != '\\'))
			content += stream.next();
		if (!stream.eof()) // in case it is unterminated
			content += stream.next();
		type = TokenQuoted;
	}
	else if (stream.peek() == '\'') // QUOTED STRING ex: 'hello'
	{
		content += stream.next();
		while (!stream.eof() && !(stream.peek() == '\'' && stream.prev() != '\\'))
			content += stream.next();
		if (!stream.eof()) // in case it is unterminated
			content += stream.next();
		type = TokenQuoted;
	}
	else if (stream.peek() == '`') // QUOTED STRING ex: `hello`
	{
		content += stream.next();
		while (!stream.eof() && !(stream.peek() == '`' && stream.prev() != '\\'))
			content += stream.next();
		if (!stream.eof()) // in case it is unterminated
			content += stream.next();
		type = TokenQuoted;
	}
	else if (stream.peek().isLetterOrNumber() || acceptableForWord.contains(stream.peek()))
	{
		while (!stream.eof() && (stream.peek().isLetterOrNumber() || acceptableForWord.contains(stream.peek())))
			content += stream.next();
		type = TokenWord;
	}
	else if (stream.peek().isSpace()) // WHITESPACE ex: SPACE TAB
	{
		while (!stream.eof() && stream.peek().isSpace())
			content += stream.next();
		type = TokenWhitespace;
	}
	else
	{
		// Just move past invalid input
		stream.next();
		type = TokenUnknown;
	}
	if (type == TokenWord)
	{
		mode |= JustParsedWord;
	}
	else
	{
		mode &= ~JustParsedWord;
	}
	end = stream.pos();
	PrivateToken token;
	token.start = start;
	token.end = end;
	token.tokenType = type;
	token.content = content;
	return token;
}

FuzzyBashStream::TokenList FuzzyBashStream::tokenize(FuzzyBashStream::InputStream& stream)
{
	PrivateToken token;
	int mode = Normal;
	TokenList tokens;
	do
	{
		token = parseToken(stream, mode);
		if (token.isValid())
			tokens += token;
	} while (token.isValid());
	return tokens;
}

FuzzyBashStream::TokenList FuzzyBashStream::tokenize2(FuzzyBashStream::TokenList tokens)
{
	QStringList reservedKeywords = {"!", "case", "coproc", "do", "done", "elif", "else", "esac", "fi", "for", "function", "if", "in", "select", "then", "until", "while", "{", "}", "time", "[", "]"};
	TokenList result;
	while (tokens.size() > 0)
	{
		PrivateToken token = tokens.first();
		tokens.pop_front();
		if (!reservedKeywords.contains(token.content) && token.tokenType == TokenWord && tokens.size() > 0)
		{
			TokenList collected = {token};
			while (tokens.size() > 0)
			{
				PrivateToken token = tokens.first();
				if (token.tokenType == TokenWhitespace && tokens.size() > 1)
				{
					int i = 1;
					bool somethingAhead = false;
					while (i < tokens.size())
					{
						PrivateToken next = tokens.at(i);
						if (next.tokenType == TokenWord || next.tokenType == TokenQuoted)
						{
							somethingAhead = true;
							break;
						}
						else if (next.tokenType != TokenWhitespace) // if it isn't TokenWord || TokenQuoted || TokenWhitespace
						{
							break;
						}
					}
					if (somethingAhead)
					{
						collected << token;
						tokens.pop_front();
					}
					else
					{
						break;
					}
				}
				else if (token.tokenType == TokenWord || token.tokenType == TokenQuoted)
				{
					collected << token;
					tokens.pop_front();
				}
				else
				{
					break;
				}
			}
			token = joinTokens(collected, TokenCommand);
			result << token;
		}
		else if (token.tokenType == TokenOpeningSquareBracket && tokens.size() > 0)
		{
			TokenList collected = {token};
			PrivateToken token = tokens.first();
			while (token.tokenType != TokenClosingSquareBracket && tokens.size() > 0)
			{
				collected << tokens.first();
				tokens.pop_front();
				token = tokens.first();
			}
			token = joinTokens(collected, TokenBooleanExpression);
			result << token;
		}
		else
		{
			result << token;
		}
	}
	return result;
}

FuzzyBashStream::Token FuzzyBashStream::Token::join(const QList<FuzzyBashStream::Token>& tokens, int type)
{
	TokenList pts;
	for (auto t : tokens)
	{
		pts.append(t.m_token);
	}
	auto pt = joinTokens(pts, type);
	return Token(pt);
}

FuzzyBashStream::Token& FuzzyBashStream::Token::add(const FuzzyBashStream::Token& t, int type)
{
	PrivateToken pt = joinTokens({m_token, t.m_token}, type);
	m_token = pt;
    return *this;
}

FuzzyBashStream::Token &FuzzyBashStream::Token::offset(int offset)
{
    m_token.start += offset;
    m_token.end += offset;
    return *this;
}

FuzzyBashStream::Token::Token(const FuzzyBashStream::PrivateToken& t)
	: m_token(t)
{
}

FuzzyBashStream::TokenRef FuzzyBashStream::Token::upgrade(FuzzyBashStream* stream) const
{
    return TokenRef(m_token, stream);
}

FuzzyBashStream::Token::Token()
{
    m_token = PrivateToken();
}

FuzzyBashStream::TokenRef::TokenRef()
    : m_stream(nullptr)
{
}

FuzzyBashStream::TokenRef::TokenRef(const FuzzyBashStream::Token &token, FuzzyBashStream* stream)
    : Token(token), m_stream(stream)
{
}

FuzzyBashStream::TokenRef::TokenRef(const FuzzyBashStream::PrivateToken& t, FuzzyBashStream* stream)
	: Token(t), m_stream(stream)
{
}

void FuzzyBashStream::TokenRef::commit()
{
    Q_ASSERT(m_stream != nullptr);
	m_stream->commitTokenRef(*this);
}

FuzzyBashStream::Token FuzzyBashStream::TokenRef::downgrade() const
{
	return static_cast<Token>(*this);
}

FuzzyBashStream::FuzzyBashStream(QString source, ParsingOptions options, QList<TokenGrouper> groupers, int offset)
    : m_source(source), m_offset(0), m_parsingFlags(options)
{
    reparse(options, groupers, offset);
}

FuzzyBashStream& FuzzyBashStream::reparse(ParsingOptions options, QList<FuzzyBashStream::TokenGrouper> groupers, int offset)
{
	InputStream stream{m_source};
	TokenList tokens = tokenize(stream);
	if (!(options & ParseDisableCommandGrouping))
		tokens = tokenize2(tokens);
	m_tokens.clear();
	for (auto t : tokens)
	{
		m_tokens.append(Token(t));
    }
    for(auto& g : groupers)
    {
        g(this, m_tokens);
    }
    for(auto& t : m_tokens)
    {
        t.offset(offset);
    }
	return *this;
}

QList<FuzzyBashStream::TokenRef> FuzzyBashStream::tokenRefs()
{
	QList<TokenRef> refs;
	for (const Token& t : m_tokens)
	{
        refs << t.upgrade(ref());
	}
	return refs;
}

QString FuzzyBashStream::unquote(const Token& t)
{
	if (t.type() != TokenQuoted)
		return t.content();
	QString input = t.content();
	if (input.isEmpty())
		return input;
	input.remove(0, 1);
	input.remove(input.size() - 1, 1);
	return input;
}

void FuzzyBashStream::commitTokenRef(const FuzzyBashStream::TokenRef& tr)
{
	int offset = (tr.m_token.end - tr.m_token.start) - tr.m_token.content.length();
	m_source.replace(tr.m_token.start + m_offset, (tr.m_token.end + m_offset) + (tr.m_token.start + m_offset), tr.m_token.content);
	m_offset += offset;
}

QDebug operator<<(QDebug debug, const FuzzyBashStream::TokenRef& tr)
{
	QDebugStateSaver _{debug};
	debug.nospace() << "TokenRef(" << tr.content() << ")";
	return debug;
}

QDebug operator<<(QDebug debug, const FuzzyBashStream::Token& tr)
{
	QDebugStateSaver _{debug};
	debug.nospace() << "Token(" << tr.content() << ")";
    return debug;
}

void variableAssignmentGrouper(FuzzyBashStream *stream, QList<FuzzyBashStream::Token> &tokens)
{
}

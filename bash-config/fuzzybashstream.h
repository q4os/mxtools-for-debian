#ifndef FUZZYBASHSTREAM_H
#define FUZZYBASHSTREAM_H

#include <QDebug>
#include <QList>
#include <QString>
#include <memory>

class FuzzyBashStream
{
  public:
	enum TokenType
	{
		TokenUnknown = -1,
		TokenEOL = 0,
		TokenQuoted,
		TokenWord,
		TokenWhitespace,
		TokenOpeningSquareBracket,
		TokenClosingSquareBracket,
		TokenOpeningCurlyBracket,
		TokenClosingCurlyBracket,
		TokenOpeningParentheses,
		TokenClosingParentheses,
		TokenEqualSign,
		TokenNotEqualSign,
		TokenRegexMatchSign,
		TokenOr,
		TokenPipe,
		TokenAnd,
		TokenFork,
		TokenComment,

		// TOKENIZEv2 token types

		TokenCommand,
		TokenBooleanExpression,

        TokenUserType, // So users can declare Token Types

        TokenAliasReserved = TokenUserType + 100, // Reserve TokenType slots for Alias grouping
        TokenVariableReserved = TokenUserType + 200,
	};

	enum ParsingOptions
	{
		ParseNormal = 0,
        ParseDisableCommandGrouping = (1 << 1),
        ParseDisableAliasGrouping = (1 << 2),
	};

  private:
	struct PrivateToken
	{
		int tokenType = TokenUnknown;
		int start = -1, end = -1;
		QString content;
		bool isValid() const { return start >= 0 && end >= 0; }
	};
	using TokenList = QList<PrivateToken>;

  public:
	class TokenRef;

	class Token
	{
	  public:
        QString content() const { return m_token.content; }
        QString quoted() const
        {
            QString t = m_token.content;
            t.remove(0, 1);
            t.remove(t.length() - 1, 1);
            return t;
        }
        int type() const { return m_token.tokenType; }
		static Token join(const QList<Token>& tokens, int type);
        Token& add(const Token& t, int type);
        Token& offset(int offset);
        int start() const { return m_token.start; }
        int end() const { return m_token.end; }
        void setData(QVariant data) { m_data = data; }
        QVariant data() const { return m_data; }
        TokenRef upgrade(FuzzyBashStream* stream) const;

        Token();

      protected:
        QVariant m_data;
		Token(const PrivateToken& t);
		PrivateToken m_token;

	  private:
		friend class FuzzyBashStream;
    };

    class TokenRef : public Token
	{
	  public:
		void setContent(const QString& content)
        {
            Q_ASSERT(m_stream != nullptr);
            m_token.content = content;
            commit();
        }
        void setQuoted(const QString& content)
        {
            Q_ASSERT(m_stream != nullptr);
            QChar quoted = m_token.content.at(0);
            m_token.content = quoted + content + quoted;
            commit();
        }

        TokenRef();
        TokenRef(const Token& token, FuzzyBashStream* stream);
        Token downgrade() const;
      private:
        TokenRef(const PrivateToken& t, FuzzyBashStream* stream);
		void commit();
        FuzzyBashStream* m_stream;
		friend class FuzzyBashStream;
    };

//    class TokenGrouper
//    {
//    public:
//        virtual ~TokenGrouper();
//        virtual QList<Token> group(FuzzyBashStream* stream, QList<Token> tokens) = 0;
//    };

    using TokenGrouper = std::function<void(FuzzyBashStream*, QList<Token>&)>;

    FuzzyBashStream(QString source, ParsingOptions options = ParseNormal, QList<TokenGrouper> groupers = {}, int offset = 0);
	QString source() const { return m_source; }
    FuzzyBashStream& reparse(ParsingOptions options = ParseNormal, QList<TokenGrouper> groupers = {}, int offset = 0);
	QList<TokenRef> tokenRefs();
    QList<Token> tokens() const { return m_tokens; }
    FuzzyBashStream* ref() { return this; }

	static QString unquote(const Token& t);

  private:
	QList<Token> m_tokens;
	QString m_source;
	int m_offset;
	int m_parsingFlags;

	void commitTokenRef(const TokenRef& tr);

	enum ParsingMode
	{
		Normal = 0,
		InsideSquareBrackets = (1 << 1),
		JustParsedWord = (1 << 2),
	};
	class InputStream
	{
	  public:
		InputStream(QString input) : m_input(input), m_pos(0) {}
		bool eof() const { return !(m_pos < m_input.size()); }
		int pos() const { return m_pos; }
		QChar next() { return m_input[m_pos++]; }
		QChar peek(int offset = 0) const { return m_input[m_pos + offset]; }
		QChar prev(int offset = 0) const { return m_input[m_pos - (offset + 1)]; }

	  private:
		const QString m_input;
		int m_pos;
	};
	static PrivateToken joinTokens(TokenList tokenRefs, int type);
	PrivateToken parseToken(InputStream& stream, int& mode);
	TokenList tokenize(InputStream& stream);
	TokenList tokenize2(TokenList tokenRefs);
};

QDebug operator<<(QDebug debug, const FuzzyBashStream::TokenRef& tr);
QDebug operator<<(QDebug debug, const FuzzyBashStream::Token& tr);

void variableAssignmentGrouper(FuzzyBashStream* stream, QList<FuzzyBashStream::Token>& tokens);

#endif // FUZZYBASHSTREAM_H

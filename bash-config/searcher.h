#pragma once

#include <QObject>
#include <QString>

class QRegularExpression;

class Searcher : public QObject
{
	Q_OBJECT
  public:
	enum SearchStates
	{
		StateCheckDoubleQuotations = (1 << 0),
		StateCheckSingleQuotations = (1 << 1),
		StateCheckSpecialQuotations = (1 << 2),
		StateNone = 0,
	};
	enum ReturnValues
	{
		ReturnValueSearchStatesFailed = -1,
		ReturnValueSearchStringNotFound = -2,
	};

  public:
	Searcher();
	Searcher(const QString* source, const int states = SearchStates::StateNone);
	Searcher(const Searcher& copy);
	Searcher(Searcher&& move);
	~Searcher();
	Searcher& operator=(const Searcher& copy);
	Searcher& operator=(Searcher&& move);
	int search(const QString& search, int from = 0);
	int search(const QRegularExpression& search, int from = 0);
	int search(const QChar& search, int from = 0);
	Searcher& setSource(const QString* source);
	QString source() const;
	Searcher& setStates(const int states);
	SearchStates states() const;

  protected:
	template <class Type>
	int templateSearch(const Type& search, int from = 0);
	QString* m_source;
	SearchStates m_states;
};

#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>

class QRegularExpression;
class QRegExp;

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
	Searcher(Searcher& copy);
	Searcher(Searcher&& move);
	~Searcher();
	Searcher& operator=(Searcher& copy);
	Searcher& operator=(Searcher&& move);
	int search(const QString search, int from = 0);
	int search(const QRegExp search, int from = 0);
	int search(const QRegularExpression search, int from = 0);
	int search(const QChar search, int from = 0);
	Searcher& setSource(const QString* source);
	QString source();
	Searcher& setStates(const int states);
	SearchStates states();

  protected:
	template <class Type>
	int templateSearch(Type search, int from = 0);
	QString* m_source;
	SearchStates m_states;
};

#endif // SEARCHER_H

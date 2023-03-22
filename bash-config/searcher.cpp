#include "searcher.h"

#include <QRegExp>
#include <QRegularExpression>

#include "buffer.h"
#include "global.h"

Searcher::Searcher()
{
	DEBUG_ENTER(Searcher::Searcher);
	DEBUG_EXIT(Searcher::Searcher);
}

Searcher::Searcher(const QString* source, const int states)
{
	DEBUG_ENTER(Searcher::Searcher);
	setSource(source);
	setStates(static_cast<SearchStates>(states));
	DEBUG_EXIT(Searcher::Searcher);
}

Searcher::Searcher(Searcher& copy)
	: QObject(copy.parent())
{
	DEBUG_ENTER(Searcher::Searcher);
	*this = copy;
	DEBUG_EXIT(Searcher::Searcher);
}

Searcher::Searcher(Searcher&& move)
	: QObject(move.parent())
{
	DEBUG_ENTER(Searcher::Searcher);
	*this = move;
	DEBUG_ENTER(Searcher::Searcher);
}

Searcher::~Searcher()
{
	DEBUG_ENTER(Searcher::~Searcher);
	DEBUG_EXIT(Searcher::~Searcher);
}

Searcher& Searcher::operator=(Searcher& copy)
{
	DEBUG_ENTER(Searcher::operator=);
	setSource(new QString(copy.source()));
	DEBUG_EXIT(Searcher::operator=);
	return *this;
}

Searcher& Searcher::operator=(Searcher&& move)
{
	DEBUG_ENTER(Searcher::operator=);
	setSource(new QString(move.source()));
	move.setSource(new QString());
	DEBUG_EXIT(Searcher::operator=);
	return *this;
}

int Searcher::search(const QRegExp search, int from)
{
	DEBUG_ENTER(Searcher::search);
	DEBUG_EXIT(Searcher::search);
	return templateSearch(search, from);
}

int Searcher::search(const QRegularExpression search, int from)
{
	DEBUG_ENTER(Searcher::search);
	DEBUG_EXIT(Searcher::search);
	return templateSearch(search, from);
}

int Searcher::search(const QChar search, int from)
{
	DEBUG_ENTER(Searcher::search);
	DEBUG_EXIT(Searcher::search);
	return templateSearch(search, from);
}

int Searcher::search(const QString search, int from)
{
	DEBUG_ENTER(Searcher::search);
	DEBUG_EXIT(Searcher::search);
	return templateSearch(search, from);
}
Searcher& Searcher::setSource(const QString* source)
{
	DEBUG_ENTER(Searcher::setSource);
	m_source = const_cast<QString*>(source);
	DEBUG_EXIT(Searcher::setSource);
	return *this;
}

QString Searcher::source()
{
	DEBUG_ENTER(Searcher::source);
	DEBUG_EXIT(Searcher::source);
	return *m_source;
}

Searcher& Searcher::setStates(const int states)
{
	DEBUG_ENTER(Searcher::setStates);
	m_states = static_cast<Searcher::SearchStates>(states);
	DEBUG_EXIT(Searcher::setStates);
	return *this;
}

Searcher::SearchStates Searcher::states()
{
	DEBUG_ENTER(Searcher::states);
	DEBUG_EXIT(Searcher::states);
	return m_states;
}
template <class Type>
int Searcher::templateSearch(Type search, int from)
{
	DEBUG_ENTER(Searcher::templateSearch);
	if (!source().contains(search))
	{
		return Searcher::ReturnValueSearchStringNotFound;
	}
	Buffer::State *q1 = nullptr, *q2 = nullptr, *q3 = nullptr; //do not have to worry about every referencing a nullptr
	Buffer buffer = Buffer(source());
	if (states() & SearchStates::StateCheckDoubleQuotations)
	{
		q1 = new Buffer::State("\"");
		buffer.addState(q1);
	}
	if (states() & SearchStates::StateCheckSingleQuotations)
	{
		q2 = new Buffer::State("\'");
		buffer.addState(q2);
	}
	if (states() & SearchStates::StateCheckSpecialQuotations)
	{
		q3 = new Buffer::State("`");
		buffer.addState(q3);
	}
	buffer.move(source().indexOf(search, from));
	if (states() & SearchStates::StateCheckDoubleQuotations)
	{
		if (!q1->state() % 2)
			return ReturnValueSearchStatesFailed;
	}
	if (states() & SearchStates::StateCheckSingleQuotations)
	{
		if (!q2->state() % 2)
			return ReturnValueSearchStatesFailed;
	}
	if (states() & SearchStates::StateCheckSpecialQuotations)
	{
		if (!q3->state() % 2)
			return ReturnValueSearchStatesFailed;
	}
	DEBUG_EXIT(Searcher::templateSearch);
	return source().indexOf(search, from);
}

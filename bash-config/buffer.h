#pragma once

#include <QList>
#include <QObject>
#include <QVector>

class Buffer : public QObject
{
	Q_OBJECT
  public:
	class State
	{
	  public:
		State();
		State(const QString& search, int state = 1);
		State(const State& copy);
		State(State&& move);
		virtual ~State();
		State& operator=(const State& copy);
		State& operator=(State&& move);
		QString searchString() const;
		State& setSearchString(const QString& search);
		int state() const;
		State& setState(int state);

	  protected:
		QString m_search;
		int m_state;
	};
	Buffer();
	Buffer(const QString& source);
	Buffer(const Buffer& copy);
	Buffer(Buffer&& move);
	virtual ~Buffer() {}
	Buffer& operator=(const Buffer& copy);
	Buffer& operator=(Buffer&& move);
	Buffer& setSource(const QString& source);
	QString source() const;
	Buffer& addState(State* state);
	Buffer& addStates(const QList<State*>& states);
	Buffer& move(int times);
	QString buffer();

  protected:
	QString m_source;
	QString::iterator m_sourceiter;
	QVector<State*> m_states;
};

#ifndef BUFFER_H
#define BUFFER_H

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
		State(const QString search, int state = 1);
		State(State& copy);
		State(State&& move);
		virtual ~State();
		State& operator=(State& copy);
		State& operator=(State&& move);
		QString searchString();
		State& setSearchString(const QString search);
		int state();
		State& setState(int state);

	  protected:
		QString m_search;
		int m_state;
	};
	Buffer();
	Buffer(const QString source);
	Buffer(Buffer& copy);
	Buffer(Buffer&& move);
	virtual ~Buffer() {}
	Buffer& operator=(Buffer& copy);
	Buffer& operator=(Buffer&& move);
	Buffer& setSource(const QString source);
	QString source();
	Buffer& addState(State* state);
	Buffer& addStates(QList<State*> states);
	Buffer& move(int times);
	QString buffer();

  protected:
	QString m_source;
	QString::iterator m_sourceiter;
	QVector<State*> m_states;
};

#endif // BUFFER_H

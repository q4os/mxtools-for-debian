#include "buffer.h"
#include "global.h"

Buffer::Buffer()
	: QObject(nullptr)
{
	SCOPE_TRACKER;
}

Buffer::Buffer(const QString source)
	: QObject(nullptr)
{
	SCOPE_TRACKER;
	setSource(source);
}

Buffer::Buffer(Buffer& copy)
	: QObject(copy.parent())
{
	SCOPE_TRACKER;
	*this = copy;
}

Buffer::Buffer(Buffer&& move)
	: QObject(move.parent())
{
	SCOPE_TRACKER;
	*this = move;
}

Buffer& Buffer::operator=(Buffer& copy)
{
	SCOPE_TRACKER;
	if (this == &copy)
	{
		return *this;
	}
	setSource(copy.source());
	return *this;
}

Buffer& Buffer::operator=(Buffer&& move)
{
	SCOPE_TRACKER;
	if (this == &move)
	{
		return *this;
	}
	setSource(move.source());
	move.setSource(QString());
	return *this;
}

Buffer& Buffer::setSource(const QString source)
{
	m_source = source;
	return *this;
}

QString Buffer::source()
{
	SCOPE_TRACKER;
	return m_source;
}

Buffer& Buffer::addState(Buffer::State* state)
{
	SCOPE_TRACKER;
	m_states.push_back(state);
	return *this;
}

Buffer& Buffer::addStates(QList<Buffer::State*> states)
{
	SCOPE_TRACKER;
	for (State* state : states)
	{
		m_states.push_back(state);
	}
	return *this;
}

Buffer& Buffer::move(int times)
{
	SCOPE_TRACKER;
	if (times > 0)
	{
		for (int i = 0; i < times; i++)
		{
			m_sourceiter++;
			if (m_sourceiter == m_source.end())
			{
				return *this;
			}
			for (State* state : m_states)
			{
				if (buffer().indexOf(state->searchString()) == 0)
				{
				}
			}
		}
	}
	else
	{
		for (int i = 0; i > times; i--)
		{
			m_sourceiter--;
			if (m_sourceiter == m_source.end())
			{
				return *this;
			}
		}
	}
	return *this;
}

QString Buffer::buffer()
{
	SCOPE_TRACKER;
	return m_source.mid(m_sourceiter - m_source.begin());
}

Buffer::State::State()
{
	SCOPE_TRACKER;
}

Buffer::State::State(const QString search, int state)
{
	SCOPE_TRACKER;
	setSearchString(search);
	setState(state);
}

Buffer::State::State(State& copy)
{
	SCOPE_TRACKER;
	*this = copy;
}

Buffer::State::State(State&& move)
{
	SCOPE_TRACKER;
	*this = move;
}

Buffer::State::~State()
{
	SCOPE_TRACKER;
}

Buffer::State& Buffer::State::operator=(Buffer::State& copy)
{
	SCOPE_TRACKER;
	if (this == &copy)
	{
		return *this;
	}
	setSearchString(copy.searchString());
	setState(copy.state());
	return *this;
}

Buffer::State& Buffer::State::operator=(Buffer::State&& move)
{
	SCOPE_TRACKER;
	if (this == &move)
	{
		return *this;
	}
	setSearchString(move.searchString());
	move.setSearchString(QString());
	setState(move.state());
	move.setState(0);
	return *this;
}

QString Buffer::State::searchString()
{
	SCOPE_TRACKER;
	return m_search;
}

Buffer::State& Buffer::State::setSearchString(const QString search)
{
	SCOPE_TRACKER;
	m_search = search;
	return *this;
}

int Buffer::State::state()
{
	SCOPE_TRACKER;
	return m_state;
}

Buffer::State& Buffer::State::setState(int state)
{
	SCOPE_TRACKER;
	m_state = state;
	return *this;
}

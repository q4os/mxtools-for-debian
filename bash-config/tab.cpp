#include "tab.h"

Tab::Tab(const QString name, const QIcon icon, QObject* parent)
	: QObject(parent), m_widget(nullptr)
{
	SCOPE_TRACKER;
	setName(name);
	setIcon(icon);
}

Tab::Tab(Tab& copy)
	: QObject(copy.parent())
{
	SCOPE_TRACKER;
	*this = copy;
}

Tab::Tab(Tab&& move)
	: QObject(move.parent())
{
	SCOPE_TRACKER;
	*this = move;
}

Tab& Tab::operator=(Tab& copy)
{
	SCOPE_TRACKER;
	setName(copy.name());
	setIcon(copy.icon());
	return *this;
}

Tab& Tab::operator=(Tab&& move)
{
	SCOPE_TRACKER;
	setName(move.name());
	setIcon(move.icon());
	move.setIcon(QIcon());
	move.setName(QString());
	return *this;
}

Tab::~Tab()
{
	SCOPE_TRACKER;
}

QString Tab::name()
{
	SCOPE_TRACKER;
	return m_name;
}

QIcon Tab::icon()
{
	SCOPE_TRACKER;
	return m_icon;
}

Tab& Tab::setWidget(QWidget* widget)
{
	SCOPE_TRACKER;
	m_widget = widget;
	return *this;
}

QWidget* Tab::widget()
{
	SCOPE_TRACKER;
	Q_ASSUME(m_widget != nullptr);
	return m_widget;
}

Tab& Tab::setIcon(const QIcon icon)
{
	SCOPE_TRACKER;
	m_icon = icon;
	return *this;
}

Tab& Tab::setName(const QString name)
{
	SCOPE_TRACKER;
	m_name = name;
	return *this;
}

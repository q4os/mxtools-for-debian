#ifndef TAB_H
#define TAB_H

#include "global.h"
#include <QIcon>
#include <QObject>
#include <QWidget>

class Tab : public QObject
{
	Q_OBJECT
  public:
	explicit Tab(const QString name, const QIcon icon = QIcon(), QObject* parent = nullptr);
	Tab(Tab& copy);
	Tab(Tab&& move);
	Tab& operator=(Tab& copy);
	Tab& operator=(Tab&& move);
	~Tab();
	Tab& setName(const QString name);
	QString name();
	Tab& setIcon(const QIcon icon);
	QIcon icon();
	Tab& setWidget(QWidget* widget);
	QWidget* widget();

	//Virtual functions
	virtual void setup(const BashrcSource data) = 0;
	virtual BashrcSource exec(const BashrcSource data) = 0;

  private:
	QString m_name;
	QIcon m_icon;
	QWidget* m_widget;
  signals:

  public slots:
};

#endif // TAB_H

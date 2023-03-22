#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
/*
 * Name is different because of weird issue of the
 * text of buttons not showing up
 */
#include "ui_window_fix.h"

class Tab;
struct BashrcSource;

template <class Type>
class QVector;

template <class Type>
class QList;

namespace Ui
{
	class Window;
}

class Window : public QWidget
{
	Q_OBJECT

  public:
	explicit Window(QWidget* parent = nullptr);
	~Window();
	BashrcSource getSource();
	void setSource(const BashrcSource data);
	void closeEvent(QCloseEvent* event);
	void writePositionSettings();
	void readPositionSettings();
  private slots:
	void on_pushButton_Help_clicked();

  private:
	//NOTE TabManager does garbage collection on any members added
	class TabManager : public QObject
	{
	  public:
		//PRIORITY LOW
		//TODO implement copy and move contructors
		TabManager(Ui::Window* ui);
		TabManager& addTabs(QList<Tab*> tabs);
		TabManager& addTab(Tab* tab);
		TabManager& setup(const BashrcSource source);
		BashrcSource exec(const BashrcSource source);

	  protected:
		QVector<Tab*> m_tabs;
		Ui::Window* window_ui;
	};

  private:
	Ui::Window* ui;
	TabManager m_manager;
};

#endif // WINDOW_H

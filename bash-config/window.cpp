#include "window.h"

//import all tab implementations
#include "aliastab.h"
#include "othertab.h"
#include "prompttab.h"
#include "tab.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <unistd.h>

#include <QSettings>

Window::Window(QWidget* parent) : QWidget(parent),
								  ui(new Ui::Window),
								  m_manager(ui)
{
	SCOPE_TRACKER;
	ui->setupUi(this);

	readPositionSettings();

	setWindowTitle(NAME);

	m_manager.addTab(new AliasTab);
	m_manager.addTab(new PromptTab);
	m_manager.addTab(new OtherTab);
	auto data = getSource();
	m_manager.setup(getSource());

	connect(ui->pushButton_Apply, &QPushButton::clicked, [=]() {
		auto source = getSource();
		if (!source.bashrc.contains("source $HOME/.config/bash-config/bashrc.bash"))
		{
			source.bashrc.append("source $HOME/.config/bash-config/bashrc.bash");
		}
		source.program = "";
		setSource(m_manager.exec(source));
	});
	connect(ui->pushButton_Close, &QPushButton::clicked, [=]() {
		close();
	});
	connect(ui->pushButton_About, &QPushButton::clicked, [=]() {
		QMessageBox::about(this, NAME, tr("An easy way to configure your ~/.bashrc and bash prompt"));
	});
}

void Window::closeEvent(QCloseEvent* event)
{
	SCOPE_TRACKER;
	Q_UNUSED(event)
	writePositionSettings();
}

Window::~Window()
{
	SCOPE_TRACKER;
	delete ui;
}

BashrcSource Window::getSource()
{
	SCOPE_TRACKER;
	QFile bashrc(USER_BASHRC);
	if (!bashrc.open(QFile::Text | QFile::ReadOnly))
	{
		DEBUG << bashrc.fileName() + " isn't readable or exists";
		return BashrcSource();
	}
	QTextStream bashrcStream(&bashrc);
	QString bashrcSource = bashrcStream.readAll();
	bashrc.close();

	QFile program(PROGRAM_BASHRC);
	if (!program.open(QFile::Text | QFile::ReadOnly))
	{
		//TODO error
		//doesn't exist or can't access
		DEBUG << program.fileName() + " isn't readable or exists";
		BashrcSource source;
		source.bashrc = bashrcSource;
		return source;
	}
	QTextStream programStream(&program);
	QString programSource = programStream.readAll();
	program.close();

	QFile bashrcAliases(USER_BASHRC_ALIASES);
	if (!bashrcAliases.open(QFile::Text | QFile::ReadOnly))
	{
		//TODO error
		//doesn't exist or can't access
		DEBUG << bashrcAliases.fileName() + " isn't readable or exists";
		BashrcSource source;
		source.bashrc = bashrcSource;
		source.program = programSource;
		return source;
	}
	QTextStream bashrcAliasesStream(&bashrcAliases);
	QString bashrcAliasesSource = bashrcAliasesStream.readAll();
	bashrcAliases.close();

	BashrcSource data;
	data.program = programSource;
	data.bashrc = bashrcSource;
	data.bashrcAliases = bashrcAliasesSource;

	return data;
}

void Window::setSource(const BashrcSource& data)
{
	SCOPE_TRACKER;
	QFile bashrc(USER_BASHRC);
	if (!bashrc.open(QFile::Text | QFile::WriteOnly))
	{
		//TODO error
		return;
	}
	QTextStream bashrcStream(&bashrc);
	bashrcStream << data.bashrc;
	bashrc.close();

	QFile program(PROGRAM_BASHRC);
	QFileInfo programInfo(program);
	QDir dir;
	if (!dir.exists(programInfo.absolutePath()))
	{
		DEBUG << "Creating dir: " << programInfo.absolutePath();
		if (!dir.mkpath(programInfo.absolutePath()))
			DEBUG << "Failed creating dir";
	}
	if (!program.open(QFile::Text | QFile::WriteOnly))
	{
		//TODO error
		//doesn't exist or can't access
		DEBUG << program.fileName() + "isn't writable or exists";
		return;
	}
	QTextStream programStream(&program);
	programStream << data.program;
	program.close();

	QFile bashrcAliases(USER_BASHRC_ALIASES);
	QFileInfo bashrcAliasesInfo(bashrcAliases);
	if (!dir.exists(bashrcAliasesInfo.absolutePath()))
	{
		DEBUG << "Creating dir: " << bashrcAliasesInfo.absolutePath();
		if (!dir.mkpath(bashrcAliasesInfo.absolutePath()))
			DEBUG << "Failed creating dir";
	}
	if (!bashrcAliases.open(QFile::Text | QFile::WriteOnly))
	{
		//TODO error
		//doesn't exist or can't access
		DEBUG << bashrcAliases.fileName() + "isn't writable or exists";
		return;
	}
	QTextStream bashrcAliasesStream(&bashrcAliases);
	bashrcAliasesStream << data.bashrcAliases;
	bashrcAliases.close();
}

Window::TabManager::TabManager(Ui::Window* ui)
	: window_ui(ui)
{
	SCOPE_TRACKER;
}

Window::TabManager& Window::TabManager::addTabs(QList<Tab*> tabs)
{
	SCOPE_TRACKER;
	for (Tab* tab : tabs)
	{
		m_tabs.append(tab);
	}
	return *this;
}

Window::TabManager& Window::TabManager::addTab(Tab* tab)
{
	SCOPE_TRACKER;
	addTabs(QList<Tab*>() << tab);
	return *this;
}

Window::TabManager& Window::TabManager::setup(const BashrcSource& source)
{
	SCOPE_TRACKER;
	for (Tab* tab : m_tabs)
	{
		tab->setup(source);
		if (tab->widget() != nullptr)
			window_ui->tabWidget_Tabs->addTab(tab->widget(), tab->icon(), tab->name());
	}
	return *this;
}

BashrcSource Window::TabManager::exec(const BashrcSource& source)
{
	SCOPE_TRACKER;
	BashrcSource rtn = source;
	for (Tab* tab : m_tabs)
	{
		rtn = tab->exec(rtn);
	}
	return rtn;
}

void Window::writePositionSettings()
{
	SCOPE_TRACKER;
	QSettings qsettings("MX-Linux", "bash-config");

	qsettings.beginGroup("mainwindow");

	qsettings.setValue("geometry", saveGeometry());
	qsettings.setValue("maximized", isMaximized());
	if (!isMaximized())
	{
		qsettings.setValue("size", size());
	}

	qsettings.endGroup();
}

void Window::readPositionSettings()
{
	SCOPE_TRACKER;
	QSettings qsettings("MX-Linux", "bash-config");

	qsettings.beginGroup("mainwindow");

	restoreGeometry(qsettings.value("geometry", saveGeometry()).toByteArray());
	resize(qsettings.value("size", size()).toSize());
	if (qsettings.value("maximized", isMaximized()).toBool())
		showMaximized();

	qsettings.endGroup();
}

void Window::on_pushButton_Help_clicked()
{
	SCOPE_TRACKER;
	QLocale locale;
	QString lang = locale.bcp47Name();

	QFileInfo viewer("/usr/bin/mx-viewer");
	QFileInfo viewer2("/usr/bin/antix-viewer");

	QString url = "file:///usr/share/doc/bash-config/help/bash-config.html";
	QString cmd;

	if (viewer.exists())
	{
		cmd = QString("mx-viewer %1 '%2' &").arg(url).arg(tr("Bash Config"));
	}
	else if (viewer2.exists())
	{
		cmd = QString("antix-viewer %1 '%2' &").arg(url).arg(tr("Bash Config"));
	}
	else
	{
		cmd = QString("xdg-open %1 &").arg(url).arg(tr("Bash Config"));
	}

	system(cmd.toUtf8());
}

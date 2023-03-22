#ifndef GLOBAL_H
#define GLOBAL_H

#include "searcher.h"
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

static QStringList __debug_start__ = {QString("noscope")};

QString randomString(int length, QString possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
QString bashInteractiveVariable(QString name);
QString escapeRegexCharacters(QString input);

struct BashrcSource
{
	QString bashrc;
	QString program;
	QString bashrcAliases;
};
struct ExecuteResult
{
	int rv;
	char __padding__[4]; // just for this compiler warning: https://stackoverflow.com/questions/20184259/what-does-the-padding-class-tester-with-4-bytes-warning-mean
	QString output;
};

#define DEBUG_ENTER(x)
#define DEBUG_EXIT(x)

ExecuteResult runCmd(QString cmd, bool interactive = false, bool onlyStdout = false);

#define joinPath(path1, path2) QDir::cleanPath(path1 + QDir::separator() + path2)

#define NAME QString("Bash Config")
#define NAME_BIN QString("bash-config")
#define ORG QString("MX-Linux")

#define USER_BASHRC joinPath(QDir::homePath(), ".bashrc")
#define USER_BASHRC_ALIASES joinPath(QDir::homePath(), ".bash_aliases")
#define PROGRAM_BASHRC joinPath(QDir::homePath(), ".config/bash-config/bashrc.bash")
#define SUGGEST_ALIASES joinPath(QDir::homePath(), ".config/bash-config/suggest_aliases")
#define BACKUP_BASHRCS joinPath(QDir::homePath(), ".config/bash-config/")

#define DEBUG qDebug() << __debug_start__.last()
#define DEBUG_VAR(x) qDebug() << __debug_start__.last() << ":" << #x << " = " << x
#define DEBUG_POS qDebug() << __FILE__ << ":" << __LINE__
#define CHECK_SEARCH(x) ((x != Searcher::ReturnValueSearchStatesFailed) && (x != Searcher::ReturnValueSearchStringNotFound))
//#define DEBUG_ENTER(x) __debug_start__.append(QObject::tr(__FILE__) + ":" + QString::number(__LINE__) + ":" + QObject::tr(#x)) //qDebug() << "+++ " << #x << " +++"
//#define DEBUG_EXIT(x) __debug_start__.pop_back() //qDebug() << "--- " << #x << " ---"
//#define CHECK(x) if(x == -1) continue

struct ScopeTracker
{
	QString output;
	bool trace;
	ScopeTracker(QString output, bool trace = false) : output(output), trace(trace)
	{
		//__debug_start__.push_back(output);
		if (trace)
			qDebug().noquote() << "+++ " << output << " +++";
	}
	~ScopeTracker()
	{
		//__debug_start__.pop_back();
		if (trace)
			qDebug().noquote() << "---" << output << " ---";
	}
};

#define SCOPE_TRACKER \
	ScopeTracker _scope_tracker { __PRETTY_FUNCTION__, false }

#endif // GLOBAL_H

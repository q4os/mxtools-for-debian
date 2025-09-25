#include "othertab.h"

#include "ui_othertab.h"

#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <algorithm>

OtherTab::OtherTab()
	: Tab("Other")
{
	SCOPE_TRACKER;
	ui = new Ui::OtherTab;
	setWidget(new QWidget);
	ui->setupUi(widget());
	ui->listWidget_Paths->setSelectionBehavior(QListWidget::SelectRows);
	connect(ui->pushButton_PathAdd, &QPushButton::clicked, [=]() {
		QString path = QFileDialog::getExistingDirectory(widget(), "Select a directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (path.length() == 0)
			return;
		ui->listWidget_Paths->addItem(new PathListItem{path, false});
	});

	connect(ui->pushButton_PathRemove, &QPushButton::clicked, [=]() {
		QList<QListWidgetItem*> items = ui->listWidget_Paths->selectedItems();
		QList<int> rows;
		for (QListWidgetItem* item : items)
		{
			rows.append(ui->listWidget_Paths->row(item));
		}
		for (int i = ui->listWidget_Paths->count() - 1; i > -1; i--)
		{
			if (!rows.contains(i))
				continue;
			PathListItem* item = static_cast<PathListItem*>(ui->listWidget_Paths->takeItem(i));
			PathData data;
			data.path = item->path();
			data.inBashrc = item->inBashrc();
			delete item;
			m_removedItems.append(data);
		}
	});

	connect(ui->listWidget_Paths, &QListWidget::itemDoubleClicked, [=](QListWidgetItem* ritem) {
		QString path = QFileDialog::getExistingDirectory(widget(), "Select a directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (path.length() == 0)
			return;
		PathListItem* item = static_cast<PathListItem*>(ritem);
		item->setPath(path);
	});

	connect(ui->pushButton_PathEdit, &QPushButton::clicked, [=]() {
		QList<QListWidgetItem*> items = ui->listWidget_Paths->selectedItems();
		if (items.size() == 0)
			return;
		QString path = QFileDialog::getExistingDirectory(widget(), "Select a directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (path.length() == 0)
			return;
		PathListItem* item = static_cast<PathListItem*>(items[0]);
		item->setPath(path);
	});

	connect(ui->checkBox_HistoryLength, &QCheckBox::clicked, [=](bool) {
		updateHistoryLengthWidgets();
	});

	connect(ui->checkBox_InfiniteHistoryLength, &QCheckBox::clicked, [=](bool) {
		updateHistoryLengthWidgets();
	});
}

OtherTab::~OtherTab()
{
	SCOPE_TRACKER;
	delete ui;
}

void OtherTab::setup(const BashrcSource& source)
{
	SCOPE_TRACKER;
	QStringList bashrcList = findPathAdditions(source.bashrc);
	bashrcList = cleanPathAdditions(bashrcList);
	DEBUG_VAR(bashrcList.size());
	for (QString item : bashrcList)
	{
		ui->listWidget_Paths->addItem(new PathListItem{item, true});
	}
	QStringList programList = findPathAdditions(source.program);
	programList = cleanPathAdditions(programList);
	DEBUG_VAR(programList.size());
	for (QString item : programList)
	{
		ui->listWidget_Paths->addItem(new PathListItem{item, false});
	}
	QRegularExpression regex{"(export\\s|)\\s*HISTSIZE=([\\d-]+)"};
	QRegularExpressionMatchIterator iter = regex.globalMatch(source.bashrc);
	while (iter.hasNext())
	{
		QRegularExpressionMatch match = iter.next();
		int val = match.captured(2).toInt();
		if (val < 0)
		{
			ui->checkBox_InfiniteHistoryLength->setChecked(true);
		}
		else
		{
			ui->spinBox_HistoryLength->setValue(val);
		}
		ui->checkBox_HistoryLength->setChecked(false);
	}
	iter = regex.globalMatch(source.program);
	while (iter.hasNext())
	{
		QRegularExpressionMatch match = iter.next();
		int val = match.captured(2).toInt();
		if (val < 0)
		{
			ui->checkBox_InfiniteHistoryLength->setChecked(true);
		}
		else
		{
			ui->spinBox_HistoryLength->setValue(val);
		}
		ui->checkBox_HistoryLength->setChecked(true);
	}
	updateHistoryLengthWidgets();
}

BashrcSource OtherTab::exec(const BashrcSource& source)
{
	SCOPE_TRACKER;
	BashrcSource result = source;
	for (int i = 0; i < ui->listWidget_Paths->count(); i++)
	{
		PathListItem* item = static_cast<PathListItem*>(ui->listWidget_Paths->item(i));
		if (item->inBashrc())
			continue;
		if (item->path().length() == 0)
			continue;
		result.program.append(QString("\nexport PATH=\"$PATH:%1\"\n").arg(item->path()));
	}
	for (PathData data : m_removedItems)
	{
		if (data.inBashrc)
		{
			DEBUG << "Removing: " << data.path;
			removePath(result.bashrc, data.path);
		}
	}
	bool exists = false;
	int historySize = findHistoryLength(source.bashrc, &exists);
	if (!exists || (historySize != uiHistoryLengthValue()))
	{
		if (ui->checkBox_HistoryLength->isChecked())
		{
			DEBUG << "Writing HISTSIZE to program bashrc";
			result.program.append(QString("\nexport HISTSIZE=%1\n").arg(uiHistoryLengthValue()));
		}
	}
	return result;
}

QStringList OtherTab::findPathAdditions(const QString& source)
{
	SCOPE_TRACKER;
	QRegularExpression regex{"(export\\s|)\\s{0,}PATH=\"(((\\$\\{{0,1}PATH\\}{0,1}:|)([\\w\\s\\d]+))|(([\\w\\d\\s]+)(:\\$\\{{0,1}PATH\\}{0,1}|)))\""};
	QRegularExpression regex2{R"(\s{0,}(export\s|)\s{0,}PATH="(((\$\{{0,1}PATH\}{0,1}:|)([\w\s\d]+))|(([\w\d\s]+)(:\$\{{0,1}PATH\}{0,1}|)))\"\s{0,})"};
    QRegularExpression regex3{R"((export\s|)\s{0,}PATH=("|)([^\n"]+)("|))"};
	QRegularExpression regex4{"PATH=\"(.*)\""};
	if (!regex3.isValid())
	{
		DEBUG << "Regex invalid";
		DEBUG << regex3.errorString();
	}
	QRegularExpressionMatchIterator iter = regex3.globalMatch(source, QRegularExpression::PartialPreferCompleteMatch);
	QStringList list;
	while (iter.hasNext())
	{
		DEBUG << "Failure";
		QRegularExpressionMatch match = iter.next();
		if (!match.isValid())
			continue;
        list.append(match.captured(3));
	}
	return list;
}

QStringList OtherTab::cleanPathAdditions(const QStringList& additions)
{
	SCOPE_TRACKER;
	QRegularExpression pathRegex{"\\$\\{*PATH\\}*"};
	QStringList result;
	for (auto addition : additions)
	{
		QStringList parts = addition.split(':');
		DEBUG << "Cleaning";
		DEBUG_VAR(parts);
		parts.removeAll("$PATH");
		parts.removeAll("${PATH}");
        // remove relative paths as it is impossible to edit with current ui
        /*parts.erase(std::remove_if(parts.begin(), parts.end(), [](QString part){
            if(part.startsWith("/") || part.startsWith("~/") || part.startsWith("$HOME/") || part.startsWith("${HOME}/")) return false;
            return true;
        }));*/
		DEBUG_VAR(parts);
		result.append(parts);
	}
	return result;
}

void OtherTab::removePath(QString& source, const QString& path)
{
	SCOPE_TRACKER;
	QRegularExpression regex{"(export|)[\\s]+PATH=(\"|)([0-9a-z/:\\$]+)\\2"};
    //QRegularExpression regex3{R"((export\s|)\s{0,}PATH="(.+)\")"};
    QRegularExpression regex3{R"((export\s|)\s{0,}PATH=("|)([^\n"]+)("|))"};
    QRegularExpressionMatchIterator iter = regex3.globalMatch(source);
	DEBUG << "Path to find: " << path;
	while (iter.hasNext())
	{
		QRegularExpressionMatch match = iter.next();
		DEBUG << "Found for maybe removal: " << match.captured();
		QString content = match.captured(2);
		if (content.contains(path))
		{
			QStringList parts = content.split(':');
			parts.removeAll(path);
			QString replacement = parts.join(':');
			source.remove(match.capturedStart(2), match.capturedLength(2));
			source.insert(match.capturedStart(2), replacement);
			return;
		}
	}
	// didn't find in source
}

void OtherTab::updateHistoryLengthWidgets()
{
	SCOPE_TRACKER;
	if (ui->checkBox_HistoryLength->isChecked())
	{
		ui->spinBox_HistoryLength->setEnabled(true);
		ui->checkBox_InfiniteHistoryLength->setEnabled(true);
		if (ui->checkBox_InfiniteHistoryLength->isChecked())
		{
			ui->spinBox_HistoryLength->setEnabled(false);
		}
	}
	else
	{
		ui->spinBox_HistoryLength->setEnabled(false);
		ui->checkBox_InfiniteHistoryLength->setEnabled(false);
	}
}

int OtherTab::findHistoryLength(const QString& source, bool* exists)
{
	SCOPE_TRACKER;
	QRegularExpression regex{"(export\\s|)\\s*HISTSIZE=([\\d-]+)"};
	QRegularExpressionMatchIterator iter = regex.globalMatch(source);
	int overall = 0;
	if (exists != nullptr)
	{
		*exists = iter.hasNext();
	}
	while (iter.hasNext())
	{
		QRegularExpressionMatch match = iter.next();
		overall = match.captured(2).toInt();
	}
	return overall;
}

int OtherTab::uiHistoryLengthValue()
{
	SCOPE_TRACKER;
	if (ui->checkBox_InfiniteHistoryLength->isChecked())
		return -1;
	return ui->spinBox_HistoryLength->value();
}

PathListItem::PathListItem(const QString& path, bool inBashrc)
{
	SCOPE_TRACKER;
	setPath(path);
	setInBashrc(inBashrc);
}

void PathListItem::setPath(const QString& path)
{
	SCOPE_TRACKER;
	m_path = path;
	m_path.replace("$HOME", "~");
	m_path.replace(QDir::homePath(), "~");
	m_path = QDir::cleanPath(m_path);
	setText(m_path);
}

QString PathListItem::path() const
{
	SCOPE_TRACKER;
	QString result = m_path;
	result.replace("~", "$HOME");
	result.replace(QDir::homePath(), "$HOME");
	result = QDir::cleanPath(result);
	return result;
}

void PathListItem::setInBashrc(bool inBashrc)
{
	SCOPE_TRACKER;
	m_inBashrc = inBashrc;
}

bool PathListItem::inBashrc() const
{
	SCOPE_TRACKER;
	return m_inBashrc;
}

void variableGroup(FuzzyBashStream *stream, QList<FuzzyBashStream::Token> &tokens)
{
    QList<FuzzyBashStream::Token> result;
    for(auto token : tokens)
    {
    }
    tokens = result;
}

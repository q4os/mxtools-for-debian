#pragma once

namespace Ui
{
	class OtherTab;
}

#include "tab.h"
#include <QListWidgetItem>
#include "fuzzybashstream.h"

void variableGroup(FuzzyBashStream* stream, QList<FuzzyBashStream::Token>& tokens);

class PathListItem : public QListWidgetItem
{
  public:
	PathListItem(const QString& path, bool inBashrc = false);
	void setPath(const QString& path);
	QString path() const;
	void setInBashrc(bool inBashrc);
	bool inBashrc() const;

  private:
	bool m_inBashrc;
	QString m_path;
};

struct PathData
{
	QString path;
	bool inBashrc;
};

class OtherTab : public Tab
{
	Q_OBJECT
  public:
	OtherTab();
	~OtherTab() override;
	void setup(const BashrcSource& source) override;
	BashrcSource exec(const BashrcSource& source) override;

  private:
	QStringList findPathAdditions(const QString& source);
	QStringList cleanPathAdditions(const QStringList& additions);

	QList<PathData> m_removedItems;
	void removePath(QString& source, const QString& path);

	void updateHistoryLengthWidgets();

	int findHistoryLength(const QString& source, bool* exists = nullptr);

	int uiHistoryLengthValue();

	Ui::OtherTab* ui;
};

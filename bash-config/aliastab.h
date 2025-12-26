#pragma once

#include "aliasstream.h"
#include "tab.h"
#include "ui_aliastab.h"
#include <QSet>

template <class, class>
class QMap;

namespace Ui
{
	class AliasTab;
}

class QCheckBox;

//special spec for a table item class so it can hold some data
class AliasTabTableWidgetItem : public QObject, public QTableWidgetItem
{
	Q_OBJECT
  public:
	AliasTabTableWidgetItem() = default;
	AliasTabTableWidgetItem(const QString& text, const QVariant& info = QVariant());
	~AliasTabTableWidgetItem();
	AliasTabTableWidgetItem& setInfo(const QVariant& info);
    QVariant info();

  protected:
	QVariant m_info;
};

class AliasTab : public Tab
{
	Q_OBJECT
  public:
	AliasTab();
	virtual ~AliasTab();

	void setup(const BashrcSource& data) override;
	BashrcSource exec(const BashrcSource& data) override;

  protected:
	Ui::AliasTab* ui;
	QList<Alias> m_deletedAliases;
	QMap<QCheckBox*, Alias> m_aliasWithCheckboxes;
    QSet<QString> m_bashrcAliasNames;
    QSet<QString> m_bashrcAliasesNames;
};

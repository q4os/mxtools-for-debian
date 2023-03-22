#ifndef PROMPTTAB_H
#define PROMPTTAB_H

#include "tab.h"
#include "ui_prompttab_fix.h"

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QPushButton;
class QLineEdit;

QString terminalEncode(QString input);
QString terminalDecode(QString input);

namespace Ui
{
	class PromptTab;
}

namespace v2
{

	class CustomPromptItemProperty : public QObject
	{
		Q_OBJECT
	  public:
		enum class Location
		{
			Begin,
			End,
		};
		enum class Gui
		{
			InGroup,
			Alone
		};
		CustomPromptItemProperty(QObject* parent = nullptr);
		virtual QString stringify() = 0;
		virtual Gui gui() = 0;
		virtual Location location() = 0;
		QWidget* widget() { return m_widget; }

	  private:
		QWidget* m_widget;
	};

	class CustomPromptItem : QObject
	{
		Q_OBJECT
	  public:
		CustomPromptItem(QObject* parent = nullptr);
		void edit();
		QMap<QString, CustomPromptItemProperty*> properties() const { return m_properties; }
		QMap<QString, CustomPromptItemProperty*>& properties() { return m_properties; }

	  private:
		QMap<QString, CustomPromptItemProperty*> m_properties;
	};

} // namespace v2

enum class CustomPromptItemType
{
	Base = 0,
	SimpleText,
	Text,
	Special,
	Module,
};

enum class CustomPromptPropertyType
{
	Base = 0,
	Color,
	Checkbox,
	Text,
	Filename,
	Dirname,
};

class XmlConverter;

class CustomPromptProperty : public QObject
{
	Q_OBJECT
  public:
	CustomPromptProperty(QString name, QObject* parent = nullptr);
	virtual ~CustomPromptProperty();
	QWidget* widget() { return m_widget; }
	void setWidget(QWidget* widget) { m_widget = widget; }
	QString name() const { return m_name; }
	void setName(QString name) { m_name = name; }
	virtual bool good() = 0; // reporting that all inputs are good
	virtual CustomPromptPropertyType type() const { return CustomPromptPropertyType::Base; }

  protected:
	QWidget* m_widget;
	QString m_name;
};

//class DirnameProperty : public CustomPromptProperty
//{
//    Q_OBJECT
//public:
//    DirnameProperty(QString name, QObject* parent = nullptr);
//    bool good() override;
//    void setPath(QString dir) { m_path = dir; }
//    QString path() const { return m_path; }
//    CustomPromptPropertyType type() const override { return CustomPromptPropertyType::Dirname; }
//private:
//    QString m_path;
//};

//class FilenameProperty : public CustomPromptProperty
//{
//    Q_OBJECT
//public:
//    FilenameProperty(QString name, QObject* parent = nullptr);
//    bool good() override;
//    void setPath(QString dir) { m_path = dir; }
//    QString path() const { return m_path; }
//    CustomPromptPropertyType type() const override { return CustomPromptPropertyType::Filename; }
//private:
//    QString m_path;
//};

class ColorProperty : public CustomPromptProperty
{
	Q_OBJECT
  public:
	ColorProperty(QString name, QObject* parent = nullptr);
	bool good() override;
	void setColor(QColor color)
	{
		m_color = color;
		updateBtn();
	}
	QColor color() const { return m_color; }
	bool enabled() const { return m_enabledColor; }
	void setEnabled(bool enabled)
	{
		m_enabledColor = enabled;
		updateBtn();
	}
	// TODO move out of the ColorProperty class
	QString stringify(QColor color) const;
	CustomPromptPropertyType type() const override { return CustomPromptPropertyType::Color; }
  private slots:
	void onBtnPress();

  private:
	void updateBtn();
	QPushButton* m_colorBtn;
	QColor m_color;
	QCheckBox* m_enabled;
	bool m_enabledColor;
};

class CheckboxProperty : public CustomPromptProperty
{
	Q_OBJECT
  public:
	CheckboxProperty(QString name, QObject* parent = nullptr);
	bool good() override;
	void setChecked(bool checked) { m_checkbox->setChecked(checked); }
	bool checked() { return m_checkbox->isChecked(); }
	CustomPromptPropertyType type() const override { return CustomPromptPropertyType::Checkbox; }

  private:
	QCheckBox* m_checkbox;
};

class TextProperty : public CustomPromptProperty
{
	Q_OBJECT
  public:
	TextProperty(QString name, QObject* parent = nullptr);
	bool good() override;
	void setText(QString text) { m_editor->setText(text); }
	QString text() { return m_editor->text(); }
	CustomPromptPropertyType type() const override { return CustomPromptPropertyType::Text; }

  private:
	QLineEdit* m_editor;
};

class CustomPromptItem : public QListWidgetItem
{
  public:
	CustomPromptItem(QString name);
	QVector<CustomPromptProperty*> properties() const { return m_properties; }
	CustomPromptProperty* propertyByName(QString name) const;
	virtual void updateMembers() {}
	virtual void updateProperties() {}
	virtual CustomPromptItemType type() const { return CustomPromptItemType::Base; }

  protected:
	QVector<CustomPromptProperty*> m_properties;
	friend XmlConverter;
};

class SimpleTextItem : public CustomPromptItem
{
  public:
	SimpleTextItem(QString name);
	void updateMembers() override;
	void updateProperties() override;
	CustomPromptItemType type() const override { return CustomPromptItemType::SimpleText; }
	QColor propertyForeground() const { return m_foreground; }
	void setPropertyForeground(QColor color)
	{
		m_foreground = color;
		updateProperties();
	}
	QColor propertyBackground() const { return m_background; }
	void setPropertyBackground(QColor color)
	{
		m_background = color;
		updateProperties();
	}
	bool propertyForegroundEnabled() const { return m_foregroundEnabled; }
	void setPropertyForegroundEnabled(bool enabled)
	{
		m_foregroundEnabled = enabled;
		updateProperties();
	}
	bool propertyBackgroundEnabled() const { return m_backgroundEnabled; }
	void setPropertyBackgroundEnabled(bool enabled)
	{
		m_backgroundEnabled = enabled;
		updateProperties();
	}
	bool propertyBold() const { return m_bold; }
	void setPropertyBold(bool bold)
	{
		m_bold = bold;
		updateProperties();
	}

  private:
	QColor m_foreground, m_background;
	bool m_bold = false, m_foregroundEnabled = false, m_backgroundEnabled = false;
	//    QColor m_defaultForegroundColor, m_defaultBackgroundColor;
};

class TextItem : public SimpleTextItem
{
  public:
	TextItem(QString name, QString text);
	void updateMembers() override;
	void updateProperties() override;
	CustomPromptItemType type() const override { return CustomPromptItemType::Text; }
	QString propertyText() const { return m_text; }
	void setPropertyText(QString text)
	{
		m_text = text;
		updateProperties();
	}

  protected:
	QString m_text;
};

class SpecialItem : public SimpleTextItem
{
  public:
	enum class Type
	{
		HostLong,
		HostShort,
		Username,
		WorkingLong,
		WorkingShort,
	};
	static QString typeToBashString(Type t);
	SpecialItem(QString name, Type type);
	void updateMembers() override;
	void updateProperties() override;
	Type itemType() const { return m_type; }
	void setItemType(Type t)
	{
		m_type = t;
		refreshText();
	}
	CustomPromptItemType type() const override { return CustomPromptItemType::Special; }

  protected:
	void refreshText();
	Type m_type;
};

class CustomItemSelectorDialog
{
  public:
	static QListWidgetItem* getItem(QWidget* parent = nullptr);
};

class CustomPromptItemEditor
{
  public:
	static void edit(CustomPromptItem* item, QWidget* parent = nullptr);
};

class XmlConverter
{
  public:
	static QString convert(CustomPromptItem* item);
	static CustomPromptItem* convert(QString source);
};

class PromptTab : public Tab
{
  public:
	PromptTab();
	~PromptTab();
	void setup(const BashrcSource data);
	BashrcSource exec(const BashrcSource data);

  protected:
	Ui::PromptTab* ui;
    const QString customPromptCommentString = "BASH_CONFIG_USING_CUSTOM_PROMPT";
};

QString itemToXml(CustomPromptItem* item);
CustomPromptItem* xmlToItem(QString xml);

#endif // PROMPTTAB_H

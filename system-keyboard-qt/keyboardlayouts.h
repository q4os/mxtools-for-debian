#ifndef KEYBOARDLAYOUTS_H
#define KEYBOARDLAYOUTS_H

#include <QObject>
#include <QXmlStreamReader>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QFileInfo>

struct KeyboardModel
{
    QString name;
    QString description;
    QString vendor;
};

struct KeyboardConfigItem
{
    QString name;
    QString shortDescription;
    QString description;
    QStringList languages;
    QStringList countries;
};

struct KeyboardLayout
{
    KeyboardConfigItem config;
    QList<KeyboardConfigItem> variants;
};

struct KeyboardOptionGroup
{
    KeyboardConfigItem config;
    QList<KeyboardConfigItem> options;
};

class KeyboardInfo
{
public:
    const QString XkbDataDirectory = "/usr/share/X11/xkb/rules";
    const QString LayoutFlagIcons = "/usr/share/flags-common";
    KeyboardInfo();
    KeyboardInfo(const KeyboardInfo& other);
    QList<KeyboardModel> models() const { return m_models; }
    QList<KeyboardLayout> layouts() const { return m_layouts; }
    QList<KeyboardOptionGroup> options() const { return m_optionGroups; }
    QIcon layoutIcon(QString layoutName);
private:
    void reparse();
    void readDocument(QXmlStreamReader& reader);
    void readXKBConfig(QXmlStreamReader& reader);
    void readModelList(QXmlStreamReader& reader);
    void readModel(QXmlStreamReader& reader);
    void readLayoutList(QXmlStreamReader& reader);
    void readLayout(QXmlStreamReader& reader);
    void readConfigItem(QXmlStreamReader& reader, KeyboardConfigItem& item);
    void readModelConfigItem(QXmlStreamReader& reader, KeyboardModel& parent);
    void readLanguageList(QXmlStreamReader& reader, QStringList& languages);
    void readVariantList(QXmlStreamReader& reader, KeyboardLayout& parent);
    void readVariant(QXmlStreamReader& reader, KeyboardLayout& parent);
    void readOptionList(QXmlStreamReader& reader);
    void readOptionGroup(QXmlStreamReader& reader);
    void readOption(QXmlStreamReader& reader, KeyboardOptionGroup& parent);
    void addLayout(const KeyboardLayout& layout);
    QList<KeyboardModel> m_models;
    QList<KeyboardLayout> m_layouts;
    QList<KeyboardOptionGroup> m_optionGroups;
};

#endif // KEYBOARDLAYOUTS_H

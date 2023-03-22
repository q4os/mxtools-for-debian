#include "keyboardlayouts.h"

KeyboardInfo::KeyboardInfo()
{
    reparse();
}

KeyboardInfo::KeyboardInfo(const KeyboardInfo &other)
{
    m_models = other.m_models;
    m_layouts = other.m_layouts;
    m_optionGroups = other.m_optionGroups;
}

void KeyboardInfo::reparse()
{
    m_models.clear();
    m_layouts.clear();
    m_optionGroups.clear();
    QStringList files = QDir(XkbDataDirectory).entryList({"base*.xml"});
    std::reverse(files.begin(), files.end());
    for(auto file : files)
    {
        QString filename = QDir::cleanPath(XkbDataDirectory + QDir::separator() + file);
        QFile io{filename};
        if(!io.open(QFile::ReadOnly))
        {
            qWarning() << "Failed to open file:" << filename;
            continue;
        }
        QXmlStreamReader reader{&io};
        readDocument(reader);
        io.close();
    }
}

void KeyboardInfo::readDocument(QXmlStreamReader& reader)
{
    while(reader.readNextStartElement())
    {
        if(reader.name() == "xkbConfigRegistry")
            readXKBConfig(reader);
    }
}

void KeyboardInfo::readXKBConfig(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "modelList")
            readModelList(reader);
        else if(reader.name() == "layoutList")
            readLayoutList(reader);
        else if(reader.name() == "optionList")
            readOptionList(reader);
    }
}

void KeyboardInfo::readModelList(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "model")
            readModel(reader);
    }
}

void KeyboardInfo::readModel(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.isStartElement());
    KeyboardModel model;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "configItem")
            readModelConfigItem(reader, model);
    }
    m_models.append(model);
}

void KeyboardInfo::readLayoutList(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "layout")
            readLayout(reader);
    }
}

void KeyboardInfo::readLayout(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    KeyboardLayout layout;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "configItem")
            readConfigItem(reader, layout.config);
        else if(reader.name() == "variantList")
            readVariantList(reader, layout);
    }
    addLayout(layout);
}

void KeyboardInfo::readConfigItem(QXmlStreamReader& reader, KeyboardConfigItem &item)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "name")
            item.name = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else if(reader.name() == "shortDescription")
            item.shortDescription = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else if(reader.name() == "description")
            item.description = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else if(reader.name() == "languageList")
            readLanguageList(reader, item.languages);
        else if(reader.name() == "countryList")
            readLanguageList(reader, item.countries);
    }
}

void KeyboardInfo::readModelConfigItem(QXmlStreamReader& reader, KeyboardModel &parent)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "name")
            parent.name = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else if(reader.name() == "description")
            parent.description = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else if(reader.name() == "vendor")
            parent.vendor = reader.readElementText(QXmlStreamReader::IncludeChildElements);
        else reader.skipCurrentElement();
    }
}

void KeyboardInfo::readLanguageList(QXmlStreamReader& reader, QStringList &languages)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        languages.append(reader.readElementText(QXmlStreamReader::IncludeChildElements));
    }
}

void KeyboardInfo::readVariantList(QXmlStreamReader& reader, KeyboardLayout &parent)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "variant")
            readVariant(reader, parent);
    }
}

void KeyboardInfo::readVariant(QXmlStreamReader& reader, KeyboardLayout &parent)
{
    Q_ASSERT(reader.isStartElement());
    KeyboardConfigItem item;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "configItem")
            readConfigItem(reader, item);
    }
    parent.variants.append(item);
}

void KeyboardInfo::readOptionList(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    while(reader.readNextStartElement())
    {
        if(reader.name() == "group")
            readOptionGroup(reader);
    }
}

void KeyboardInfo::readOptionGroup(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.isStartElement());
    KeyboardOptionGroup group;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "configItem")
            readConfigItem(reader, group.config);
        else if(reader.name() == "option")
            readOption(reader, group);
    }
    m_optionGroups.append(group);
}

void KeyboardInfo::readOption(QXmlStreamReader &reader, KeyboardOptionGroup &parent)
{
    Q_ASSERT(reader.isStartElement());
    KeyboardConfigItem item;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "configItem")
            readConfigItem(reader, item);
    }
    parent.options.append(item);
}

void KeyboardInfo::addLayout(const KeyboardLayout &layout)
{
    auto iter = std::find_if(m_layouts.begin(), m_layouts.end(), [layout](const KeyboardLayout& other){
        return other.config.name == layout.config.name;
    });
    if(iter == m_layouts.end())
    {
        m_layouts.append(layout);
        return;
    }
    (*iter).variants.append(layout.variants);
}

QIcon KeyboardInfo::layoutIcon(QString layoutName)
{
    QString path = QDir::cleanPath(LayoutFlagIcons + QDir::separator() + layoutName + ".png");
    QFileInfo fileInfo{path};
    if(fileInfo.isFile())
        return QIcon{path};
    else
        return QIcon{};
}




#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include "translation.h"
#include "keyboardlayouts.h"

#include <QStringListModel>
#include <QSortFilterProxyModel>

namespace Ui {
class Window;
}

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

enum ComboBoxRoles
{
    OptionName = Qt::UserRole,
};

const QStringList hotkeyOptions = {"grp", "lv3", "Compose key"};
const QStringList advancedOptions = {"grp_led", "caps", "terminate"};
const QString KeyboardDefaultFile = "/etc/default/keyboard";

class KeyboardLayoutListWidgetItem : public QListWidgetItem
{
public:
    KeyboardLayoutListWidgetItem(KeyboardConfigItem layout, KeyboardConfigItem variant, QIcon icon)
        : m_layout(layout), m_variant(variant)
    {
        QString variantTranslated = keyboardtr(variant.description);
        if(variant.description.isEmpty())
            variantTranslated = "";
        setText(keyboardtr(m_layout.name) + " " + keyboardtr(m_layout.description) + " " + variantTranslated);
        setIcon(icon);
    }
    virtual ~KeyboardLayoutListWidgetItem();
    KeyboardConfigItem m_layout;
    KeyboardConfigItem m_variant;
};

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();
private slots:
    void refreshLayoutButtonStates();
    QString generateOutput();
    void loadDefaults();

private:
    bool apply();

    QString getModel();
    QPair<QStringList, QStringList> getLayoutsAndVariants();
    QStringList getOptions();

private:
    void addLayout(KeyboardConfigItem layout, KeyboardConfigItem variant);
    void populateLayout(QLayout* layout, QStringList options);
    QMap<QComboBox*, QMap<int, QString>> m_comboBoxInfo;
    Ui::Window *ui;
    KeyboardInfo m_keyboardInfo;
    QStringList m_extraOptions;
    QStringListModel m_keyboardModelsModel;
    QSortFilterProxyModel m_keyboardModelsModelFilter;
};

#endif // WINDOW_H

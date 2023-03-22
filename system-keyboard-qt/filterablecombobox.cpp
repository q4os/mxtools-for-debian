#include "filterablecombobox.h"

#include <QDebug>

FilterableComboBox::FilterableComboBox(QWidget *parent)
    : QComboBox(parent), m_filterModel(this), m_completer(&m_filterModel, this)
{
    setEditable(true);
    m_filterModel.setSourceModel(model());
    m_filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);

    lineEdit()->setPlaceholderText(tr("Search"));

    m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    m_completer.setPopup(view());

    setCompleter(&m_completer);

    // have to make sure it only triggers on text being edited by the user because if it triggers every time
    // like when using QLineEdit::textChanged it will cause the filtering to update and filter out things
    // but that isn't wanted because when you scroll throught the options it changes the text in lineEdit() and
    // that causes filtering that can mess with how it is to actually work.
    connect(lineEdit(), &QLineEdit::textEdited, &m_filterModel, [this](const QString& text){
        m_filterModel.setFilterFixedString(text);
    });
    // When the completer is done, this ensures that the correct entry is selected and not some random invalid text
    connect(&m_completer, QOverload<const QString&>::of(&QCompleter::activated), [this](const QString& text){
        if(!text.isEmpty())
        {
            int index = findText(text);
            setCurrentIndex(index);
            m_saved = currentText();
        }
    });
}

FilterableComboBox::~FilterableComboBox()
{
}

void FilterableComboBox::setModel(QAbstractItemModel *model)
{
    QComboBox::setModel(model);
    m_filterModel.setSourceModel(model);
    m_completer.setModel(&m_filterModel);
}

void FilterableComboBox::setModelColumn(int column)
{
    m_completer.setCompletionColumn(column);
    m_filterModel.setFilterKeyColumn(column);
    QComboBox::setModelColumn(column);
}

void FilterableComboBox::setCurrentText(const QString &text)
{
    QComboBox::setCurrentText(text);
    if(findText(text) > -1)
    {
        m_saved = text;
    }
}

void FilterableComboBox::setCurrentIndex(int index)
{
    QComboBox::setCurrentIndex(index);
    if(index > -1)
    {
        m_saved = currentText();
    }
}

QAbstractItemView *FilterableComboBox::view()
{
    return m_completer.popup();
}

int FilterableComboBox::index()
{
    return currentIndex();
}

void FilterableComboBox::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    int index = findText(currentText());
    if(index == -1)
    {
        qDebug() << m_saved;
        index = findText(m_saved);
    }
    if(index == -1)
    {
        index = 0;
    }
    setCurrentIndex(index);
}


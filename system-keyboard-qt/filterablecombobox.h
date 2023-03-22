#ifndef FILITERABLECOMBOBOX_H
#define FILITERABLECOMBOBOX_H

#include <QComboBox>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QLineEdit>
#include <QFocusEvent>

// Based on this: https://stackoverflow.com/questions/4827207/how-do-i-filter-the-pyqt-qcombobox-items-based-on-the-text-input

class FilterableComboBox : public QComboBox
{
    Q_OBJECT
public:
    FilterableComboBox(QWidget* parent = nullptr);
    ~FilterableComboBox();
    void setModel(QAbstractItemModel* model);
    void setModelColumn(int column);
    void setCurrentText(const QString& text);
    void setCurrentIndex(int index);
    QAbstractItemView* view();
    int index();
    void focusOutEvent(QFocusEvent* event);
private:
    QString m_saved;
    QSortFilterProxyModel m_filterModel;
    QCompleter m_completer;

};

#endif // FILITERABLECOMBOBOX_H

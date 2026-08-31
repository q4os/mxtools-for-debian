#pragma once

#include <QTreeView>

// QTreeView's vertical scrollbar value() is in row-index units, not pixels, so
// bringing a partially-visible row into view needs converting a pixel offset
// into a row-count delta via the row's own height. CronView, ExecuteView, and
// VariableView all need this identical conversion.
class ScrollableTreeView : public QTreeView
{
public:
    explicit ScrollableTreeView(QWidget *parent = nullptr)
        : QTreeView(parent)
    {
    }

protected:
    void scrollTo(const QModelIndex &idx, ScrollHint hint) override;
};

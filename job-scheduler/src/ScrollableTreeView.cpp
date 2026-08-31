#include "ScrollableTreeView.h"

#include <QScrollBar>

void ScrollableTreeView::scrollTo(const QModelIndex &idx, ScrollHint /*hint*/)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(idx);
    if (rect.height() == 0) {
        return;
    }
    double step = 1.0 / rect.height();
    if (rect.top() < 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + static_cast<int>(rect.top() * step));
    } else if (rect.bottom() > area.bottom()) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()
                                      + static_cast<int>((rect.bottom() - area.bottom()) * step) + 5);
    }
}

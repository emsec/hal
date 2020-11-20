#include "gui/grouping/grouping_color_delegate.h"
#include "gui/grouping/grouping_table_model.h"
#include <QTableView>
#include <QBrush>
#include <QPainter>
#include <QRect>

namespace hal {

    void GroupingColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        const QTableView* tv = static_cast<const QTableView*>(parent());
        if (!tv) return;
        const GroupingTableModel* gtm = static_cast<const GroupingTableModel*>(tv->model());
        if (!gtm) return;
        QRect ro = option.rect;
        int w10 = ro.width()/10;
        int h10 = ro.height()/10;
        QRect rb(ro.topLeft() + QPoint(w10,h10), QSize(ro.width()-2*w10,ro.height()-2*h10));
        painter->fillRect(rb,Qt::black);
        QColor color = gtm->data(index,Qt::BackgroundRole).value<QColor>();
        QRect rc(ro.topLeft() + QPoint(2*w10,2*h10), QSize(ro.width()-4*w10,ro.height()-4*h10));
        painter->fillRect(rc, color);
    }

    QSize GroupingColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index);
        return option.rect.size();
    }
}

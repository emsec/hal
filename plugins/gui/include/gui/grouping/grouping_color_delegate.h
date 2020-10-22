#pragma once

#include <QStyledItemDelegate>

namespace hal {
    class GroupingColorDelegate : public QStyledItemDelegate
    {
        Q_OBJECT


    public:
        GroupingColorDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {;}

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    };
}

#pragma once

#include <QStyledItemDelegate>

namespace hal
{
    class GroupingColorDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        GroupingColorDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {;}

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    };
}

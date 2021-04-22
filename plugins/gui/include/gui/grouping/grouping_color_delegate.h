#pragma once

#include <QStyledItemDelegate>

namespace hal {
    /**
     * @ingroup utility_widgets-grouping
     * QStyledItemDelegate used for displaying a colored box in the 'Color'-column
     * within the grouping_manager_widget table.
     */
    class GroupingColorDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GroupingColorDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {;}

        /**
         * Paints the colored box in the table cell.
         *
         * @param painter - The used painter
         * @param option - The used style options
         * @param index - The index of the grouping within the GroupingTableModel
         */
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        /**
         * Returns the sized needed by the delegate to display the colored box.
         *
         * @param option - The used style options
         * @param index - The index of the grouping within the GroupingTableModel
         * @returns he sized needed by the delegate to display the colored box
         */
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    };
}

#pragma once

#include <QSortFilterProxyModel>
#include <QRegularExpression>

#include "graph_widget/contexts/graph_context.h"

namespace hal
{
    class ContextTableProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        ContextTableProxyModel(QObject* parent = 0);
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
        
    public Q_SLOTS:
        void handle_filter_text_changed(const QString& filter_text);

    protected:
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    private:
        QRegularExpression m_filter_expression;
    };
}

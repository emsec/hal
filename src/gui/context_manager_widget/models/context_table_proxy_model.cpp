#include "context_manager_widget/models/context_table_proxy_model.h"

#include "gui_utils/sort.h"

#include <QDateTime>

namespace hal
{
    ContextTableProxyModel::ContextTableProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
    {
        m_filter_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    bool ContextTableProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {        
        const QModelIndex& context_name_index = sourceModel()->index(source_row, 0, source_parent);
        const QModelIndex& context_date_index = sourceModel()->index(source_row, 1, source_parent);

        const QString& context_name = context_name_index.data().toString();
        const QString& context_date = context_date_index.data().toString();

        if(m_filter_expression.match(context_name).hasMatch() || m_filter_expression.match(context_date).hasMatch())
            return true;
        else
            return false; 
    }

    bool ContextTableProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        QVariant leftData = sourceModel()->data(left, Qt::UserRole);
        QVariant rightData = sourceModel()->data(right, Qt::UserRole);

        if(leftData.userType() == QMetaType::QDateTime)
            return leftData.toDateTime() > rightData.toDateTime();
        else
            return !(gui_utility::compare(gui_utility::sort_mechanism::natural, leftData.toString(), rightData.toString()));    
    }

    void ContextTableProxyModel::handle_filter_text_changed(const QString& filter_text)
    {
        m_filter_expression.setPattern(filter_text);
        invalidateFilter();
    }
}

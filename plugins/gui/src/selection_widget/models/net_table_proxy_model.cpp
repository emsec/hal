#include "gui/selection_widget/models/net_table_proxy_model.h"

net_table_proxy_model::net_table_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{
    m_filter_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
}

bool net_table_proxy_model::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex& net_name_index = sourceModel()->index(source_row, 0, source_parent);
    const QModelIndex& net_id_index = sourceModel()->index(source_row, 1, source_parent);

    const QString& net_name = net_name_index.data().toString();
    const QString& net_id = net_id_index.data().toString();

    if(m_filter_expression.match(net_name).hasMatch() || m_filter_expression.match(net_id).hasMatch())
        return true;
    else
        return false;
}

void net_table_proxy_model::handle_filter_text_changed(const QString& filter_text)
{
    m_filter_expression.setPattern(filter_text);
    invalidateFilter();
}

#include "gui/selection_widget/models/gate_table_proxy_model.h"

gate_table_proxy_model::gate_table_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{
    m_filter_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
}

bool gate_table_proxy_model::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex& gate_name_index = sourceModel()->index(source_row, 0, source_parent);
    const QModelIndex& gate_id_index = sourceModel()->index(source_row, 1, source_parent);
    const QModelIndex& gate_type_index = sourceModel()->index(source_row, 2, source_parent);

    const QString& gate_name = gate_name_index.data().toString();
    const QString& gate_id = gate_id_index.data().toString();
    const QString& gate_type = gate_type_index.data().toString();

    if(m_filter_expression.match(gate_type).hasMatch() || m_filter_expression.match(gate_name).hasMatch() || m_filter_expression.match(gate_id).hasMatch())
        return true;
    else
        return false;
}

void gate_table_proxy_model::handle_filter_text_changed(const QString& filter_text)
{
    m_filter_expression.setPattern(filter_text);
    invalidateFilter();
}

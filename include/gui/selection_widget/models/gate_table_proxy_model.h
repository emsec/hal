#pragma once

#include <QSortFilterProxyModel>
#include <QRegularExpression>

class gate_table_proxy_model : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    gate_table_proxy_model(QObject* parent = 0);
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

public Q_SLOTS:
    void handle_filter_text_changed(const QString& filter_text);

private:
    QRegularExpression m_filter_expression;
};

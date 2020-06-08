#pragma once

#include "def.h"

#include<QAbstractTableModel>

class net_table_model : public QAbstractTableModel
{
    Q_OBJECT

public:
    net_table_model(QObject* parent = 0);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& inddex, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void fetch_selection();

private:
    QVector<u32> m_selected_net_ids;
};

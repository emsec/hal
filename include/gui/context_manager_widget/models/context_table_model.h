#pragma once

#include "def.h"

#include <QAbstractTableModel>

#include "graph_widget/contexts/graph_context.h"

namespace hal
{
    class ContextTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        ContextTableModel(QObject* parent = 0);
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex & parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& inddex, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        GraphContext* get_context(const QModelIndex& index) const;
        QModelIndex get_index(GraphContext* context) const;

        void begin_insert_context(GraphContext* context);
        void end_insert_context();

        void begin_remove_context(GraphContext* context);
        void end_remove_context();

        void update(QVector<GraphContext*>* pointer);

    private:
        QVector<GraphContext*>* m_context_list;
    };
}
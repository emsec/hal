#include "gui/context_manager_widget/models/context_table_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    ContextTableModel::ContextTableModel(QObject* parent) : QAbstractTableModel(parent)
    {

    }

    int ContextTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return m_context_list->size();
    }

    int ContextTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 2;
    }

    QVariant ContextTableModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        if(index.row() >= m_context_list->size() || index.row() < 0)
            return QVariant();

        const GraphContext* context = m_context_list->at(index.row());

        if(!context)
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            switch(index.column())
            {
                case 0: return context->name(); break;
                case 1: return context->get_timestamp().toString(Qt::SystemLocaleShortDate); break;
                default: return QVariant();
            }
        }

        if(role == Qt::UserRole)
        {
            switch(index.column())
            {
                case 0: return context->name(); break;
                case 1: return context->get_timestamp(); break;
                default: return QVariant();
            }
        }

        return QVariant();
    }

    QVariant ContextTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
                case 0: return "View Name"; break;
                case 1: return "Timestamp"; break;
                default: QVariant();
            }
        }
        else if(orientation == Qt::Vertical)
        {
            return section + 1;
        }

        return QVariant();
    }

    void ContextTableModel::begin_insert_context(GraphContext* context)
    {
        Q_UNUSED(context)

        beginInsertRows(QModelIndex(), m_context_list->size(), m_context_list->size());
    }

    void ContextTableModel::end_insert_context()
    {
        endInsertRows();
    }

    void ContextTableModel::begin_remove_context(GraphContext* context)
    {
        const int row = m_context_list->indexOf(context);
        beginRemoveRows(QModelIndex(), row, row);
    }

    void ContextTableModel::end_remove_context()
    {
        endRemoveRows();
    }

    void ContextTableModel::update(QVector<GraphContext*>* pointer)
    {
        m_context_list = pointer;
    }

    GraphContext* ContextTableModel::get_context(const QModelIndex& index) const
    {
        return (*m_context_list)[index.row()];
    }

    QModelIndex ContextTableModel::get_index(GraphContext* context) const
    {
        return createIndex(m_context_list->indexOf(context), 0);
    }
}

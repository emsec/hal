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

        return mContextList->size();
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

        if(index.row() >= mContextList->size() || index.row() < 0)
            return QVariant();

        const GraphContext* context = mContextList->at(index.row());

        if(!context)
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            switch(index.column())
            {
                case 0: return context->name(); break;
                case 1: return context->getTimestamp().toString(Qt::SystemLocaleShortDate); break;
                default: return QVariant();
            }
        }

        if(role == Qt::UserRole)
        {
            switch(index.column())
            {
                case 0: return context->name(); break;
                case 1: return context->getTimestamp(); break;
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

    void ContextTableModel::beginInsertContext(GraphContext* context)
    {
        Q_UNUSED(context)

        beginInsertRows(QModelIndex(), mContextList->size(), mContextList->size());
    }

    void ContextTableModel::endInsertContext()
    {
        endInsertRows();
    }

    void ContextTableModel::beginRemoveContext(GraphContext* context)
    {
        const int row = mContextList->indexOf(context);
        beginRemoveRows(QModelIndex(), row, row);
    }

    void ContextTableModel::endRemoveContext()
    {
        endRemoveRows();
    }

    void ContextTableModel::update(QVector<GraphContext*>* pointer)
    {
        mContextList = pointer;
    }

    GraphContext* ContextTableModel::getContext(const QModelIndex& index) const
    {
        return (*mContextList)[index.row()];
    }

    QModelIndex ContextTableModel::getIndex(GraphContext* context) const
    {
        return createIndex(mContextList->indexOf(context), 0);
    }
}

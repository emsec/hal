#include "gui/selection_details_widget/net_details_widget/endpoint_table_model.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/net.h"

namespace hal
{

    EndpointTableModel::EndpointTableModel(Type type, QObject* parent) : QAbstractTableModel(parent), mType(type)
    {
    }

    void EndpointTableModel::clear()
    {
        beginResetModel();
        mEntries.clear();
        endResetModel();
    }

    int EndpointTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return mEntries.size();
    }

    int EndpointTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 4;
    }

    QVariant EndpointTableModel::data(const QModelIndex& index, int role) const
    {
        if (index.row() < mEntries.size() && index.column() < 4)
        {
            if (role == Qt::DisplayRole)
            {
                switch (index.column())
                {
                    case 0:
                        return mEntries[index.row()].name;
                    case 1:
                        return mEntries[index.row()].id;
                    case 2:
                        return mEntries[index.row()].type;
                    case 3:
                        return mEntries[index.row()].pin;
                }
            }
        }

        return QVariant();
    }

    QVariant EndpointTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
                case 0:
                    return "Name";
                case 1:
                    return "ID";
                case 2:
                    return "Type";
                case 3:
                    return "Pin";
                default:
                    return QVariant();
            }
        }

        return section + 1;
    }

    void EndpointTableModel::setNet(Net* net)
    {
        if (net == nullptr)
            return;

        mNetId = net->get_id();

        QList<Entry> newEntryList;

        std::vector<Endpoint*> v;

        if (mType == Type::source)
            v = net->get_sources();
        else
            v = net->get_destinations();

        for (Endpoint* e : v)
        {
            Entry newEntry;

            newEntry.name = QString::fromStdString(e->get_gate()->get_name());
            newEntry.id   = e->get_gate()->get_id();
            newEntry.type = QString::fromStdString(e->get_gate()->get_type()->get_name());
            newEntry.pin  = QString::fromStdString(e->get_pin()->get_name());

            newEntryList.append(newEntry);
        }

        beginResetModel();
        mEntries = newEntryList;
        endResetModel();
    }

    int EndpointTableModel::getCurrentNetID()
    {
        return mNetId;
    }

    u32 EndpointTableModel::getGateIDFromIndex(const QModelIndex& index)
    {
        return mEntries[index.row()].id;
    }

    QString EndpointTableModel::getPinNameFromIndex(const QModelIndex& index)
    {
        return mEntries.at(index.row()).pin;
    }

    QString EndpointTableModel::typeString()
    {
        if (mType == source)
            return "Source";
        else
            return "Destination";
    }

    EndpointTableModel::Type EndpointTableModel::getType()
    {
        return mType;
    }
}    // namespace hal

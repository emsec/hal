#include "gui/new_selection_details_widget/models/module_table_model.h"

#include "hal_core/netlist/net.h"

#include "gui/gui_globals.h"


namespace hal
{

    ModuleTableModel::ModuleTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
    }

    void ModuleTableModel::clear()
    {
        beginResetModel();
        mEntries.clear();
        endResetModel();
    }

    int ModuleTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return mEntries.size();
    }

    int ModuleTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 4;
    }

    QVariant ModuleTableModel::data(const QModelIndex& index, int role) const
    {
        if(index.row() < mEntries.size() && index.column() < 4)
        {
            if (role == Qt::DisplayRole)
            {
                switch (index.column())
                {
                case 0: return mEntries[index.row()].name;
                case 1: return mEntries[index.row()].id;
                case 2: return mEntries[index.row()].type;
                case 3: return mEntries[index.row()].used_port;
                }
            }
            else if (role == Qt::TextAlignmentRole)
            {
                return Qt::AlignCenter;
            }
        }

        return QVariant();
    }

    QVariant ModuleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0: return "Name";
            case 1: return "ID";
            case 2: return "Type";
            case 3: return "Used Port";
            default: return QVariant();
            }
        }

        return section + 1;
    }

    void ModuleTableModel::setNet(Net* net)
    {
        if(net == nullptr)
            return;

        mNetId = net->get_id();

        QList<Entry> newEntryList;

        for (Endpoint* e : net->get_sources())
        {
            Module* m = e->get_gate()->get_module();

            Entry newEntry;

            newEntry.name = QString::fromStdString(m->get_name());
            newEntry.id = m->get_id();
            newEntry.type = QString::fromStdString(m->get_type());
            newEntry.used_port = QString::fromStdString(m->get_output_port_name(net));

            newEntryList.append(newEntry);
        }

        for (Endpoint* e : net->get_destinations())
        {
            Module* m = e->get_gate()->get_module();

            Entry newEntry;

            newEntry.name = QString::fromStdString(m->get_name());
            newEntry.id = m->get_id();
            newEntry.type = QString::fromStdString(m->get_type());
            newEntry.used_port = QString::fromStdString(m->get_input_port_name(net));

            newEntryList.append(newEntry);
        }

        beginResetModel();
        mEntries = newEntryList;
        endResetModel();
    }

    int ModuleTableModel::getCurrentNetID()
    {
        return mNetId;
    }

    u32 ModuleTableModel::getModuleIDFromIndex(const QModelIndex& index)
    {
        return mEntries[index.row()].id;
    }

    QString ModuleTableModel::getPortNameFromIndex(const QModelIndex& index)
    {
        return mEntries[index.row()].used_port;
    }
}

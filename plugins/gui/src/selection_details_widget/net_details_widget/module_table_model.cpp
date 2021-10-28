#include "gui/selection_details_widget/net_details_widget/module_table_model.h"

#include "hal_core/netlist/net.h"

#include "gui/gui_globals.h"
#include <QSet>
#include <QDebug>


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

        //1. variant: simple brute force through all modules
        //2. variant: more complex algorithm:
        //  use net sources/destinations, crawl through parent-modules, keep track of already
        //  visited modules to not list modules (ports) twice

        //1.variant
        for(auto const &m : gNetlist->get_modules())
        {
            for(auto port : m->get_ports())
            {
                if(port->contains_net(net))
                {
                    Entry newEntry;

                    newEntry.name = QString::fromStdString(m->get_name());
                    newEntry.id = m->get_id();
                    newEntry.type = QString::fromStdString(m->get_type());
                    newEntry.used_port = QString::fromStdString(port->get_name());//change to pin?

                    newEntryList.append(newEntry);
                }
            }
        }
//            auto portMap = m->get_input_port_names();
//            auto it = portMap.find(net);
//            if(it != portMap.end())
//            {
//                Entry newEntry;

//                newEntry.name = QString::fromStdString(m->get_name());
//                newEntry.id = m->get_id();
//                newEntry.type = QString::fromStdString(m->get_type());
//                newEntry.used_port = QString::fromStdString(it->second);

//                newEntryList.append(newEntry);

//            }

//            portMap = m->get_output_port_names();
//            it = portMap.find(net);
//            if(it != portMap.end())
//            {
//                Entry newEntry;

//                newEntry.name = QString::fromStdString(m->get_name());
//                newEntry.id = m->get_id();
//                newEntry.type = QString::fromStdString(m->get_type());
//                newEntry.used_port = QString::fromStdString(it->second);

//                newEntryList.append(newEntry);

//            }
//        }

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

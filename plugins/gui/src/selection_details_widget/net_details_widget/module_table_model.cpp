#include "gui/selection_details_widget/net_details_widget/module_table_model.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <QDebug>

namespace hal
{
    ModuleTableModel::ModuleTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
        //connections
        connect(gNetlistRelay, &NetlistRelay::modulePortsChanged, this, &ModuleTableModel::handleModulePortsChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleRemoved, this, &ModuleTableModel::handleModuleRemoved);
    }

    void ModuleTableModel::clear()
    {
        beginResetModel();
        mEntries.clear();
        endResetModel();

        mModIds.clear();
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
                        return mEntries[index.row()].used_port;
                }
            }
        }

        return QVariant();
    }

    QVariant ModuleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                    return "Used Port";
                default:
                    return QVariant();
            }
        }

        return section + 1;
    }

    void ModuleTableModel::setNet(Net* net)
    {
        if (net == nullptr)
            return;

        mNetId = net->get_id();
        mModIds.clear();
        QList<Entry> newEntryList;

        //1. variant: simple brute force through all modules
        //2. variant: more complex algorithm:
        //  use net sources/destinations, crawl through parent-modules, keep track of already
        //  visited modules to not list modules (ports) twice

        //1.variant
        for (auto const& m : gNetlist->get_modules())
        {
            if (const auto res = m->get_pin_by_net(net); res.is_ok())
            {
                Entry newEntry;

                newEntry.name      = QString::fromStdString(m->get_name());
                newEntry.id        = m->get_id();
                newEntry.type      = QString::fromStdString(m->get_type());
                newEntry.used_port = QString::fromStdString(res.get()->get_name());

                newEntryList.append(newEntry);
                mModIds.insert((int)m->get_id());
            }
        }

        beginResetModel();
        mEntries = newEntryList;
        endResetModel();

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

    void ModuleTableModel::handleModulePortsChanged(Module* m)
    {
        if (mModIds.find((int)m->get_id()) != mModIds.end())
        {
            Net* net = gNetlist->get_net_by_id(mNetId);
            if (net != nullptr)
            {
                std::unordered_set<Net*> inputNets  = m->get_input_nets();
                std::unordered_set<Net*> outputNets = m->get_output_nets();
                if (inputNets.find(net) != inputNets.end() || outputNets.find(net) != outputNets.end())
                {
                    setNet(net);
                }
            }
        }
    }

    void ModuleTableModel::handleModuleRemoved(Module* m)
    {
        if (mModIds.find((int)m->get_id()) != mModIds.end())
        {
            Net* net = gNetlist->get_net_by_id(mNetId);
            if (net != nullptr)
            {
                setNet(net);
            }
        }
    }
}    // namespace hal

#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"
#include "gui/basic_tree_model/tree_item.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "gui/gui_globals.h"

namespace hal
{
    PortTreeModel::PortTreeModel(QObject *parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QList<QVariant>() << "Name" << "Direction" << "Type" << "Connected Net");
        setModule(gNetlist->get_module_by_id(1));

        //connections
        //connect(gNetlistRelay, &NetlistRelay::moduleInputPortNameChanged, this, &PortTreeModel::handleModuleInputOutputPortNameChanged);
        //connect(gNetlistRelay, &NetlistRelay::moduleOutputPortNameChanged, this, &PortTreeModel::handleModuleInputOutputPortNameChanged);
    }

    PortTreeModel::~PortTreeModel()
    {
        delete mRootItem;
    }

    void PortTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleId = -1;
        mPortGroupingToTreeItem.clear();
    }

    void PortTreeModel::setModule(Module *m)
    {
        clear();
        mModuleId = m->get_id();
        //if a port is within a group, this helper-map maps the items to their port index
        QMap<TreeItem*, u32> portItemToIndex;
        beginResetModel();

        for(auto port : m->get_ports())
        {
            QString portName = QString::fromStdString(port->get_name());
            QString portDirection = QString::fromStdString(enum_to_string(port->get_direction()));
            QString portType = QString::fromStdString(enum_to_string(port->get_type()));
            QString netName = port->get_net() ? QString::fromStdString(port->get_net()->get_name()) : "";

            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << portDirection << portType << netName);

            if(!port->get_group_name().empty())
            {
                portItemToIndex.insert(portItem, port->get_group_index());
                TreeItem* groupingsItem = mPortGroupingToTreeItem.value(port->get_group_name(), nullptr);
                if(!groupingsItem)
                {
                    groupingsItem = new TreeItem(QList<QVariant>() << QString::fromStdString(port->get_group_name()) << "" << "" << "");
                    groupingsItem->setAdditionalData(keyType, QVariant::fromValue(itemType::grouping));
                    mRootItem->appendChild(groupingsItem);
                    mPortGroupingToTreeItem.insert(port->get_group_name(), groupingsItem);
                }
                groupingsItem->appendChild(portItem);
            }
            else
                mRootItem->appendChild(portItem);

        }
        //sort ports within all groupingitems (easy way)
        for(auto it = mPortGroupingToTreeItem.begin(); it != mPortGroupingToTreeItem.end(); it++)
        {
            QMap<u32, TreeItem*> tmpSortedPortItems;
            TreeItem* groupingsItem = it.value();
            while(groupingsItem->getChildCount() > 0)
            {
                TreeItem* currPortItem = groupingsItem->removeChildAtPos(0);
                tmpSortedPortItems.insert(portItemToIndex.value(currPortItem), currPortItem);
            }

            for(auto portItem : tmpSortedPortItems)
                groupingsItem->appendChild(portItem);
        }
        endResetModel();

        Q_EMIT numberOfPortsChanged(m->get_ports().size());
    }

    Net* PortTreeModel::getNetFromItem(TreeItem *item)
    {
        if(mModuleId == -1) //no current module = no represented net
            return nullptr;

        Module* m = gNetlist->get_module_by_id(mModuleId);
        if(!m) return nullptr;

        return m->get_port_by_name(item->getData(sNameColumn).toString().toStdString())->get_net();
    }

    int PortTreeModel::getRepresentedModuleId()
    {
        return mModuleId;
    }

    void PortTreeModel::handleModuleInputOutputPortNameChanged(Module *m, int associated_data)
    {
        Q_UNUSED(associated_data)

        if((int)m->get_id() == mModuleId)
        {
            setModule(m);
            Q_EMIT numberOfPortsChanged(m->get_ports().size());
        }
    }



}

#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"

#include "gui/basic_tree_model/tree_item.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/enums.h"
#include <QDebug>

namespace hal
{
    PortTreeModel::PortTreeModel(QObject* parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QList<QVariant>() << "Name"
                                          << "Direction"
                                          << "Type"
                                          << "Connected Net");
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

    void PortTreeModel::setModule(Module* m)
    {
        clear();
        mModuleId = m->get_id();
        beginResetModel();

        for (auto port : m->get_ports())
        {
            QString portName = QString::fromStdString(port->get_name());
            QString portDirection = QString ::fromStdString(enum_to_string(port->get_direction()));
            QString portType = QString::fromStdString(enum_to_string(port->get_type()));
            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << portDirection << portType << "");

            if(!port->is_multi_bit())
            {
                portItem->setDataAtIndex(sNetColumn, QString::fromStdString(port->get_nets().at(0)->get_name()));
                portItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portSingleBit));
            }
            else
            {
                portItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portMultiBit));
                for(auto pinNetPair : port->get_pins_and_nets()) //pair str,net
                {
                    TreeItem* pinItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pinNetPair.first)
                                                     << portDirection << portType << QString::fromStdString(pinNetPair.second->get_name()));
                    pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                    portItem->appendChild(pinItem);
                }
            }
            mRootItem->appendChild(portItem);
        }
        endResetModel();

        Q_EMIT numberOfPortsChanged(m->get_ports().size());
    }

    Net* PortTreeModel::getNetFromItem(TreeItem* item)
    {
        if (mModuleId == -1)    //no current module = no represented net
            return nullptr;

        itemType type = getTypeOfItem(item);
        if (type == itemType::portMultiBit)
            return nullptr;

        Module* m = gNetlist->get_module_by_id(mModuleId);
        if (!m)
            return nullptr;

        std::string name = item->getData(sNameColumn).toString().toStdString();
        auto port = (type == itemType::portSingleBit) ? m->get_port(name) : m->get_port_by_pin_name(name);
        auto it = std::find_if(port->get_pins_and_nets().begin(), port->get_pins_and_nets().end(),[&name](const std::pair<std::string, Net*> e){return e.first == name;});
        if(it != port->get_pins_and_nets().end())
            return it->second;
        else
            return nullptr;
    }

    int PortTreeModel::getRepresentedModuleId()
    {
        return mModuleId;
    }

    PortTreeModel::itemType PortTreeModel::getTypeOfItem(TreeItem* item)
    {
        return item->getAdditionalData(keyType).value<itemType>();
    }

    void PortTreeModel::handleModuleInputOutputPortNameChanged(Module* m, int associated_data)
    {
        Q_UNUSED(associated_data)

        if ((int)m->get_id() == mModuleId)
        {
            setModule(m);
            Q_EMIT numberOfPortsChanged(m->get_ports().size());
        }
    }

}    // namespace hal

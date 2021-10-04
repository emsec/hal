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
        connect(gNetlistRelay, &NetlistRelay::moduleInputPortNameChanged, this, &PortTreeModel::handleModuleInputOutputPortNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleOutputPortNameChanged, this, &PortTreeModel::handleModuleInputOutputPortNameChanged);
    }

    PortTreeModel::~PortTreeModel()
    {
        delete mRootItem;
    }

    void PortTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleId = -1;
    }

    void PortTreeModel::setModule(Module *m)
    {
        clear();
        //until port groups are implemented, simply display all input and then all output ports
        mModuleId = m->get_id();
        beginResetModel();
        for(const auto& [key, value] : m->get_input_port_names())//key=net, value=string
        {
            QString portName = QString::fromStdString(value);
            QString portDirection = "input";
            //get the type from the pin.. since it can be multiple source pins connected to the net, pick the first
            //Endpoint* ep = key ? key->get_sources().at(0) : nullptr;
            //QString type = (key && ep) ? QString::fromStdString(enum_to_string(ep->get_gate()->get_type()->get_pin_type(ep->get_pin()))) : "";
            QString netName = key ? QString::fromStdString(key->get_name()) : "";
            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << "input(placeholder)" << "N/A" << netName);
            mRootItem->appendChild(portItem);
        }

        for(const auto& [key, value] : m->get_output_port_names())//key=net, value=string
        {
            QString portName = QString::fromStdString(value);
            QString portDirection = "output";
            //get the type from the pin.. since it can be multiple source pins connected to the net, pick the first
            //Endpoint* ep = key ? key->get_sources().at(0) : nullptr;
            //QString type = (key && ep) ? QString::fromStdString(enum_to_string(ep->get_gate()->get_type()->get_pin_type(ep->get_pin()))) : "";
            QString netName = key ? QString::fromStdString(key->get_name()) : "";
            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << "output(placeholder)" << "N/A" << netName);
            mRootItem->appendChild(portItem);
        }
        endResetModel();

        Q_EMIT numberOfPortsChanged((m->get_input_port_names().size() + m->get_output_port_names().size()));
    }

    Net* PortTreeModel::getNetFromItem(TreeItem *item)
    {
        if(mModuleId == -1) //no current module = no represented net
            return nullptr;

        Module* m = gNetlist->get_module_by_id(mModuleId);
        if(!m) return nullptr;

        if(item->getData(sDirectionColumn).toString() == "input(placeholder)")//dirty input direction checking
            return m->get_input_port_net(item->getData(sNameColumn).toString().toStdString());
        else
            return m->get_output_port_net(item->getData(sNameColumn).toString().toStdString());
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
            Q_EMIT numberOfPortsChanged((m->get_input_port_names().size() + m->get_output_port_names().size()));
        }
    }



}

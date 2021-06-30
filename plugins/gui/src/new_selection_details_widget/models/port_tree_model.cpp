#include "gui/new_selection_details_widget/models/port_tree_model.h"
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
    }

    PortTreeModel::~PortTreeModel()
    {
        delete mRootItem;
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
            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << portDirection << "" << netName);
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
            TreeItem* portItem = new TreeItem(QList<QVariant>() << portName << portDirection << "" << netName);
            mRootItem->appendChild(portItem);
        }
        endResetModel();
    }



}

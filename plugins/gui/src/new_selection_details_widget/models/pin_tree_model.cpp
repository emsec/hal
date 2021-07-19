#include "gui/new_selection_details_widget/models/pin_tree_model.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "gui/gui_globals.h"
#include <QVector>
#include <QDebug>
#include "hal_core/utilities/enums.h"

namespace hal
{

    PinTreeModel::PinTreeModel(QObject *parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QList<QVariant>() << "Name" << "Direction" << "Type" << "Connected Net");
        setGate(gNetlist->get_gate_by_id(19));
    }

    PinTreeModel::~PinTreeModel()
    {
        delete mRootItem;

    }

    void PinTreeModel::clear()
    {
        BaseTreeModel::clear();
        mPinGroupingToTreeItem.clear();
        mGateId = -1;

    }

    void PinTreeModel::setGate(Gate *g)
    {
        clear();

        beginResetModel();
        GateType* gateType = g->get_type();
        for(auto pin : gateType->get_pins())
        {
            TreeItem* pinItem = new TreeItem();
            //get all infos for that pin
            std::string grouping = gateType->get_pin_group(pin);
            QString pinDirection = QString::fromStdString(enum_to_string(gateType->get_pin_direction(pin)));
            QString pinType = QString::fromStdString(enum_to_string(gateType->get_pin_type(pin)));
            //evaluate netname (in case of inout multiple possible nets), method depends on pindirection (kind of ugly switch)
            QString netName;
            switch(gateType->get_pin_direction(pin))
            {
                case PinDirection::input:
                    netName = (g->get_fan_in_net(pin)) ? QString::fromStdString(g->get_fan_in_net(pin)->get_name()) : "" ; break;
                case PinDirection::output:
                    netName = (g->get_fan_out_net(pin)) ? QString::fromStdString(g->get_fan_out_net(pin)->get_name()) : "" ; break;
                case PinDirection::inout: //must take input and output net into account
                    if(g->get_fan_in_net(pin)) netName += QString::fromStdString(g->get_fan_in_net(pin)->get_name());
                    if(g->get_fan_out_net(pin))
                    {
                        if(!netName.isEmpty()) netName += " / "; //add / when there is a input net to seperate it from the output net
                        netName += QString::fromStdString(g->get_fan_out_net(pin)->get_name());
                    }
                    break;
                default: break; //none and internal, dont know how to handle internal (whatever an internal pin is)
            }

            pinItem->setData(QList<QVariant>() << QString::fromStdString(pin) << pinDirection << pinType << netName);
            pinItem->setAdditionalData(keyType, itemType::pin);
            if(!grouping.empty())
            {
                TreeItem* groupingsItem = mPinGroupingToTreeItem.value(grouping, nullptr); //since its a map, its okay
                if(!groupingsItem)
                {
                    //assume all items in the same grouping habe the same direction and type, so the grouping-item has also these types
                    groupingsItem = new TreeItem(QList<QVariant>() << QString::fromStdString(grouping) << pinDirection << pinType << "");
                    groupingsItem->setAdditionalData(keyType, itemType::grouping);
                    mRootItem->appendChild(groupingsItem);
                    mPinGroupingToTreeItem.insert(grouping, groupingsItem);
                }
                groupingsItem->appendChild(pinItem);

            }
            else
                mRootItem->appendChild(pinItem);

        }
        endResetModel();
    }

    int PinTreeModel::getCurrentGateID()
    {
        return mGateId;
    }

    void PinTreeModel::appendInputOutputPins(Gate* g, std::vector<std::string> inputOrOutputPins, bool areInputPins)
    {
        GateType* gateType = g->get_type();
        QVector<TreeItem*> appendAtTheEnd;
        for(auto pin : inputOrOutputPins)
        {
            std::string grouping = gateType->get_pin_group(pin); //is ok, underlying structure is a unordered_map->fast
            Net* connectedNet = areInputPins ? g->get_fan_in_net(pin) : g->get_fan_out_net(pin);
            QString netName = (connectedNet) ? QString::fromStdString(connectedNet->get_name()) : "";
            QString pinType = QString::fromStdString(enum_to_string(gateType->get_pin_type(pin)));
            QString pinDirection = QString::fromStdString(enum_to_string(gateType->get_pin_direction(pin)));
            TreeItem* currentPinItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pin) << pinDirection << pinType << netName);
            currentPinItem->setAdditionalData(keyType, itemType::pin);
            //input pin has no grouping, put at the end
            if(grouping.empty())
            {
                appendAtTheEnd.append(currentPinItem);
            }
            else
            {
                TreeItem* groupingsItem = mPinGroupingToTreeItem.value(grouping, nullptr); //since its a map, its okay
                if(!groupingsItem)
                {
                    //assume all items in the same grouping habe the same direction and type, so the grouping-item has also these types
                    groupingsItem = new TreeItem(QList<QVariant>() << QString::fromStdString(grouping) << pinDirection << pinType << "");
                    groupingsItem->setAdditionalData(keyType, itemType::grouping);
                    mRootItem->appendChild(groupingsItem);
                    mPinGroupingToTreeItem.insert(grouping, groupingsItem);
                }
                groupingsItem->appendChild(currentPinItem);
            }
        }
        for(auto pinItem : appendAtTheEnd)
            mRootItem->appendChild(pinItem);
    }

}

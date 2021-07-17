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

    void PinTreeModel::setGate(Gate *g)
    {
        //1. display all input grouping items at the beginning, try to keep order by iterate over input-pins instead of groupings
        //2. display all input pins not belonging to any group after that
        //3. thereafter do the same for output pins

        clear();
        mPinGroupingToTreeItem.clear();
        mGateId = g->get_id();

        beginResetModel();
        appendInputOutputPins(g, g->get_input_pins(), true);
        appendInputOutputPins(g, g->get_output_pins(), false);

        endResetModel();
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

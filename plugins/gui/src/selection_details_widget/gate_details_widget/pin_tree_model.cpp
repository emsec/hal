#include "gui/selection_details_widget/gate_details_widget/pin_tree_model.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/enums.h"

#include <QDebug>
#include <QVector>

namespace hal
{

    GatePinsTreeModel::GatePinsTreeModel(QObject* parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QList<QVariant>() << "Name"
                                          << "Direction"
                                          << "Type"
                                          << "Connected Net");

        //added to store a list of (multiple) net ids in a given treeitem (perhaps dont do this
        //at all, handle it in the view? (since the gate-id and pin name is accessable, the nets can be evaluated there
        qRegisterMetaType<QList<int>>();
    }

    GatePinsTreeModel::~GatePinsTreeModel()
    {
        delete mRootItem;
    }

    void GatePinsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mPinGroupingToTreeItem.clear();
        mGateId = -1;
    }

    void GatePinsTreeModel::setGate(Gate* g)
    {
        clear();
        mGateId = g->get_id();

        beginResetModel();
        GateType* gateType = g->get_type();
        for (auto pin : gateType->get_pins())
        {
            BaseTreeItem* pinItem = new BaseTreeItem();
            //get all infos for that pin
            const std::string& grouping = pin->get_group().first->get_name();
            PinDirection direction      = pin->get_direction();
            QString pinDirection        = QString::fromStdString(enum_to_string(direction));
            QString pinType             = QString::fromStdString(enum_to_string(pin->get_type()));

            //evaluate netname (in case of inout multiple possible nets), method depends on pindirection (kind of ugly switch)
            QString netName = "";
            QList<int> netIDs;
            switch (direction)
            {
                case PinDirection::input:
                    if (g->get_fan_in_net(pin))
                    {
                        netName = QString::fromStdString(g->get_fan_in_net(pin)->get_name());
                        netIDs.append(g->get_fan_in_net(pin)->get_id());
                    }
                    break;
                    //netName = (g->get_fan_in_net(pin)) ? QString::fromStdString(g->get_fan_in_net(pin)->get_name()) : "" ; break;
                case PinDirection::output:
                    if (g->get_fan_out_net(pin))
                    {
                        netName = QString::fromStdString(g->get_fan_out_net(pin)->get_name());
                        netIDs.append(g->get_fan_out_net(pin)->get_id());
                    }
                    break;
                    //netName = (g->get_fan_out_net(pin)) ? QString::fromStdString(g->get_fan_out_net(pin)->get_name()) : "" ; break;
                case PinDirection::inout:    //must take input and output net into account
                    if (g->get_fan_in_net(pin))
                    {
                        netName += QString::fromStdString(g->get_fan_in_net(pin)->get_name());
                        netIDs.append(g->get_fan_in_net(pin)->get_id());
                    }
                    if (g->get_fan_out_net(pin))
                    {
                        if (!netName.isEmpty())
                            netName += " / ";    //add / when there is a input net to seperate it from the output net
                        netName += QString::fromStdString(g->get_fan_out_net(pin)->get_name());
                        netIDs.append(g->get_fan_out_net(pin)->get_id());
                    }
                    break;
                default:
                    break;    //none and internal, dont know how to handle internal (whatever an internal pin is)
            }

            pinItem->setData(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinDirection << pinType << netName);
            pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
            pinItem->setAdditionalData(keyRepresentedNetsID, QVariant::fromValue(netIDs));
            if (!grouping.empty())
            {
                BaseTreeItem* groupingsItem = mPinGroupingToTreeItem.value(grouping, nullptr);    //since its a map, its okay
                if (!groupingsItem)
                {
                    //assume all items in the same grouping habe the same direction and type, so the grouping-item has also these types
                    groupingsItem = new BaseTreeItem(QList<QVariant>() << QString::fromStdString(grouping) << pinDirection << pinType << "");
                    groupingsItem->setAdditionalData(keyType, QVariant::fromValue(itemType::grouping));
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

    int GatePinsTreeModel::getCurrentGateID()
    {
        return mGateId;
    }

    QList<int> GatePinsTreeModel::getNetIDsOfTreeItem(BaseTreeItem* item)
    {
        return item->getAdditionalData(keyRepresentedNetsID).value<QList<int>>();
    }

    GatePinsTreeModel::itemType GatePinsTreeModel::getTypeOfItem(BaseTreeItem* item)
    {
        return item->getAdditionalData(keyType).value<itemType>();
    }

    int GatePinsTreeModel::getNumberOfDisplayedPins()
    {
        Gate* g = gNetlist->get_gate_by_id(mGateId);
        if (!g)
            return 0;

        return g->get_type()->get_pins().size();
    }
}    // namespace hal

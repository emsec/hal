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
            TreeItem* pinItem = new TreeItem();
            //get all infos for that pin
            PinGroup<GatePin>* pinGroup = pin->get_group().first;
            QString pinDirection        = QString::fromStdString(enum_to_string(pin->get_direction()));
            QString pinType             = QString::fromStdString(enum_to_string(pin->get_type()));

            //evaluate netname (in case of inout multiple possible nets), method depends on pindirection (kind of ugly switch)
            QString netName = "";
            QList<int> netIDs;
            switch (pin->get_direction())
            {
                case PinDirection::input:
                    if (auto res = g->get_fan_in_net(pin); res.is_ok())
                    {
                        Net* fan_in_net = res.get();
                        netName         = QString::fromStdString(fan_in_net->get_name());
                        netIDs.append(fan_in_net->get_id());
                    }
                    break;
                case PinDirection::output:
                    if (auto res = g->get_fan_out_net(pin); res.is_ok())
                    {
                        Net* fan_out_net = res.get();
                        netName          = QString::fromStdString(fan_out_net->get_name());
                        netIDs.append(fan_out_net->get_id());
                    }
                    break;
                case PinDirection::inout:    //must take input and output net into account
                    if (auto res = g->get_fan_in_net(pin); res.is_ok())
                    {
                        Net* fan_in_net = res.get();
                        netName         = QString::fromStdString(fan_in_net->get_name());
                        netIDs.append(fan_in_net->get_id());
                    }
                    if (auto res = g->get_fan_out_net(pin); res.is_ok())
                    {
                        if (!netName.isEmpty())
                        {
                            netName += " / ";    //add / when there is a input net to seperate it from the output net
                        }
                        Net* fan_out_net = res.get();
                        netName += QString::fromStdString(fan_out_net->get_name());
                        netIDs.append(fan_out_net->get_id());
                    }
                    break;
                default:
                    break;    //none and internal
            }

            pinItem->setData(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinDirection << pinType << netName);
            pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
            pinItem->setAdditionalData(keyRepresentedNetsID, QVariant::fromValue(netIDs));
            if (pinGroup != nullptr)
            {
                const std::string& pinGroupName = pinGroup->get_name();
                TreeItem* groupingsItem         = mPinGroupingToTreeItem.value(pinGroupName, nullptr);    //since its a map, its okay
                if (!groupingsItem)
                {
                    //assume all items in the same grouping habe the same direction and type, so the grouping-item has also these types
                    groupingsItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pinGroupName) << QString::fromStdString(enum_to_string(pinGroup->get_direction()))
                                                                   << QString::fromStdString(enum_to_string(pinGroup->get_type())) << "");
                    groupingsItem->setAdditionalData(keyType, QVariant::fromValue(itemType::grouping));
                    mRootItem->appendChild(groupingsItem);
                    mPinGroupingToTreeItem.insert(pinGroupName, groupingsItem);
                }
                groupingsItem->appendChild(pinItem);
            }
            else
            {
                mRootItem->appendChild(pinItem);
            }
        }
        endResetModel();
    }

    int GatePinsTreeModel::getCurrentGateID()
    {
        return mGateId;
    }

    QList<int> GatePinsTreeModel::getNetIDsOfTreeItem(TreeItem* item)
    {
        return item->getAdditionalData(keyRepresentedNetsID).value<QList<int>>();
    }

    GatePinsTreeModel::itemType GatePinsTreeModel::getTypeOfItem(TreeItem* item)
    {
        return item->getAdditionalData(keyType).value<itemType>();
    }

    int GatePinsTreeModel::getNumberOfDisplayedPins()
    {
        Gate* g = gNetlist->get_gate_by_id(mGateId);
        if (g == nullptr)
        {
            return 0;
        }

        return g->get_type()->get_pins().size();
    }
}    // namespace hal

#include "gui/selection_details_widget/gate_details_widget/gate_pins_tree_model.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"

#include <QDebug>
#include <QVector>

namespace hal
{

    GatePinsTreeItem::GatePinsTreeItem(const std::string &pinName, QString pinDirection, QString pinType, QString netName, int inx)
        :mPinName(pinName), mPinDirection(pinDirection), mPinType(pinType), mNetName(netName), mIndex(inx)
    {;}

    GatePinsTreeItem::GatePinsTreeItem()
    {;}

    QVariant GatePinsTreeItem::getData(int index) const
    {
        switch (index)
        {
        case 0:
            return QString::fromStdString(mPinName);
        case 1:
            return mPinDirection;
        case 2:
            return mPinType;
        case 3:
            return mNetName;
        case 4:
            if (mType == GatePinsTreeItem::Group)
                return (mIndex ? "descending" : "ascending");
            return mIndex;
        }
        return QVariant();
    }

    void GatePinsTreeItem::setData(QList<QVariant> data)
    {
        Q_ASSERT(data.size() >= 5);
        mPinName = data[0].toString().toStdString();
        mPinDirection = data[1].toString();
        mPinType = data[2].toString();
        mNetName = data[3].toString();
        mIndex = data[4].toInt();
    }

    void GatePinsTreeItem::setDataAtColumn(int column, QVariant &data)
    {
        switch (column)
        {
        case 0:
            mPinName = data.toString().toStdString();
            break;
        case 1:
            mPinDirection = data.toString();
            break;
        case 2:
            mPinType = data.toString();
            break;
        case 3:
            mNetName = data.toString();
            break;
        case 4:
            mIndex = data.toInt();
            break;
        }


    }

    void GatePinsTreeItem::appendData(QVariant data) {}

    int GatePinsTreeItem::getColumnCount() const
    {
        return 5;
    }

    GatePinsTreeModel::GatePinsTreeModel(QObject* parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QStringList() << "Name"
                                      << "Direction"
                                      << "Type"
                                      << "Connected Net"
                                      << "Index");

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
        mPinGroupToTreeItem.clear();
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
            GatePinsTreeItem* pinItem = new GatePinsTreeItem();
            //get all infos for that pin
            const PinGroup<GatePin>* pg = pin->get_group().first;
            const std::string& grpName  = pg->get_name();
            int iDescending             = pg->is_ascending() ? 0 : 1;
            int inx                     = pin->get_group().second;
            PinDirection direction      = pin->get_direction();
            QString pinDirection        = QString::fromStdString(enum_to_string(direction));
            QString pinType             = QString::fromStdString(enum_to_string(pin->get_type()));

            //evaluate netname (in case of inout multiple possible nets), method depends on pindirection (kind of ugly switch)
            QString netName = "";
            QList<u32> netIDs;
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

            pinItem->setData(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinDirection << pinType << netName << inx);
            pinItem->setType(GatePinsTreeItem::Pin);
            pinItem->setNetIds(netIDs);
            if (!grpName.empty())
            {
                GatePinsTreeItem* pingroupItem = dynamic_cast<GatePinsTreeItem*>(mPinGroupToTreeItem.value(grpName, nullptr));    //since its a map, its okay
                if (!pingroupItem)
                {
                    //assume all items in the same grouping habe the same direction and type, so the grouping-item has also these types
                    pingroupItem = new GatePinsTreeItem(grpName, pinDirection, pinType, "", iDescending);
                    pingroupItem->setType(GatePinsTreeItem::Group);
                    mRootItem->appendChild(pingroupItem);
                    mPinGroupToTreeItem.insert(grpName, pingroupItem);
                }
                pingroupItem->appendChild(pinItem);
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

    int GatePinsTreeModel::getNumberOfDisplayedPins()
    {
        Gate* g = gNetlist->get_gate_by_id(mGateId);
        if (!g)
            return 0;

        return g->get_type()->get_pins().size();
    }



}    // namespace hal

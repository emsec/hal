#include "gui/pin_model/pin_model.h"
#include "gui/pin_model/pin_item.h"

#include "gui/gui_globals.h"

#include "gui/selection_details_widget/selection_details_icon_provider.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    PinModel::PinModel(QObject* parent) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "Direction" << "Type" << "TODO");
    }

    Qt::ItemFlags PinModel::flags(const QModelIndex& index) const
    {
        return Qt::ItemIsEditable | BaseTreeModel::flags(index);
    }

    void PinModel::setGate(GateType* gate){

        //TODO change font of dummy entries
        clear();
        mGate = gate;

        beginResetModel();
        for(auto group : mGate->get_pin_groups()){
            auto groupItem = new PinItem(PinItem::TreeItemType::PinGroup);
            //create group
            groupItem->setData(QList<QVariant>() << group->get_id() << QString::fromStdString(group->get_name()));

            for (auto pin : group->get_pins())
            {
                auto pinItem = new PinItem(PinItem::TreeItemType::Pin);

                //get all infos for that pin
                const std::string& grouping = pin->get_group().first->get_name();
                PinDirection direction      = pin->get_direction();
                QString pinDirection        = QString::fromStdString(enum_to_string(direction));
                QString pinType             = QString::fromStdString(enum_to_string(pin->get_type()));

                pinItem->setData(QList<QVariant>() << pin->get_id() << QString::fromStdString(pin->get_name()) << pinDirection << pinType);

                groupItem->appendChild(pinItem);
            }

            //add dummy to each group
            auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
            dummyPin->setData(QList<QVariant>() << "create new pin ...");
            groupItem->appendChild(dummyPin);
            mRootItem->appendChild(groupItem);
        }
        //create dummy for the group creator
        auto dummyGroup = new PinItem(PinItem::TreeItemType::GroupCreator);
        dummyGroup->setData(QList<QVariant>() << "create new pingroup ...");
        mRootItem->appendChild(dummyGroup);
        endResetModel();
    }

    void PinModel::addPin()
    {
        //TODO create new pin
    }

    void PinModel::addPinGroup(QString groupName)
    {
        //TODO generate pin group id
    }

    void PinModel::addPinToPinGroup(u32 pinId, u32 groupId)
    {
        GatePin* pin = mGate->get_pin_by_id(pinId);
        if(!pin)
            return;
        mGate->get_pin_group_by_id(groupId)->assign_pin(pin);
    }

    void PinModel::handleEdit(QModelIndex index, QString input)
    {
        //TODO handle text edited from PinDelegate
        auto castTest = static_cast<PinItem*>(index.internalPointer());
        auto itemType = castTest->getItemType();
        switch(itemType){
            case PinItem::TreeItemType::PinGroup:{
                qInfo() << "was Group: " << castTest->getName();
                break;
            }
            case PinItem::TreeItemType::Pin:{
                qInfo() << "was Pin: " << castTest->getName();

                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                qInfo() << "was GroupCreator: " << castTest->getName();

                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << castTest->getName();

                break;
            }
        }
    }

}

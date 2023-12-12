#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

#include <gui/pin_model/pin_model.h>

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

    void PinModel::handleEditName(QModelIndex index, QString input)
    {
        //TODO handle text edited from PinDelegate
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch(itemType){
            case PinItem::TreeItemType::PinGroup: {
                qInfo() << "was Group: " << pinItem->getName();
                if (renamePinGroup(pinItem->id(), input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::Pin:{
                qInfo() << "was Pin: " << pinItem->getName();
                if(renamePin(pinItem->id(), input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                qInfo() << "was GroupCreator: " << pinItem->getName();

                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << pinItem->getName();

                // creates a new pin which is not valid until now nor created via gate->create_pin()

                pinItem->setFields(input, 0, PinDirection::none, PinType::none);
                pinItem->setItemType(PinItem::TreeItemType::InvalidPin);

                beginInsertRows(index.parent(),0,0);
                //add new dummy after entry
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->getParent()->appendChild(dummyPin);
                endInsertRows();

                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                //TODO validate if pin is now valid and if so then create it via gate->create_pin(...)
                qInfo() << "was InvalidPin: " << pinItem->getName();
                pinItem->setName(input);
                break;
            }
        }
    }

    void PinModel::handleEditDirection(QModelIndex index, QString direction)
    {
        //TODO handle text edited from PinDelegate
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch(itemType){
            case PinItem::TreeItemType::PinGroup: {
                qInfo() << "was Group: " << pinItem->getName();

                break;
            }
            case PinItem::TreeItemType::Pin:{
                qInfo() << "was Pin: " << pinItem->getName();

                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                qInfo() << "was GroupCreator: " << pinItem->getName();

                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << pinItem->getName();

                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                break;
            }
        }
    }

    Result<GatePin*> PinModel::createPin(const QString& name)
    {
        //TODO errorhandling

        return mGate-> create_pin(name.toStdString(), PinDirection::none, PinType::none, false);
    }

    Result<PinGroup<GatePin>*> PinModel::createPinGroup(const QString& name)
    {
        //TODO errorhandling

        Result<GatePin*> pinResult = createPin(name);
        GatePin* pin = pinResult.get();

        return mGate->create_pin_group(name.toStdString(), {pin}, PinDirection::none, PinType::none);
    }

    void PinModel::addPinToPinGroup(u32 pinId, u32 groupId)
    {
        GatePin* pin = mGate->get_pin_by_id(pinId);
        if(!pin)
            return;
        mGate->get_pin_group_by_id(groupId)->assign_pin(pin);
    }


    bool PinModel::renamePin(u32 pinId, const QString& newName) const
    {
        //Check if name is already in use
        if(!isNameAvailable(newName, PinItem::TreeItemType::Pin))
            return false;

        //rename if not in use
        auto pin = mGate->get_pin_by_id(pinId);
        if(!pin)
            return false;

        pin->set_name(newName.toStdString());
        return true;
    }

    bool PinModel::renamePinGroup(u32 pinGroupId, const QString& newName) const
    {
        //Check if name is already in use
        if(!isNameAvailable(newName, PinItem::TreeItemType::PinGroup))
            return false;

        auto pinGroup = mGate->get_pin_group_by_id(pinGroupId);
        if(!pinGroup)
            return false;

        pinGroup->set_name(newName.toStdString());
        return true;
    }

    u32 PinModel::getNewUniqueId(PinItem::TreeItemType type)
    {
        //TODO pins already provide ID creation - discuss
        QSet<u32> nums = {};
        //Check if it is a pin or a pingroup
        if(type == PinItem::TreeItemType::PinGroup){
            for(auto group : mGate->get_pin_groups()){
                nums.insert(group->get_id());
            }
        }
        else{
            for(auto pin : mGate->get_pins()){
                nums.insert(pin->get_id());
            }
        }

        //get next free available ID - we assume that there is at least one u32 available, otherwise this will run in an endless loop
        u32 i = 1;
        while(nums.contains(i))
            i++;

        return i;
    }

    bool PinModel::isNameAvailable(const QString& name, PinItem::TreeItemType type) const
    {
        switch(type){
            case PinItem::TreeItemType::PinGroup:{
                for(auto group : mGate->get_pin_groups()){
                    if(group->get_name() == name.toStdString())
                        return false;
                }
                break;
            }
            case PinItem::TreeItemType::Pin:{
                for(auto pin : mGate->get_pins()){
                    if(pin->get_name() == name.toStdString())
                        return false;
                }
                break;
            }
        }
        return true;
    }


}

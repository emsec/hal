#include "gui/selection_details_widget/selection_details_icon_provider.h"
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
        //TODO dont add dummys if model is in read-only mode
        clear();
        mGate = gate;

        beginResetModel();
        for(auto group : mGate->get_pin_groups()){
            auto groupItem = new PinItem(PinItem::TreeItemType::PinGroup);
            //get all infos for that group
            QString groupDirection        = QString::fromStdString(enum_to_string(group->get_direction()));
            QString groupType             = QString::fromStdString(enum_to_string(group->get_type()));

            //create group item
            groupItem->setData(QList<QVariant>() << group->get_id() << QString::fromStdString(group->get_name()) << groupDirection << groupType);

            for (auto pin : group->get_pins())
            {
                auto pinItem = new PinItem(PinItem::TreeItemType::Pin);

                //get all infos for that pin
                QString pinDirection        = QString::fromStdString(enum_to_string(pin->get_direction()));
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

    void PinModel::handleEditName(QModelIndex index, const QString& input)
    {
        //TODO handle text edited from PinDelegate
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch(itemType){
            case PinItem::TreeItemType::PinGroup: {
                qInfo() << "was Group: " << pinItem->getName();
                if (renamePinGroup(pinItem, input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::Pin:{
                qInfo() << "was Pin: " << pinItem->getName();
                if(renamePin(pinItem, input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                //TODO also check pin names when a group is created - otherwise a group with e.g name I1 will create a pin with the same name but this one isnt checked

                qInfo() << "was GroupCreator: " << pinItem->getName();
                // creates a new pingroup with a pin which is not valid until edited

                auto result = mGate->create_pin_group(input.toStdString(), {}, PinDirection::none, PinType::none);
                if(result.is_error()){
                    qInfo() << "Could not create pingroup: " << QString::fromStdString(result.get_error().get());
                    return;
                }
                auto group = result.get();
                pinItem->setFields(QString::fromStdString(group->get_name()), group->get_id(), group->get_direction(), group->get_type());
                pinItem->setItemType(PinItem::TreeItemType::InvalidPinGroup);
                mInvalidGroups.append(pinItem);

                beginInsertRows(index.parent(), 0, 0);
                // add initial pin to pingroup
                auto initialPin = new PinItem(PinItem::TreeItemType::InvalidPin);
                initialPin->setFields(input, 0, PinDirection::none, PinType::none);
                pinItem->appendChild(initialPin);

                //add new dummypin to the group
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->appendChild(dummyPin);

                //add new dummygroup after entry
                auto dummyGroup = new PinItem(PinItem::TreeItemType::GroupCreator);
                dummyGroup->setData(QList<QVariant>() << "create new group ...");
                pinItem->getParent()->appendChild(dummyGroup);
                endInsertRows();

                //TODO delete group if empty at the end
                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << pinItem->getName();
                // creates a new pin which is not valid until now nor created via gate->create_pin()
                if(!isNameAvailable(input, pinItem))
                    break;
                pinItem->setFields(input, 0, PinDirection::none, PinType::none);
                pinItem->setItemType(PinItem::TreeItemType::InvalidPin);
                mInvalidPins.append(pinItem);

                beginInsertRows(index.parent(),0,0);
                //add new dummy after entry
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->getParent()->appendChild(dummyPin);
                endInsertRows();

                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                qInfo() << "was InvalidPin: " << pinItem->getName();
                if(renamePin(pinItem, input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::InvalidPinGroup:{
                //TODO
                break;
            }
        }
    }

    void PinModel::handleEditDirection(QModelIndex index, const QString& direction)
    {
        //TODO handle direction edited from PinDelegate
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch(itemType){
            case PinItem::TreeItemType::PinGroup:
            case PinItem::TreeItemType::InvalidPinGroup:{
                qInfo() << "was Group: " << pinItem->getName();
                handleGroupDirectionUpdate(pinItem, enum_from_string<PinDirection>(direction.toStdString()));
                //TODO
                break;
            }
            case PinItem::TreeItemType::Pin:{
                qInfo() << "was Pin: " << pinItem->getName();
                GatePin* pin = mGate->get_pin_by_id(pinItem->id());
                if(!pin)
                    break;
                pin->set_direction(enum_from_string<PinDirection>(direction.toStdString()));
                pinItem->setDirection(direction);
                //get the groupItem and update it
                auto groupItem = static_cast<PinItem*>(pinItem->getParent());
                handleGroupDirectionUpdate(groupItem);

                //TODO
                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                qInfo() << "was GroupCreator: " << pinItem->getName();
                //TODO
                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << pinItem->getName();
                //TODO
                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                pinItem->setDirection(direction);
                handleInvalidPinUpdate(pinItem);
                break;
            }
        }
    }

    void PinModel::handleEditType(QModelIndex index, const QString& type)
    {
        //TODO handle type edited from PinDelegate
        auto pinItem  = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch (itemType)
        {
            case PinItem::TreeItemType::PinGroup: {
                qInfo() << "was Group: " << pinItem->getName();
                //TODO
                break;
            }
            case PinItem::TreeItemType::Pin: {
                qInfo() << "was Pin: " << pinItem->getName();
                //get corresponding pin and set the type
                GatePin* pin = mGate->get_pin_by_id(pinItem->id());;
                if(!pin)
                    break;
                //set real pins type
                pin->set_type(enum_from_string<PinType>(type.toStdString()));
                //update pin items type in the model
                pinItem->setType(type);
                break;
            }
            case PinItem::TreeItemType::GroupCreator: {
                qInfo() << "was GroupCreator: " << pinItem->getName();
                //TODO
                break;
            }
            case PinItem::TreeItemType::PinCreator: {
                qInfo() << "was PinCreator: " << pinItem->getName();
                //TODO
                break;
            }
            case PinItem::TreeItemType::InvalidPin: {
                //update the invalid pins type and handle the update
                pinItem->setType(type);
                handleInvalidPinUpdate(pinItem);
                break;
            }
        }
    }

    Result<GatePin*> PinModel::createPin(PinItem* pinItem, bool addToGroup)
    {
        return mGate-> create_pin(
            pinItem->getName().toStdString(),
            enum_from_string<PinDirection>(pinItem->getDirection().toStdString()),
            enum_from_string<PinType>(pinItem->getType().toStdString()),
            addToGroup);
    }


    void PinModel::addPinToPinGroup(u32 pinId, u32 groupId)
    {
        GatePin* pin = mGate->get_pin_by_id(pinId);
        if(!pin)
            return;
        mGate->get_pin_group_by_id(groupId)->assign_pin(pin);
    }


    bool PinModel::renamePin(PinItem* pinItem, const QString& newName) const
    {
        //Check if name is already in use
        if(!isNameAvailable(newName, pinItem))
            return false;

        if(pinItem->id() == 0) return true;

        //rename if not in use
        auto pin = mGate->get_pin_by_id(pinItem->id());
        if(!pin)
            return false;

        pin->set_name(newName.toStdString());
        return true;
    }

    bool PinModel::renamePinGroup(PinItem* groupItem, const QString& newName) const
    {
        //Check if name is already in use
        if(!isNameAvailable(newName, groupItem))
            return false;

        if(groupItem->id() == 0) return true;

        auto pinGroup = mGate->get_pin_group_by_id(groupItem->id());
        if(!pinGroup)
            return false;

        pinGroup->set_name(newName.toStdString());
        return true;
    }

    bool PinModel::isNameAvailable(const QString& name, PinItem* pinItem) const
    {
        switch(pinItem->getItemType()){
            case PinItem::TreeItemType::PinGroup:
            case PinItem::TreeItemType::InvalidPinGroup:
            case PinItem::TreeItemType::GroupCreator:{
                for(auto group : mGate->get_pin_groups()){
                    //dont check the name for itself

                    if(pinItem->id() == group->get_id())
                        continue;
                    if( group->get_name() == name.toStdString())
                        return false;
                }
                //traverse groups that are not yet created
                for(auto group : mInvalidGroups){
                    if(pinItem == group)
                        continue;
                    if(group->getName() == name)
                        return false;
                }
                break;
            }
            case PinItem::TreeItemType::Pin:
            case PinItem::TreeItemType::InvalidPin:
            case PinItem::TreeItemType::PinCreator:{
                for(auto pin : mGate->get_pins()){
                    //dont check the name for itself
                    if(pinItem->id() == pin->get_id())
                        continue;
                    if(pin->get_name() == name.toStdString())
                        return false;
                }
                //traverse pins that are not yet created
                for(auto pin : mInvalidPins){
                    if(pinItem == pin)
                        continue;
                    if(pin->getName() == name)
                        return false;
                }
                break;
            }
        }
        return true;
    }

    void PinModel::handleInvalidPinUpdate(PinItem* pinItem)
    {

        if(!isNameAvailable(pinItem->getName(), pinItem)
            || enum_from_string<PinDirection>(pinItem->getDirection().toStdString()) == PinDirection::none)
            return;  // Pin is not valid

        //pin is valid
        pinItem->setItemType(PinItem::TreeItemType::Pin);
        mInvalidPins.removeAll(pinItem);

        //checks if the groups status is affected and if so it updates the group
        handleInvalidGroupUpdate(static_cast<PinItem*>(pinItem->getParent()));

        //create the new pin
        auto result = createPin(pinItem);
        if(result.is_error()){
            qInfo() << "Could not create pin: " << QString::fromStdString(result.get_error().get());
            return;
        }

        //get corresponding group
        GatePin* pin = result.get();
        auto groupItem = static_cast<PinItem*>(pinItem->getParent());
        u32 pinGroupId = groupItem->id();
        //set pinId to new Id
        pinItem->setId(pin->get_id());

        //add pin to group
        addPinToPinGroup(pin->get_id(),pinGroupId);
    }

    void PinModel::handleInvalidGroupUpdate(PinItem* groupItem){
        //TODO implement logic
        bool isValid = true;

        //calculate new direction
        handleGroupDirectionUpdate(groupItem);

        //check each pin in the group if its valid or not
        for(auto baseTreeItem : groupItem->getChildren()){
            auto childPin = static_cast<PinItem*>(baseTreeItem);
            //if the child is invalid then the group cant be valid
            if(childPin->getItemType() == PinItem::TreeItemType::InvalidPin)
                isValid = false;
        }

        if(isValid){
            //TODO make group valid and define direction and type
            groupItem->setItemType(PinItem::TreeItemType::PinGroup);
            mInvalidGroups.removeAll(groupItem);
        }
    }

    void PinModel::handleGroupDirectionUpdate(PinItem* groupItem ,PinDirection direction){
        if(direction != PinDirection::none){
            //direction was chosen manually
            auto group = mGate->get_pin_group_by_id(groupItem->id());
            if(!group)
                return;
            group->set_direction(direction);
            groupItem->setDirection(QString::fromStdString(enum_to_string(direction)));
            return;
        }

        //direction has to be calculated based on contained pins

        //bitmask with inout = 2³, out = 2², in = 2¹, internal = 2⁰
        int directionMask = 0;

        for(BaseTreeItem* pin : groupItem->getChildren()){
            QString dir = static_cast<PinItem*>(pin)->getDirection();
            if(dir.toStdString() == enum_to_string(PinDirection::inout))
                directionMask = directionMask | (1 << 3);
            else if(dir.toStdString() == enum_to_string(PinDirection::output))
                directionMask = directionMask | (1 << 2);
            else if(dir.toStdString() == enum_to_string(PinDirection::input))
                    directionMask = directionMask | 1 << 1;
            else if(dir.toStdString() == enum_to_string(PinDirection::internal))
                directionMask = directionMask | 1;

            //if all bits are set then break
            if(directionMask >= (1 << 4) - 1)
                break;
        }

        auto group = mGate->get_pin_group_by_id(groupItem->id());
        if(!group)
            return;
        PinDirection calcDir = group->get_direction();
        //inout or  in and out was set
        if(directionMask >= (1 << 3) || (directionMask & (1<<2) && directionMask & (1<<1))){
            calcDir = PinDirection::inout;
        }
        //inout is not set and in and out are not set simultaneously
        else if(directionMask >= 1<<2){
            calcDir = PinDirection::output;
        }
        else if(directionMask > 1){
            calcDir = PinDirection::input;
        }
        else if(directionMask > 0){
            calcDir = PinDirection::internal;
        }

        //else it should stay to what it was before

        //set calculated direction
        group->set_direction(calcDir);
        groupItem->setDirection(QString::fromStdString(enum_to_string(calcDir)));
    }

    bool PinModel::assertionTestForEntry(PinItem* item){
        //TODO remove function after verification
        // tests if a pin or group which is valid does exist in the Gate aswell

        if(item->getItemType() == PinItem::TreeItemType::PinGroup){
            //check if id is present and if so check the names
            auto group = mGate->get_pin_group_by_id(item->id());
            if(!group)
                return false;
            return (group->get_name() == item->getName().toStdString());
        }
        else if(item->getItemType() == PinItem::TreeItemType::Pin){
            //check if id is present and if so check the names
            auto pin = mGate->get_pin_by_id(item->id());
            if(!pin)
                return false;

            return (pin->get_name() == item->getName().toStdString());
        }

        return true;
    }
}

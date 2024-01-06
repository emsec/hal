#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/pin_model/pin_model.h"

namespace hal
{
    PinModel::PinModel(QObject* parent) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "Direction" << "Type" << "");
        mAssignedNames = QSet<QString>();
        mEditable = false;
    }

    PinModel::PinModel(QObject* parent, bool editable) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "Direction" << "Type" << (mEditable ? "DELETE (RENAME ME)" : ""));
        mAssignedNames = QSet<QString>();
        mEditable = editable;
    }

    PinModel::~PinModel(){
        //free old allocated memory of structs
        for(auto pinGroup : mPinGroups){
            for(auto pin : pinGroup->pins){
                delete(pin);
            }
            delete(pinGroup);
        }
    }


    Qt::ItemFlags PinModel::flags(const QModelIndex& index) const
    {
        if(!mEditable)
            return BaseTreeModel::flags(index);

        return Qt::ItemIsEditable | BaseTreeModel::flags(index);
    }

    void PinModel::setGate(GateType* gate){

        //TODO change font of dummy entries
        clear();

        //free old allocated memory of structs
        for(auto pinGroup : mPinGroups){
            for(auto pin : pinGroup->pins){
                delete(pin);
            }
            delete(pinGroup);
            mPinGroups.clear();
        }

        if(gate){
            for (auto pinGroup : gate->get_pin_groups())
            {
                PINGROUP* groupStruct  = new PINGROUP;
                groupStruct->name      = QString::fromStdString(pinGroup->get_name());
                groupStruct->id        = pinGroup->get_id();
                groupStruct->direction = pinGroup->get_direction();
                groupStruct->type      = pinGroup->get_type();
                for (auto pin : pinGroup->get_pins())
                {
                    PIN* pinStruct       = new PIN;
                    pinStruct->name      = QString::fromStdString(pin->get_name());
                    pinStruct->id        = pin->get_id();
                    pinStruct->direction = pin->get_direction();
                    pinStruct->type      = pin->get_type();
                    groupStruct->pins.append(pinStruct);
                }
                mPinGroups.append(groupStruct);
            }
        }

        beginResetModel();
        for(auto group : mPinGroups){
            auto groupItem = new PinItem(PinItem::TreeItemType::PinGroup);
            //get all infos for that group
            QString groupDirection        = QString::fromStdString(enum_to_string(group->direction));
            QString groupType             = QString::fromStdString(enum_to_string(group->type));

            //create group item
            groupItem->setData(QList<QVariant>() << group->id << group->name << groupDirection << groupType);

            for (auto pin : group->pins)
            {
                auto pinItem = new PinItem(PinItem::TreeItemType::Pin);

                //get all infos for that pin
                QString pinDirection        = QString::fromStdString(enum_to_string(pin->direction));
                QString pinType             = QString::fromStdString(enum_to_string(pin->type));

                pinItem->setData(QList<QVariant>() << pin->id << pin->name << pinDirection << pinType);

                groupItem->appendChild(pinItem);

                //add pinItem to the mapped name
                mAssignedNames.insert(pinItem->getName());
            }

            //add dummy to each group if model is editable
            if(mEditable)
            {
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                dummyPin->setData(QList<QVariant>() << "create new pin ...");
                groupItem->appendChild(dummyPin);
            }

            mRootItem->appendChild(groupItem);

            //add groupItem to mapped name
            //add pinItem to the mapped name
            mAssignedNames.insert(groupItem->getName());
        }

        //create dummy for the group creator if model is editable
        if(mEditable)
        {
            auto dummyGroup = new PinItem(PinItem::TreeItemType::GroupCreator);
            dummyGroup->setData(QList<QVariant>() << "create new pingroup ...");
            mRootItem->appendChild(dummyGroup);
        }

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

                if(!isNameAvailable(input, pinItem, true))
                    return;

                pinItem->setFields(input, getNextId(PinItem::TreeItemType::PinGroup), PinDirection::none, PinType::none);
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

                //create new pinGroupStruct and add it to the list of pingroups
                PINGROUP* newPinGroup = new PINGROUP;
                newPinGroup->id = pinItem->id();
                newPinGroup->name = pinItem->getName();
                newPinGroup->direction = enum_from_string<PinDirection>(pinItem->getDirection().toStdString());
                newPinGroup->type = enum_from_string<PinType>(pinItem->getType().toStdString());

                mPinGroups.append(newPinGroup);

                //add pin to group
                addPinToPinGroup(initialPin, pinItem);

                //TODO delete group if empty at the end
                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                qInfo() << "was PinCreator: " << pinItem->getName();
                // creates a new pin which is not valid until now nor created via gate->create_pin()
                if(!isNameAvailable(input, pinItem, true))
                    break;
                pinItem->setFields(input, getNextId(PinItem::TreeItemType::Pin), PinDirection::none, PinType::none);
                pinItem->setItemType(PinItem::TreeItemType::InvalidPin);
                mInvalidPins.append(pinItem);

                beginInsertRows(index.parent(),0,0);
                //add new dummy after entry
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->getParent()->appendChild(dummyPin);
                endInsertRows();

                //add pinItem to pinGroups
                auto pinGroup = static_cast<PinItem*>(pinItem->getParent());
                addPinToPinGroup(pinItem, pinGroup);

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
        printGateMember();
    }

    void PinModel::handleEditDirection(QModelIndex index, const QString& direction)
    {
        //TODO handle direction edited from PinDelegate
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch(itemType){
            case PinItem::TreeItemType::PinGroup:
            case PinItem::TreeItemType::InvalidPinGroup:{
                handleGroupDirectionUpdate(pinItem, enum_from_string<PinDirection>(direction.toStdString()));
                break;
            }
            case PinItem::TreeItemType::Pin:{
                pinItem->setDirection(direction);
                //get the groupItem and update it
                auto groupItem = static_cast<PinItem*>(pinItem->getParent());
                handleGroupDirectionUpdate(groupItem);
                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                pinItem->setDirection(direction);
                handleInvalidPinUpdate(pinItem);
                break;
            }
        }
        printGateMember();
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
        printGateMember();
    }


    void PinModel::addPinToPinGroup(PinItem* pinItem, PinItem* groupItem)
    {
        for(auto pinGroup : mPinGroups){

            if(pinGroup->id == groupItem->id()){
                PIN* pin = new PIN;
                pin->id = pinItem->id();
                pin->name = pinItem->getName();
                pin->direction = enum_from_string<PinDirection>(pinItem->getDirection().toStdString());
                pin->type = enum_from_string<PinType>(pinItem->getType().toStdString());

                pinGroup->pins.append(pin);
                break;
            }
        }

        for(auto pinGroup : mPinGroups){
            auto name = pinGroup->name;
            for(auto pin : pinGroup->pins){
                auto name = pin->name;
            }
        }
    }


    bool PinModel::renamePin(PinItem* pinItem, const QString& newName)
    {
        //TODO change pinItems name within function and not after its call
        //Check if name is already in use
        bool wasRenamed = isNameAvailable(newName, pinItem, true);
        if(wasRenamed){
            //rename pin within pinGroup
            auto pinGroup = static_cast<PinItem*>(pinItem->getParent());
            for(auto group : mPinGroups){
                if(group->name == pinGroup->getName()){
                    for(auto pin : group->pins){
                        if(pin->id == pinItem->id()){
                            pin->name = newName;
                            break;
                        }
                    }
                }
            }
        }

        return wasRenamed;
    }

    bool PinModel::renamePinGroup(PinItem* groupItem, const QString& newName)
    {
        //TODO change pinItems name within function and not after its call
        //Check if name is already in use
        bool wasRenamed = isNameAvailable(newName, groupItem, true);
        if(wasRenamed){
            //rename pinGroup
            for(auto group : mPinGroups){
                if(group->id == groupItem->id()){
                    group->name = newName;
                    break;
                }
            }
        }

        return wasRenamed;
    }

    bool PinModel::isNameAvailable(const QString& name, PinItem* pinItem, bool assign)
    {
        //TODO adjust to structs
        //if the new name is the same as the old one then accept
        if(name == pinItem->getName())
        {
            if(assign)
                mAssignedNames.insert(name);
            return true;
        }

        //check if the name is in the assigned names
        if(!mAssignedNames.contains(name))
        {
            if(assign)
            {
                mAssignedNames.insert(name);
                mAssignedNames.remove(pinItem->getName());
            }
            return true;
        }

        PinItem::TreeItemType type = pinItem->getItemType();
        bool isGroup = (type == PinItem::TreeItemType::PinGroup || type == PinItem::TreeItemType::InvalidPinGroup || type == PinItem::TreeItemType::GroupCreator);

        //get the name of the corresponding group item
        QString groupName = isGroup ? pinItem->getName() : static_cast<PinItem*>(pinItem->getParent())->getName();

        //if it is a group and the name is already taken then return false - or if the pin tries to take a taken name which is not the groupname
        if(isGroup || groupName != name)
            return false;

        //it is not a group item so check if pin can take the name
        PinItem* groupItem = static_cast<PinItem*>(pinItem->getParent());
        //iterate over each child
        for(auto basePin : groupItem->getChildren()){
            auto pin = static_cast<PinItem*>(basePin);
            //check each pin if it already has the name
            if(pin != pinItem){
                if(pin->getName() == name)
                    return false;
            }
        }

        if(assign)
        {
            mAssignedNames.insert(name);
            mAssignedNames.remove(pinItem->getName());
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
        auto pinGroup = static_cast<PinItem*>(pinItem->getParent());
        handleInvalidGroupUpdate(pinGroup);

    }

    void PinModel::handleInvalidGroupUpdate(PinItem* groupItem){
        bool isValid = true;

        //calculate new direction
        handleGroupDirectionUpdate(groupItem);

        //check each pin in the group if its valid or not
        QList<PinItem*> childs = QList<PinItem*>();
        for(auto baseTreeItem : groupItem->getChildren()){
            auto childPin = static_cast<PinItem*>(baseTreeItem);
            //if the child is invalid then the group cant be valid
            if(childPin->getItemType() == PinItem::TreeItemType::InvalidPin){
                isValid = false;
                break;
            }
            if(childPin->getItemType() != PinItem::TreeItemType::PinCreator)
                childs.append(childPin);
        }

        if(isValid){

            groupItem->setItemType(PinItem::TreeItemType::PinGroup);
            /*
            mInvalidGroups.removeAll(groupItem);
            PINGROUP* pinGroup = new PINGROUP;
            pinGroup->id = groupItem->id();
            pinGroup->name = groupItem->getName();
            pinGroup->direction = enum_from_string<PinDirection>(groupItem->getDirection().toStdString());
            pinGroup->type = enum_from_string<PinType>(groupItem->getType().toStdString());

            //add children to pinGroup
            for(auto pin : childs){
                PIN* pinStruct = new PIN;
                pinStruct->name = pin->getName();
                pinStruct->id = pin->id();
                pinStruct->direction = enum_from_string<PinDirection>(pin->getDirection().toStdString());
                pinStruct->type = enum_from_string<PinType>(pin->getType().toStdString());
                pinGroup->pins.append(pinStruct);
            }
            mPinGroups.append(pinGroup);*/
        }
    }

    void PinModel::handleGroupDirectionUpdate(PinItem* groupItem ,PinDirection direction){
        if(direction != PinDirection::none){
            //direction was chosen manually
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


        PinDirection calcDir = enum_from_string<PinDirection>(groupItem->getDirection().toStdString());

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
        groupItem->setDirection(QString::fromStdString(enum_to_string(calcDir)));
    }

    void PinModel::printGateMember()
    {
        qInfo() << "Printing PinGroups and Pins";
        for(auto pinGroup : mPinGroups){
            qInfo() << "PinGroup: " << pinGroup->name << " " << pinGroup->id;
            for(auto pin : pinGroup->pins){
                qInfo() << "  Pin: " << pin->name << " " << pin->id;
            }
        }
    }

    u32 PinModel::getNextId(PinItem::TreeItemType type)
    {
        QSet<u32> takenGroupIds = QSet<u32>();
        QSet<u32> takenPinIds = QSet<u32>();

        //get all currently taken ids
        for(auto pinGroup : mPinGroups){
            takenGroupIds.insert(pinGroup->id);
            for(auto pin : pinGroup->pins){
                takenPinIds.insert(pin->id);
            }
        }

        //find next available id
        u32 counter = 1;
        // can result in an endless loop if 2^32 ids are taken

        if(type == PinItem::TreeItemType::PinGroup){
            while(takenGroupIds.contains(counter)){
                counter++;
            }
        }
        else if(type == PinItem::TreeItemType::Pin){
            while(takenPinIds.contains(counter)){
                counter++;
            }
        }
        else{
            return 0;
        }

        return counter;
    }

    QList<PinModel::PINGROUP*> PinModel::getPinGroups()
    {
        return mPinGroups;
    }

}

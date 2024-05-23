#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/pin_model/pin_model.h"
#include "gui/pin_model/pin_item.h"

namespace hal
{
    PinModel::PinModel(QObject* parent) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "Direction" << "Type");
        mAssignedGroupNames = QSet<QString>();
        mAssignedPinNames = QSet<QString>();
        mEditable = false;
    }

    PinModel::PinModel(QObject* parent, bool editable) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "Direction" << "Type");
        mAssignedGroupNames = QSet<QString>();
        mAssignedPinNames = QSet<QString>();
        mEditable = editable;
    }

    PinModel::~PinModel(){
        BaseTreeModel::clear();
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


        //get all pins of a given gate if it exists
        /*if(gate){
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
        }*/

        QList<PinItem*> tempGroupList;

        if(gate){
            for (auto pinGroup : gate->get_pin_groups())
            {
                PinItem* groupStruct  = new PinItem(PinItem::TreeItemType::PinGroup);
                groupStruct->setName(QString::fromStdString(pinGroup->get_name()));
                groupStruct->setId(pinGroup->get_id());
                groupStruct->setDirection(pinGroup->get_direction());
                groupStruct->setType(pinGroup->get_type());
                for (auto pin : pinGroup->get_pins())
                {
                    PinItem* pinStruct       = new PinItem(PinItem::TreeItemType::Pin);
                    pinStruct->setName(QString::fromStdString(pin->get_name()));
                    pinStruct->setId(pin->get_id());
                    pinStruct->setDirection(pin->get_direction());
                    pinStruct->setType(pin->get_type());
                    groupStruct->appendChild(pinStruct);
                }
                tempGroupList.append(groupStruct);
            }
        }

        beginResetModel();
        for(auto group : tempGroupList){
            auto groupItem = new PinItem(PinItem::TreeItemType::PinGroup);
            //get all infos for that group
            QString groupDirection        = group->getDirection();
            QString groupType             = group->getType();

            //create group item
            groupItem->setData(QList<QVariant>() << group->getId() << group->getName() << groupDirection << groupType);
            for (auto item : group->getChildren())
            {
                auto pinItem = new PinItem(PinItem::TreeItemType::Pin);
                PinItem* pin = static_cast<PinItem*>(item);

                //get all infos for that pin
                QString pinDirection        = pin->getDirection();
                QString pinType             = pin->getType();

                pinItem->setData(QList<QVariant>() << pin->getId() << pin->getName() << pinDirection << pinType);

                groupItem->appendChild(pinItem);

                //add pinItem to the mapped name
                mAssignedPinNames.insert(pinItem->getName());
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
            mAssignedGroupNames.insert(groupItem->getName());
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
                //qInfo() << "was Group: " << pinItem->getName();
                if (renamePinGroup(pinItem, input))
                    pinItem->setName(input);

                break;
            }
            case PinItem::TreeItemType::Pin:{
                //qInfo() << "was Pin: " << pinItem->getName();
                if(renamePin(pinItem, input))
                    pinItem->setName(input);
                break;
            }
            case PinItem::TreeItemType::GroupCreator:{
                //TODO also check pin names when a group is created - otherwise a group with e.g name I1 will create a pin with the same name but this one isnt checked

                //qInfo() << "was GroupCreator: " << pinItem->getName();
                // creates a new pingroup with a pin which is not valid until edited

                if(!renamePinGroup(pinItem, input))
                    return;

                pinItem->setFields(input, getNextId(PinItem::TreeItemType::PinGroup), PinDirection::none, PinType::none);
                pinItem->setItemType(PinItem::TreeItemType::InvalidPinGroup);
                mInvalidGroups.append(pinItem);

                beginInsertRows(index.parent(), 0, 0);
                // add initial pin to pingroup
                auto initialPin = new PinItem(PinItem::TreeItemType::InvalidPin);
                initialPin->setFields(input, 0, PinDirection::none, PinType::none);
                pinItem->appendChild(initialPin);

                mAssignedPinNames.insert(input);

                //add new dummypin to the group
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                //dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->appendChild(dummyPin);

                //add new dummygroup after entry
                auto dummyGroup = new PinItem(PinItem::TreeItemType::GroupCreator);
                //dummyGroup->setData(QList<QVariant>() << "create new group ...");
                pinItem->getParent()->appendChild(dummyGroup);


                //create new pinGroupStruct and add it to the list of pingroups
                PinItem* newPinGroup = new PinItem(PinItem::TreeItemType::PinGroup);
                newPinGroup->setId(pinItem->getId());
                newPinGroup->setName(pinItem->getName());
                newPinGroup->setDirection(pinItem->getDirection());
                newPinGroup->setType(pinItem->getType());
                endInsertRows();

                //add pin to group
                //addPinToPinGroup(initialPin, pinItem);

                break;
            }
            case PinItem::TreeItemType::PinCreator:{
                //qInfo() << "was PinCreator: " << pinItem->getName();
                // creates a new pin which is not valid until now nor created via gate->create_pin()
                if(!renamePin(pinItem, input))
                    break;
                pinItem->setFields(input, getNextId(PinItem::TreeItemType::Pin), PinDirection::none, PinType::none);
                pinItem->setItemType(PinItem::TreeItemType::InvalidPin);
                mInvalidPins.append(pinItem);

                beginInsertRows(index.parent(),0,0);
                //add new dummy after entry
                auto dummyPin = new PinItem(PinItem::TreeItemType::PinCreator);
                //dummyPin->setData(QList<QVariant>() << "create new pin ...");
                pinItem->getParent()->appendChild(dummyPin);
                endInsertRows();

                //add pinItem to pinGroups
                auto pinGroup = static_cast<PinItem*>(pinItem->getParent());
                addPinToPinGroup(pinItem, pinGroup);

                break;
            }
            case PinItem::TreeItemType::InvalidPin:{
                if(renamePin(pinItem, input))
                    pinItem->setName(input);
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
        /*{
                pinItem->setDirection(direction);
                qInfo()<<pinItem->getChildren().length();
                for(auto child : pinItem->getChildren()) //set same direction for all pins of the pingroup
                {
                    PinItem* pin = static_cast<PinItem*>(child);
                    qInfo()<<"1";
                    pin->setDirection(direction);
                }
                handleInvalidGroupUpdate(pinItem);
                //qInfo()<<"handleEditDirection PinGroup";
                break;
            }*/
            case PinItem::TreeItemType::InvalidPinGroup:{
                pinItem->setDirection(direction);
                for(auto child : pinItem->getChildren()) //set same direction for all pins of the pingroup
                {
                    PinItem* pin = static_cast<PinItem*>(child);
                    if(pin->getItemType() == PinItem::TreeItemType::Pin || pin->getItemType() == PinItem::TreeItemType::InvalidPin){
                        pin->setDirection(direction);
                        handleInvalidPinUpdate(pin);
                    }
                }
                handleInvalidGroupUpdate(pinItem);
                break;
            }
            case PinItem::TreeItemType::Pin:
        /*{
                pinItem->setDirection(direction);
                //get the groupItem and update it
                //auto groupItem = static_cast<PinItem*>(pinItem->getParent());
                //handleGroupDirectionUpdate(groupItem);
                handleInvalidPinUpdate(pinItem);
                break;
            }*/
            case PinItem::TreeItemType::InvalidPin:{
                QMessageBox warning;
                QPushButton* acceptBtn = warning.addButton(tr("Continue with changes"), QMessageBox::AcceptRole);
                QPushButton* abortBtn = warning.addButton(QMessageBox::Abort);
                PinItem* parentGroup = static_cast<PinItem*>(pinItem->getParent());
                warning.setWindowTitle("New pin creation");
                warning.setText(QString("You are about to create an %1 pin in an %2 pin group")
                                .arg(direction)
                                .arg(parentGroup->getDirection()));
                if(direction != parentGroup->getDirection())
                {
                    warning.exec();
                    if(warning.clickedButton() == acceptBtn){
                        pinItem->setDirection(direction);
                        handleInvalidPinUpdate(pinItem);
                    }
                }
                else
                {
                    pinItem->setDirection(direction);
                    handleInvalidPinUpdate(pinItem);
                }

                break;
            }
        }
        //printGateMember();
    }

    void PinModel::handleEditType(QModelIndex index, const QString& type)
    {
        auto pinItem  = static_cast<PinItem*>(index.internalPointer());
        auto itemType = pinItem->getItemType();

        switch (itemType)
        {
            case PinItem::TreeItemType::InvalidPin:{
                pinItem->setType(type);
                handleInvalidPinUpdate(pinItem);
                break;
            }
            case PinItem::TreeItemType::Pin: {
                pinItem->setType(type);
                break;
            }
            case PinItem::TreeItemType::InvalidPinGroup:{
                        pinItem->setType(type);
                        for(auto child : pinItem->getChildren()) //set same type for all pins of the pingroup
                        {
                            PinItem* pin = static_cast<PinItem*>(child);
                            if(pin->getItemType() == PinItem::TreeItemType::Pin || pin->getItemType() == PinItem::TreeItemType::InvalidPin){
                                pin->setType(type);
                                handleInvalidPinUpdate(pin);
                            }
                        }
                        handleInvalidGroupUpdate(pinItem);
                        break;
                    }
            case PinItem::TreeItemType::PinGroup: {
                pinItem->setType(type);
                //PinModel::PINGROUP* pinGroup = static_cast<PinModel::PINGROUP*>(pinItem);

                /*for(auto child : pinItem->getChildren()) //set same type for all pins of the pingroup
                {
                    PinItem* pin = static_cast<PinItem*>(child);
                    if(pin->getItemType() == PinItem::TreeItemType::Pin || pin->getItemType() == PinItem::TreeItemType::InvalidPin){
                        pin->setType(type);
                    }
                }*/

                for(auto group : getPinGroups()){
                    if(group->getName() == pinItem->getName()){
                        for(auto item : group->getChildren()){
                            PinItem* pin = static_cast<PinItem*>(item);
                            pin->setType(type);
                        }
                        break;
                    }
                }
            }
        }
    }


    void PinModel::addPinToPinGroup(PinItem* pinItem, PinItem* groupItem)
    {
        QModelIndex index = getIndexFromItem(groupItem);
        beginInsertRows(index, groupItem->getChildCount(), groupItem->getChildCount());
        groupItem->appendChild(pinItem);
        endInsertRows();
    }


    bool PinModel::renamePin(PinItem* pinItem, const QString& newName)
    {
        //TODO change pinItems name within function and not after its call
        //Check if name is already in use

        if(isNameAvailable(newName, pinItem)){
            //delete old name from being assigned and add new name
            mAssignedPinNames.remove(pinItem->getName());
            mAssignedPinNames.insert(newName);

            //rename PinItem
            pinItem->setName(newName);

            //rename pin within pinGroup
            auto pinGroup = static_cast<PinItem*>(pinItem->getParent());
            for(auto group : getPinGroups()){
                if(group->getName() == pinGroup->getName()){
                    for(auto item : group->getChildren()){
                        PinItem* pin = static_cast<PinItem*>(item);
                        if(pin->getId() == pinItem->getId()){
                            pin->setName(newName);
                            break;
                        }
                    }
                    break;
                }
            }
            return true;
        }

        return false;
    }

    bool PinModel::renamePinGroup(PinItem* groupItem, const QString& newName)
    {
        //TODO change pinItems name within function and not after its call
        //Check if name is already in use
        if(isNameAvailable(newName, groupItem)){
            //delete old name from being assigned and add new name
            mAssignedGroupNames.remove(groupItem->getName());
            mAssignedGroupNames.insert(newName);

            //rename PinItem
            groupItem->setName(newName);

            //rename corresponding pinGroup
            for(auto group : getPinGroups()){
                if(group->getId() == groupItem->getId()){
                    group->setName(newName);
                    break;
                }
            }
            return true;
        }

        return false;
    }

    bool PinModel::isNameAvailable(const QString& newName, PinItem* treeItem){
        //check if the new name is the same as the old one
        if(newName == treeItem->getName())
            return true;

        //get type of the pin item
        PinItem::TreeItemType type = treeItem->getItemType();
        bool isGroup = (type == PinItem::TreeItemType::PinGroup || type == PinItem::TreeItemType::GroupCreator || type == PinItem::TreeItemType::InvalidPinGroup);
        if(isGroup){
            return !(mAssignedGroupNames.contains(newName) || mAssignedPinNames.contains(newName));
        }else{
            //get group name
            QString groupName = static_cast<PinItem*>(treeItem->getParent())->getName();
            //check if pin can be named after the group
            if(groupName == newName && !mAssignedPinNames.contains(newName))
            {
                return true;
            }
            //check if other group or pin has the name
            else{
                return !(mAssignedGroupNames.contains(newName) || mAssignedPinNames.contains(newName));
            }
        }
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
        //handleGroupDirectionUpdate(groupItem);

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

    void PinModel::handleGroupDirectionUpdate(PinItem* groupItem, PinDirection direction){
        if(direction != PinDirection::none){
            //direction was chosen manually
            groupItem->setDirection(QString::fromStdString(enum_to_string(direction)));
            return;
        }

        //direction has to be calculated based on contained pins

        //bitmask with inout = 2³, out = 2², in = 2¹, internal = 2⁰
        /*int directionMask = 0;

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
        groupItem->setDirection(QString::fromStdString(enum_to_string(calcDir)));*/
    }

    void PinModel::printGateMember()
    {
        qInfo() << "Printing PinGroups and Pins";
        for(auto pinGroup : getPinGroups()){
            qInfo() << "PinGroup: " << pinGroup->getName() << " " << pinGroup->getId();
            for(auto item : pinGroup->getChildren()){
                PinItem* pin = static_cast<PinItem*>(item);
                qInfo() << "  Pin: " << pin->getName() << " " << pin->getId();
            }
        }
    }

    u32 PinModel::getNextId(PinItem::TreeItemType type)
    {
        QSet<u32> takenGroupIds = QSet<u32>();
        QSet<u32> takenPinIds = QSet<u32>();

        //get all currently taken ids
        for(auto item : getPinGroups()){
            PinItem* pinGroup = static_cast<PinItem*>(item);
            takenGroupIds.insert(pinGroup->getId());
            for(auto item : pinGroup->getChildren()){
                PinItem* pin = static_cast<PinItem*>(item);
                takenPinIds.insert(pin->getId());
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

    QList<PinItem*> PinModel::getPinGroups() const
    {
        QList<PinItem*> retval;
        for (BaseTreeItem* bti : mRootItem->getChildren())
            retval.append(static_cast<PinItem*>(bti));
        return retval;
    }

    void PinModel::handleDeleteItem(QModelIndex index)
    {
        //get item from index
        auto item = static_cast<PinItem*>(getItemFromIndex(index));

        if(!item)
            return;

        //remove item from the PinItem list

        //check the type of the item
        PinItem::TreeItemType type = item->getItemType();
        if(type == PinItem::TreeItemType::PinGroup || type == PinItem::TreeItemType::InvalidPinGroup){
            //handle group deletion
            beginRemoveRows(index, 0, 0);
            for(auto item : getPinGroups()){
                PinItem* group = static_cast<PinItem*>(item);
                //delete all pins from the group and free group afterward
                if(group->getId() == item->getId()){
                    for(auto pin : group->getChildren()){
                        delete(pin);
                    }
                    delete(group);
                    break;
                }
            }
            //remove actual modelItem

            getRootItem()->removeChild(item);
            endRemoveRows();
            handleItemRemoved(item);
        }
        else if(type == PinItem::TreeItemType::Pin || type == PinItem::TreeItemType::InvalidPin){
            //handle pin deletion

            //get the parent group of the pin
            auto parentGroup = static_cast<PinItem*>(item->getParent());
            for(auto item : getPinGroups()){
                PinItem* group = static_cast<PinItem*>(item);
                if(group->getId() == parentGroup->getId()){
                    //delete pin from the group
                    for(auto itemPin : group->getChildren()){
                        PinItem* pin = static_cast<PinItem*>(itemPin);
                        if(pin->getId() == item->getId()){
                            group->removeChild(pin);
                          delete(pin);
                          break;
                        }
                    }
                    break;
                }
            }
            //remove actual modelItem
            beginRemoveRows(index, 0, 0);
            parentGroup->removeChild(item);
            endRemoveRows();
            handleItemRemoved(item);
        }
    }

    void PinModel::handleItemRemoved(PinItem* item)
    {

        //TODO reorganize name handling
        //remove name from assigned list
        if(item->getItemType() == PinItem::TreeItemType::PinGroup || item->getItemType() == PinItem::TreeItemType::InvalidPinGroup){
            //name should be unique amongst pingroups so this can be deleted
            mAssignedGroupNames.remove(item->getName());
            mAssignedPinNames.remove(item->getName());
        }
        else{
            //item was a pin so we have to check if the parent group is assigned to that name,  otherwise it can be freed
            mAssignedPinNames.remove(item->getName());
        }
    }

}

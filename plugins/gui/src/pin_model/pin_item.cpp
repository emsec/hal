#include "gui/pin_model/pin_item.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{

    PinItem::PinItem(PinItem::TreeItemType type) :
          BaseTreeItem(),
          mItemType(type)
    {

    }

    int PinItem::row() const
    {
        BaseTreeItem* parent = getParent();
        if (!parent) return 0;
        return parent->getRowForChild(this);
    }

    QVariant PinItem::getData(int column) const
    {
        switch (column){
            case 0:
                return QVariant(mName);
            case 1:
                return QVariant(mDirection);
            case 2:
                return QVariant(mType);
        }
        return QVariant();
    }

    void PinItem::setData(QList<QVariant> data)
    {
        //TODO initialize
        switch(mItemType){
            case TreeItemType::PinGroup:{
                mId = data[0].toInt();
                mName = data[1].toString();
                mDirection = "";
                mType = "";
                break;
            }
            case TreeItemType::Pin:{
                mId = data[0].toInt();
                mName = data[1].toString();
                mDirection = data[2].toString();
                mType = data[3].toString();
                break;
            }
            case TreeItemType::GroupCreator:
            case TreeItemType::PinCreator: {
                mName = data[0].toString();
                mId = 0;
                mDirection = "";
                mType = "";
                break;
            }

        }
    }

    void PinItem::setDataAtIndex(int index, QVariant &data)
    {

    }

    QString PinItem::getName() const
    {
        return mName;
    }

    u32 PinItem::id() const
    {
        return mId;
    }

    void PinItem::setName(const QString& name)
    {
        mName = name;
    }

    int PinItem::getColumnCount() const
    {
        return 3;
    }

    void PinItem::appendData(QVariant data) {}

    PinItem::TreeItemType PinItem::getItemType() const
    {
        return mItemType;
    }


    void PinItem::setDirection(PinDirection direction)
    {
        //TODO provide enum to string method
        switch(direction){
            case PinDirection::none:{
                mDirection = "none";
                break;
            }
            case PinDirection::input:{
                mDirection = "input";
                break;
            }
            case PinDirection::inout:{
                mDirection = "inout";
                break;
            }
            case PinDirection::internal:{
                mDirection = "internal";
                break;
            }
            case PinDirection::output:{
                mDirection = "output";
                break;
            }
        }
    }

    void PinItem::setType(PinType type)
    {
        //TODO provide enum to string method
        switch(type){
            case PinType::none:{
                mType = "none";
                break;
            }
            case PinType::power:{
                mType = "power";
                break;
            }
            case PinType::ground:{
                mType = "ground";
                break;
            }
            case PinType::lut:{
                mType = "lut";
                break;
            }
            case PinType::state:{
                mType = "state";
                break;
            }
            case PinType::neg_state:{
                mType = "neg_state";
                break;
            }
            case PinType::clock:{
                mType = "clock";
                break;
            }
            case PinType::enable:{
                mType = "enable";
                break;
            }
            case PinType::set:{
                mType = "set";
                break;
            }
            case PinType::reset:{
                mType = "reset";
                break;
            }
            case PinType::data:{
                mType = "data";
                break;
            }
            case PinType::address:{
                mType = "address";
                break;
            }
            case PinType::io_pad:{
                mType = "io_pad";
                break;
            }
            case PinType::select:{
                mType = "select";
                break;
            }
            case PinType::carry:{
                mType = "carry";
                break;
            }
            case PinType::sum:{
                mType = "sum";
                break;
            }
        }
    }

    void PinItem::setFields(GatePin* pin){
        setFields(QString::fromStdString(pin->get_name()), pin->get_id(), pin->get_direction(), pin->get_type());
    }

    void PinItem::setFields(const QString& name, u32 id, PinDirection direction, PinType type)
    {
        mId = id;
        mName = name;
        setDirection(direction);
        setType(type);
    }

    void PinItem::setItemType(PinItem::TreeItemType type)
    {
        mItemType = type;
    }
}

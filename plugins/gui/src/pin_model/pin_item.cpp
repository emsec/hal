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
            case TreeItemType::PinGroup:
            case TreeItemType::Pin: {
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

    QString PinItem::getType() const
    {
        return mType;
    }

    QString PinItem::getDirection() const
    {
        return mDirection;
    }

    u32 PinItem::id() const
    {
        return mId;
    }

    void PinItem::setName(const QString& name)
    {
        mName = name;
    }

    void PinItem::setDirection(const QString& direction)
    {
        mDirection = direction;
    }

    void PinItem::setType(const QString& type){
        mType = type;
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
        mDirection = QString::fromStdString(enum_to_string(direction));
    }

    void PinItem::setType(PinType type)
    {
        mType = QString::fromStdString(enum_to_string(type));
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

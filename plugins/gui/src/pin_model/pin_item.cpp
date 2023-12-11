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
}

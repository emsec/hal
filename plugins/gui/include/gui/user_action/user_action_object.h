#pragma once
#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "hal_core/defines.h"
#include "gui/selection_relay/selection_relay.h"

namespace hal {
    class UserActionObjectType : public QObject
    {
        Q_OBJECT
    public:
        enum ObjectType{ None, Module, Gate, Net, Netlist, Grouping, MaxObjectType };
        Q_ENUM(ObjectType)

        static ObjectType fromString(const QString& s);
        static QString toString(ObjectType t);
        static ObjectType fromItemType(SelectionRelay::ItemType itp);
        static SelectionRelay::ItemType toItemType(ObjectType t);
    };


    class UserActionObject
    {
    public:
        UserActionObject(u32 i=0, UserActionObjectType::ObjectType t=UserActionObjectType::None)
            : mId(i), mType(t) {;}
        UserActionObjectType::ObjectType type() const { return mType; }
        u32 id() const { return mId; }
        void writeToXml(QXmlStreamWriter& xmlOut) const;
        void readFromXml(QXmlStreamReader& xmlIn);
    private:
        u32 mId;
        UserActionObjectType::ObjectType mType;
    };

}

#include "gui/user_action/user_action_object.h"
#include "gui/selection_relay/selection_relay.h"
#include <QMetaEnum>

namespace hal {
    UserActionObjectType::ObjectType UserActionObjectType::fromString(const QString& s)
    {
        QMetaEnum me = QMetaEnum::fromType<ObjectType>();
        for (int t = None; t < MaxObjectType; t++)
            if (s == me.key(t))
                return static_cast<ObjectType>(t);
        return None;
    }

    QString UserActionObjectType::toString(ObjectType t)
    {
        QMetaEnum me = QMetaEnum::fromType<ObjectType>();
        return QString(me.key(t));
    }

    UserActionObjectType::ObjectType UserActionObjectType::fromItemType(SelectionRelay::ItemType itp)
    {
        ObjectType retval = None;
        switch (itp)
        {
        case SelectionRelay::ItemType::Module: retval = Module; break;
        case SelectionRelay::ItemType::Gate:   retval = Gate;   break;
        case SelectionRelay::ItemType::Net:    retval = Net;    break;
        default:                                                break;
        }
        return retval;
    }

    SelectionRelay::ItemType UserActionObjectType::toItemType(UserActionObjectType::ObjectType t)
    {
        SelectionRelay::ItemType retval = SelectionRelay::ItemType::None;
        switch (t)
        {
        case Module: retval = SelectionRelay::ItemType::Module; break;
        case Gate:   retval = SelectionRelay::ItemType::Gate;   break;
        case Net:    retval = SelectionRelay::ItemType::Net;    break;
        default:                                                break;
        }
        return retval;
    }

    void UserActionObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        if (mType==UserActionObjectType::None) return;
        xmlOut.writeAttribute("id", QString::number(mId));
        xmlOut.writeAttribute("type",UserActionObjectType::toString(mType));
    }

    void UserActionObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        QStringRef srId = xmlIn.attributes().value("id");
        if (srId.isNull() || srId.isEmpty()) return;
        QStringRef srTp = xmlIn.attributes().value("type");
        if (srTp.isNull() || srTp.isEmpty()) return;
        bool ok = false;
        mId = srId.toInt(&ok);
        if (!ok) return;
        mType = UserActionObjectType::fromString(srTp.toString());
    }
}

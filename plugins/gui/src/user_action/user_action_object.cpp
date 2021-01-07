#include "gui/user_action/user_action_object.h"
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

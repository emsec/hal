#pragma once
#include <QString>
#include "user_action_manager.h"
#include "user_action_object.h"

namespace hal
{
    class UserAction
    {
    public:
        virtual void exec();
        virtual QString tagname() const = 0;
        virtual void writeToXml(QXmlStreamWriter& xmlOut) const = 0;
        virtual void readFromXml(QXmlStreamReader& xmlIn) = 0;

        virtual UserActionObject object() const { return mObject; }
        virtual void setObject(const UserActionObject& o) { mObject = o; }
        bool isWaitForReady() const { return mWaitForReady; }
    protected:
        UserAction();
        UserActionObject mObject;
        bool mWaitForReady;
        UserAction *mUndoAction;
    };

    class UserActionFactory
    {
    protected:
        QString mTagname;
    public:
        UserActionFactory(const QString& nam);
        QString tagname() const { return mTagname; }
        virtual UserAction* newAction() const = 0;
    };
}


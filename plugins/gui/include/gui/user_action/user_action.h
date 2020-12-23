#pragma once
#include <QString>
#include "user_action_manager.h"

namespace hal
{
    class UserAction
    {
    public:
        virtual void exec();
        UserActionManager::UserActionType type() const { return mType; }
        virtual void writeToXml(QXmlStreamWriter& xmlOut) const = 0;
        virtual void readFromXml(QXmlStreamReader& xmlIn) = 0;

        virtual u32 objectId() const { return mObjectId; }
        virtual void setObjectId(u32 id_) { mObjectId = id_; }
        bool isWaitForReady() const { return mWaitForReady; }
    protected:
        UserAction(UserActionManager::UserActionType type_);

        UserActionManager::UserActionType mType;
        u32 mObjectId;
        bool mWaitForReady;
        UserAction *mUndoAction;
    };
}


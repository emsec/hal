#pragma once

#include "user_action.h"
#include <QList>

namespace hal {
    class UserActionCompound : public UserAction
    {
    public:
        UserActionCompound();
        void addAction(UserAction* act);
        void exec() override;
        void writeToXml(QXmlStreamWriter& xmlOut) const;
        void readFromXml(QXmlStreamReader& xmlIn);
        void setUseCreatedObject() { mUseCreatedObject = true; }

    protected:
        QList<UserAction*> mActionList;
        bool mUseCreatedObject;
    };
}

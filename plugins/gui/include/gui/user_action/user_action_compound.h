#pragma once

#include "user_action.h"
#include <QList>

namespace hal {
    class UserActionCompound : public UserAction
    {
    public:
        UserActionCompound();
        void addAction(UserAction* act);
        QString tagname() const override;
        void exec() override;
        void writeToXml(QXmlStreamWriter& xmlOut) const;
        void readFromXml(QXmlStreamReader& xmlIn);
        void setUseCreatedObject() { mUseCreatedObject = true; }

    protected:
        QList<UserAction*> mActionList;
        bool mUseCreatedObject;
    };

    class UserActionCompundFactory : public UserActionFactory
    {
    public:
        UserActionCompundFactory();
        UserAction* newAction() const;
    };

}

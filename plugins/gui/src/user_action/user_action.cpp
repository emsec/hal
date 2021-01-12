#include "gui/user_action/user_action.h"

namespace hal
{
    UserActionFactory::UserActionFactory(const QString& nam)
        : mTagname(nam)
    {
        UserActionManager::instance()->registerFactory(this);
    }

    UserAction::UserAction()
        : mWaitForReady(false), mCompound(false),
          mUndoAction(nullptr) {;}

    void UserAction::exec() {
        UserActionManager::instance()->addExecutedAction(this);
    }

    void UserAction::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        Q_UNUSED(xmlOut);
    }

    void UserAction::readFromXml(QXmlStreamReader& xmlIn)
    {
        Q_UNUSED(xmlIn);
    }
}

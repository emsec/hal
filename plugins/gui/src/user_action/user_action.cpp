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
          mUndoAction(nullptr), mTimeStamp(0)
    {;}

    UserAction::~UserAction()
    {
        if (mUndoAction) delete mUndoAction;
    }

    void UserAction::exec()
    {
        UserActionManager* uam = UserActionManager::instance();
        mTimeStamp = uam->timeStamp();
        uam->addExecutedAction(this);
    }

    QString UserAction::cryptographicHash() const
    {
        QCryptographicHash cryptoHash(QCryptographicHash::Md5);
        cryptoHash.addData(tagname().toUtf8());
        cryptoHash.addData((char*) (&mObject), sizeof(mObject));
        cryptoHash.addData((char*) (&mTimeStamp), sizeof(mTimeStamp));
        addToHash(cryptoHash);
        return QString::fromUtf8(cryptoHash.result().toHex(0));
    }

    void UserAction::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        Q_UNUSED(xmlOut);
    }

    void UserAction::readFromXml(QXmlStreamReader& xmlIn)
    {
        Q_UNUSED(xmlIn);
    }

    void UserAction::addToHash(QCryptographicHash& cryptoHash) const
    {
        Q_UNUSED(cryptoHash);
    }
}

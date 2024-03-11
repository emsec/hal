#include "gui/user_action/user_action.h"
#include "gui/gui_globals.h"

namespace hal
{
    UserActionFactory::UserActionFactory(const QString& nam)
        : mTagname(nam)
    {
        UserActionManager::instance()->registerFactory(this);
    }

    UserAction::UserAction()
        : mCompoundOrder(-1), mUndoAction(nullptr),
          mTimeStamp(0), mObjectLock(false),
          mProjectModified(true)
    {;}

    UserAction::~UserAction()
    {
        if (mUndoAction) delete mUndoAction;
    }

    bool UserAction::exec()
    {
        UserActionManager* uam = UserActionManager::instance();
        mTimeStamp = uam->timeStamp();
        uam->addExecutedAction(this);
        if (hasProjectModified()) gFileStatusManager->netlistChanged();
        return true;
    }

    void UserAction::setObject(const UserActionObject &obj)
    {
        if (mObjectLock) return;
        mObject = obj;
    }

    QString UserAction::cryptographicHash(int recordNo) const
    {
        QCryptographicHash cryptoHash(QCryptographicHash::Sha256);
        cryptoHash.addData((char*) (&recordNo), sizeof(int));
        cryptoHash.addData(tagname().toUtf8());
        cryptoHash.addData((char*) (&mObject), sizeof(mObject));
        cryptoHash.addData((char*) (&mTimeStamp), sizeof(mTimeStamp));
        addToHash(cryptoHash);
        return QString::fromUtf8(cryptoHash.result().toHex(0));
    }

    QString UserAction::setToText(const QSet<u32> &set)
    {
        QString retval;
        for (u32 id : set)
        {
            if (!retval.isEmpty()) retval += ',';
            retval += QString::number(id);
        }
        return retval;
    }

    QSet<u32> UserAction::setFromText(const QString& s)
    {
        QSet<u32> retval;
        for (QString x : s.split(QChar(',')))
            retval.insert(x.toInt());
        return retval;
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

    QString UserAction::debugDump() const
    {
        QString retval = "  ";
        if (mCompoundOrder >= 0) retval = QString("%1 ").arg((char)('0' + mCompoundOrder%10));
        retval += QString("%1 %2\n").arg(tagname(),-22).arg(mObject.debugDump());
        return retval;
    }
}

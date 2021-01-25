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
}

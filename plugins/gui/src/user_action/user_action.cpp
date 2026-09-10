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
        cryptoHash.addData((char*) (&mParentObject), sizeof (mParentObject));
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

    QString UserAction::gridToText(const QHash<hal::Node,QPoint>& grid)
    {
        QString retval;
        for (auto it = grid.constBegin(); it!= grid.constEnd(); ++it)
        {
            if (!retval.isEmpty()) retval += ',';
            retval += QString("%1%2:%3:%4")
                    .arg(it.key().type()==hal::Node::Module?'M':'G')
                    .arg(it.key().id())
                    .arg(it.value().x())
                    .arg(it.value().y());
        }
        return retval;
    }

    QHash<hal::Node,QPoint> UserAction::gridFromText(const QString& txt)
    {
        QHash<hal::Node,QPoint> retval;
        for (QString s : txt.split(','))
        {
            if (s.isEmpty()) continue;
            hal::Node::NodeType tp = hal::Node::None;
            switch (s.at(0).unicode())
            {
            case 'M':
                tp = hal::Node::Module;
                break;
            case 'G':
                tp = hal::Node::Gate;
                break;
            default:
                continue;
            }
            QStringList num = s.mid(1).split(':');
            if (num.size() != 3) continue;
            retval.insert(hal::Node(num.at(0).toUInt(),tp),QPoint(num.at(1).toInt(),num.at(2).toInt()));
        }
        return retval;
    }

    void UserAction::writeParentObjectToXml(QXmlStreamWriter &xmlOut) const
    {
        if(mParentObject.type() != UserActionObjectType::None)
        {
            xmlOut.writeStartElement("parentObj");
            mParentObject.writeToXml(xmlOut);
            xmlOut.writeEndElement();
        }
    }

    void UserAction::readParentObjectFromXml(QXmlStreamReader &xmlIn)
    {
        if(xmlIn.name() == QString("parentObj"))
        {
            mParentObject.readFromXml(xmlIn);
            xmlIn.readNext();//to read the corresponding EndElement
        }
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

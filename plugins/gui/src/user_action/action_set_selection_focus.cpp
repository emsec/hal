#include "gui/user_action/action_set_selection_focus.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionSetSelectionFocusFactory::ActionSetSelectionFocusFactory()
        : UserActionFactory("SetSelectionFocus") {;}

    ActionSetSelectionFocusFactory* ActionSetSelectionFocusFactory::sFactory = new ActionSetSelectionFocusFactory;

    UserAction* ActionSetSelectionFocusFactory::newAction() const
    {
        return new ActionSetSelectionFocus;
    }

    ActionSetSelectionFocus::ActionSetSelectionFocus()
    {
        mProjectModified = false;
    }

    QString ActionSetSelectionFocus::tagname() const
    {
        return ActionSetSelectionFocusFactory::sFactory->tagname();
    }

    void ActionSetSelectionFocus::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData((char*)(&mSubfocus),sizeof(mSubfocus));
        cryptoHash.addData((char*)(&mSubfocusIndex),sizeof(mSubfocusIndex));
        cryptoHash.addData("mod",3);
        cryptoHash.addData(setToText(mModules).toUtf8());
        cryptoHash.addData("gat",3);
        cryptoHash.addData(setToText(mGates).toUtf8());
        cryptoHash.addData("net",3);
        cryptoHash.addData(setToText(mNets).toUtf8());
    }

    void ActionSetSelectionFocus::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        switch (mSubfocus)
        {
        case SelectionRelay::Subfocus::Left:
            xmlOut.writeAttribute("subfocus","Left");
            xmlOut.writeAttribute("subfocusIndex", QString::number(mSubfocusIndex));
            break;
        case SelectionRelay::Subfocus::Right:
            xmlOut.writeAttribute("subfocus","Right");
            xmlOut.writeAttribute("subfocusIndex", QString::number(mSubfocusIndex));
            break;
        default:
            break;
        }

        if (!mModules.isEmpty()) xmlOut.writeTextElement("modules",setToText(mModules));
        if (!mGates.isEmpty())   xmlOut.writeTextElement("gates",setToText(mGates));
        if (!mNets.isEmpty())    xmlOut.writeTextElement("nets",setToText(mNets));
    }

    void ActionSetSelectionFocus::readFromXml(QXmlStreamReader& xmlIn)
    {
        auto sfocAttribute = xmlIn.attributes().value("subfocus");   // Qt5: QStringRef    Qt6: QStringView
        if (!sfocAttribute.isNull() && !sfocAttribute.isEmpty())
        {
            if (sfocAttribute == QString("Left"))
            {
                mSubfocus = SelectionRelay::Subfocus::Left;
                mSubfocusIndex = xmlIn.attributes().value("subfocusIndex").toInt();
            }
            else if (sfocAttribute == QString("Right"))
            {
                mSubfocus = SelectionRelay::Subfocus::Right;
                mSubfocusIndex = xmlIn.attributes().value("subfocusIndex").toInt();
            }
        }

        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name()==QString("modules"))
                mModules = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()==QString("gates"))
                mGates = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()==QString("nets"))
                mNets = setFromText(xmlIn.readElementText());
        }
    }

    bool ActionSetSelectionFocus::hasModifications() const
    {
        if (gSelectionRelay->focusId()         != mObject.id())     return true;
        if (UserActionObjectType::fromSelectionType(gSelectionRelay->focusType())
                                               != mObject.type())   return true;
        if (gSelectionRelay->subfocus()        != mSubfocus)        return true;
        if (gSelectionRelay->subfocusIndex()   != mSubfocusIndex)   return true;
        if (gSelectionRelay->selectedModules() != mModules)         return true;
        if (gSelectionRelay->selectedGates()   != mGates)           return true;
        if (gSelectionRelay->selectedNets()    != mNets)            return true;
        return false;
    }

    void ActionSetSelectionFocus::setObject(const UserActionObject &obj)
    {
        UserAction::setObject(obj);
        if (obj.id() > 0)
            switch (obj.type())
            {
            case UserActionObjectType::Module:
                mModules.insert(obj.id());
                break;
            case UserActionObjectType::Gate:
                mGates.insert(obj.id());
                break;
            case UserActionObjectType::Net:
                mNets.insert(obj.id());
                break;
            default:
                break;
            }
    }

    bool ActionSetSelectionFocus::exec()
    {
        ActionSetSelectionFocus* undo = new ActionSetSelectionFocus;
        undo->mModules = gSelectionRelay->selectedModules();
        undo->mGates   = gSelectionRelay->selectedGates();
        undo->mNets    = gSelectionRelay->selectedNets();
        undo->mSubfocus = gSelectionRelay->subfocus();
        undo->mSubfocusIndex = gSelectionRelay->subfocusIndex();
        undo->mObject = UserActionObject(gSelectionRelay->focusId(),
                                         UserActionObjectType::fromSelectionType(gSelectionRelay->focusType()));
        mUndoAction = undo;
        gSelectionRelay->actionSetSelected(mModules, mGates, mNets);
        gSelectionRelay->setFocusDirect(UserActionObjectType::toSelectionType(mObject.type()),
                                  mObject.id(),mSubfocus,mSubfocusIndex);
        return UserAction::exec();
    }
}

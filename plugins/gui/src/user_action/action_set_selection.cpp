#include "gui/user_action/action_set_selection.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionSetSelectionFactory::ActionSetSelectionFactory()
        : UserActionFactory("SetSelection") {;}

    ActionSetSelectionFactory* ActionSetSelectionFactory::sFactory = new ActionSetSelectionFactory;

    UserAction* ActionSetSelectionFactory::newAction() const
    {
        return new ActionSetSelection;
    }

    QString ActionSetSelection::tagname() const
    {
        return ActionSetSelectionFactory::sFactory->tagname();
    }

    void ActionSetSelection::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData("mod",3);
        cryptoHash.addData(setToText(mModules).toUtf8());
        cryptoHash.addData("gat",3);
        cryptoHash.addData(setToText(mGates).toUtf8());
        cryptoHash.addData("net",3);
        cryptoHash.addData(setToText(mNets).toUtf8());
    }

    void ActionSetSelection::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        if (!mModules.isEmpty()) xmlOut.writeTextElement("modules",setToText(mModules));
        if (!mGates.isEmpty())   xmlOut.writeTextElement("gates",setToText(mGates));
        if (!mNets.isEmpty())    xmlOut.writeTextElement("nets",setToText(mNets));
    }

    void ActionSetSelection::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name()=="modules")
                mModules = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="gates")
                mGates = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="nets")
                mNets = setFromText(xmlIn.readElementText());
        }
    }

    bool ActionSetSelection::hasModifications() const
    {
        if (gSelectionRelay->selectedModules() != mModules) return true;
        if (gSelectionRelay->selectedGates()   != mGates)   return true;
        if (gSelectionRelay->selectedNets()    != mNets)    return true;
        return false;
    }

    void ActionSetSelection::exec()
    {
        ActionSetSelection* undo = new ActionSetSelection;
        undo->mModules = gSelectionRelay->selectedModules();
        undo->mGates   = gSelectionRelay->selectedGates();
        undo->mNets    = gSelectionRelay->selectedNets();
        mUndoAction = undo;
        gSelectionRelay->actionSetSelected(mModules, mGates, mNets);
        UserAction::exec();
    }
}

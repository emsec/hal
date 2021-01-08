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

    QString ActionSetSelection::setToText(const QSet<u32> &set) const
    {
        QString retval;
        for (u32 id : set)
        {
            if (!retval.isEmpty()) retval += ',';
            retval += QString::number(id);
        }
        return retval;
    }

    QSet<u32> ActionSetSelection::setFromText(const QString& s) const
    {
        QSet<u32> retval;
        for (QString x : s.split(QChar(',')))
            retval.insert(x.toInt());
        return retval;
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

    void ActionSetSelection::exec()
    {
        gSelectionRelay->setSelectedModules(mModules);
        gSelectionRelay->setSelectedGates(mGates);
        gSelectionRelay->setSelectedNets(mNets);
        gSelectionRelay->relaySelectionChanged(nullptr);
        UserAction::exec();
    }
}

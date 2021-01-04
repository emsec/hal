#include "gui/user_action/action_create_grouping.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"

namespace hal
{
    ActionCreateGrouping::ActionCreateGrouping(const QString &gname)
      : UserAction(UserActionManager::CreateGrouping),
        mGroupingName(gname)
    {;}

    void ActionCreateGrouping::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("groupingname", mGroupingName);
    }

    void ActionCreateGrouping::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "groupingname")
                mGroupingName = xmlIn.readElementText();
        }
    }

    void ActionCreateGrouping::exec()
    {
        Grouping* g = gNetlist->create_grouping(mGroupingName.toStdString());
        setObjectId(g->get_id());
    }
}

#include "gui/user_action/action_open_netlist_file.h"
#include "gui/file_manager/file_manager.h"

namespace hal
{
    ActionOpenNetlistFile::ActionOpenNetlistFile(const QString &filename_)
      : UserAction(UserActionManager::OpenNetlistFile),
        mFilename(filename_)
    {
        mWaitForReady = true;
    }

    void ActionOpenNetlistFile::exec()
    {
        FileManager::get_instance()->openFile(mFilename);
        UserAction::exec();
    }

    void ActionOpenNetlistFile::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("filename", mFilename);
    }

    void ActionOpenNetlistFile::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "filename")
                mFilename = xmlIn.readElementText();
        }
    }
}

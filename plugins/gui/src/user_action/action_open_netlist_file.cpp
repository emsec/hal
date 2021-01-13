#include "gui/user_action/action_open_netlist_file.h"
#include "gui/file_manager/file_manager.h"

namespace hal
{
    ActionOpenNetlistFileFactory::ActionOpenNetlistFileFactory()
       : UserActionFactory("OpenNetlistFile") {;}

    ActionOpenNetlistFileFactory* ActionOpenNetlistFileFactory::sFactory = new ActionOpenNetlistFileFactory;

    UserAction* ActionOpenNetlistFileFactory::newAction() const
    {
        return new ActionOpenNetlistFile;
    }

    QString ActionOpenNetlistFile::tagname() const
    {
        return ActionOpenNetlistFileFactory::sFactory->tagname();
    }

    ActionOpenNetlistFile::ActionOpenNetlistFile(const QString &filename_)
        : mFilename(filename_)
    {
        mWaitForReady = true;
    }

    void ActionOpenNetlistFile::exec()
    {
        FileManager::get_instance()->openFile(mFilename);
        UserAction::exec();
    }

    void ActionOpenNetlistFile::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mFilename.toUtf8());
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

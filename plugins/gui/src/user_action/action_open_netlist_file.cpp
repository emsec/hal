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

    ActionOpenNetlistFile::ActionOpenNetlistFile(const QString &filename_, bool isProj)
        : mFilename(filename_), mProject(isProj)
    {
        // mWaitForReady = true;
    }

    bool ActionOpenNetlistFile::exec()
    {
        if (mProject)
            FileManager::get_instance()->openProject(mFilename);
        else
            FileManager::get_instance()->importFile(mFilename);
        return UserAction::exec();
    }

    void ActionOpenNetlistFile::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mFilename.toUtf8());
        cryptoHash.addData((char*) &mProject, sizeof(mProject));
    }

    void ActionOpenNetlistFile::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("filename", mFilename);
        xmlOut.writeTextElement("isproject", mProject ? "true" : "false");
    }

    void ActionOpenNetlistFile::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "filename")
                mFilename = xmlIn.readElementText();
            if (xmlIn.name() == "isproject")
                mProject = (xmlIn.readElementText() == "true");
        }
    }
}

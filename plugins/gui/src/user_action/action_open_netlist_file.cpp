#include "gui/user_action/action_open_netlist_file.h"
#include "gui/file_manager/file_manager.h"

namespace hal
{
    const char* sOpenMethodsPersist[] = {"Undefined", "CreateNew", "ImportFile", "OpenProject", nullptr};

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

    ActionOpenNetlistFile::ActionOpenNetlistFile(OpenMethod method, const QString &filename_)
        : mFilename(filename_), mMethod(method)
    {
        mProjectModified = false;
    }

    bool ActionOpenNetlistFile::exec()
    {
        switch (mMethod)
        {
        case Undefined:
            return false;
        case CreateNew:
            FileManager::get_instance()->newProject();
            break;
        case ImportFile:
            FileManager::get_instance()->importFile(mFilename);
            break;
        case OpenProject:
            FileManager::get_instance()->openProject(mFilename);
            break;
        }
        return UserAction::exec();
    }

    void ActionOpenNetlistFile::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mFilename.toUtf8());
        cryptoHash.addData((char*) &mMethod, sizeof(mMethod));
    }

    void ActionOpenNetlistFile::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("filename", mFilename);
        xmlOut.writeTextElement("method", sOpenMethodsPersist[mMethod]);
    }

    void ActionOpenNetlistFile::readFromXml(QXmlStreamReader& xmlIn)
    {
        mMethod = Undefined;
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "filename")
                mFilename = xmlIn.readElementText();
            if (xmlIn.name() == "method")
            {
                QString meth = xmlIn.readElementText();
                for (int i=0; sOpenMethodsPersist[i]; i++)
                    if (sOpenMethodsPersist[i] == meth)
                    {
                        mMethod = static_cast<OpenMethod>(i);
                        break;
                    }
            }
        }
    }
}

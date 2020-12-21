#include "gui/user_action/action_open_netlist_file.h"
#include "gui/file_manager/file_manager.h"

namespace hal
{
    ActionOpenNetlistFile::ActionOpenNetlistFile(const QString &filename_)
      : UserAction(UserActionManager::OpenNetlistFile),
        mFilename(filename_)
    {;}

    void ActionOpenNetlistFile::exec()
    {
        FileManager::get_instance()->openFile(mFilename);
        UserAction::exec();
    }
}

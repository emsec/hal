#include "gui/graph_widget/graph_context_serializer.h"
#include "hal_core/utilities/project_filelist.h"
#include "hal_core/utilities/project_manager.h"
#include "gui/gui_globals.h"
#include <QString>

namespace hal
{

    void GraphContextSerializer::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        restore(loaddir);
    }

    bool GraphContextSerializer::restore(const std::filesystem::path& loaddir) const
    {
        ProjectManager* pm = ProjectManager::instance();
        ProjectFilelist* pfl = pm->get_filelist(m_name);
        if (!pfl || pfl->empty()) return false;
        std::filesystem::path filename = loaddir.empty() ? pm->get_project_directory() : loaddir;
        filename.append(pfl->at(0));
        return gGraphContextManager->restoreFromFile(QString::fromStdString(filename.string()));
    }

    ProjectFilelist* GraphContextSerializer::serialize(Netlist* netlist, const std::filesystem::path& savedir)
    {
        Q_UNUSED(netlist);
        std::string ctxFile("views.json");
        std::filesystem::path ctxPath(savedir);
        ctxPath.append(ctxFile);
        if (!gGraphContextManager->handleSaveTriggered(QString::fromStdString(ctxPath.string())))
            return nullptr;
        ProjectFilelist* retval = new ProjectFilelist;
        retval->push_back(ctxFile);
        return retval;
    }
}


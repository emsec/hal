#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_modifier/netlist_modifier.h"

#include <QApplication>
#include <QMessageBox>

namespace hal
{
    /**
     * @brief This functions replaces all the selected gates with a gate of an unknown gate type
     * 
     * @param probe_type The allowed probe type that should be written in the ini file
     * @param probe_limit The allowed probe limit that should be written in the ini file
     * @return bool true in success
     */
    bool NetlistModifierPlugin::modify_in_place(int probe_type, int probe_limit)
    {
        // lock the GUI so the view is not updated for every replaced gate individually but rather only once when all gates are replaced
        UIPluginInterface* mGuiPlugin = plugin_manager::get_plugin_instance<UIPluginInterface>("hal_gui");
        if (mGuiPlugin)
            mGuiPlugin->set_layout_locker(true);

        // modify the gate lib
        if (!modify_gatelibrary())
        {
            if (mGuiPlugin)
                mGuiPlugin->set_layout_locker(false);
            QMessageBox::warning(qApp->activeWindow(), "Warning", "Creating modified gate library with unknown gate types was not successful!");
            return false;
        }

        // get all selected gates to replace them
        std::vector<Gate*> gates = GuiApi().getSelectedGates();

        // save original netlist if it does not contain any UNKNOWN gates
        bool contains_unknown = false;
        for (Gate* gate : gNetlist->get_gates())
        {
            if (gate->get_type()->get_name().find("UNKNOWN_") != std::string::npos)
            {
                contains_unknown = true;
            }
        }

        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        // If the netlist does not contain any unknown gates then create the encrypted zip archive
        if (!contains_unknown)
        {
            if (!create_encrypted_zip(SECRET_PASSWORD, probe_type, probe_limit))
            {
                if (mGuiPlugin)
                    mGuiPlugin->set_layout_locker(false);
                QMessageBox::warning(qApp->activeWindow(), "Warning", "Creating encrypted zip file was not successful!");
                return false;
            }
        }
        // if the netlist already contains unknown gates then just update the existing zip archive
        else
        {
            if (std::filesystem::exists(project_dir_path / "original/original.zip"))
            {
                if (!update_encrypted_zip(SECRET_PASSWORD, probe_type, probe_limit))
                {
                    if (mGuiPlugin)
                        mGuiPlugin->set_layout_locker(false);
                    QMessageBox::warning(qApp->activeWindow(), "Warning", "Updating encrypted zip file was not successful!");
                    return false;
                }
            }
            else
            {
                if (mGuiPlugin)
                    mGuiPlugin->set_layout_locker(false);
                QMessageBox::warning(qApp->activeWindow(), "Warning", "You opened a project containing unknown gates but there is no original.zip file containing the original netlist!");
                return false;
            }
        }

        // iterate over all selected gates and replace them one by one
        for (Gate* gate : gates)
        {
            if (!replace_gate_in_netlist(gNetlist, gate))
            {
                if (mGuiPlugin)
                    mGuiPlugin->set_layout_locker(false);
                QMessageBox::warning(qApp->activeWindow(), "Warning", "Error while replacing gates!");
                return false;
            }
        }
        
        // release the layout lock once finished
        if (mGuiPlugin)
            mGuiPlugin->set_layout_locker(false);

        return true;
    }
}    // namespace hal
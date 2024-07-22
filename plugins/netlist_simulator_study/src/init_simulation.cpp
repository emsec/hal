#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/project_manager.h"
#include "netlist_simulator_study/netlist_simulator_study.h"

#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>

namespace hal
{
    void q_setting_from_string(std::string data, QSettings* settings)
    {
        QTemporaryFile tempFile;
        if (tempFile.open())
        {
            // Write INI data to the temporary file
            QTextStream out(&tempFile);
            out << QString::fromStdString(data);
            tempFile.close();

            // Read and parse INI data from the temporary file using QSettings
            QSettings tmp_settings(tempFile.fileName(), QSettings::IniFormat);

            // Remove the temporary file
            tempFile.remove();

            settings->clear();

            for (const QString& key : tmp_settings.allKeys())
            {
                settings->setValue(key, tmp_settings.value(key));
            }

            return;
        }

        log_error("netlist_simulator_study", "Can't create tmp ini file!");
        return;
    }

    std::vector<const Net*> gen_scan_chain_probes(std::vector<Net*> nets)
    {
        std::vector<const Net*> output;

        for (Net* net : nets)
        {
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                continue;
            }

            std::vector<hal::Endpoint*> sources = net->get_sources();
            bool dff_as_source                  = false;

            for (Endpoint* source : sources)
            {
                if (source->get_gate()->get_type()->has_property(GateTypeProperty::ff))
                {
                    dff_as_source = true;
                    break;
                }
            }

            if (!dff_as_source)
            {
                continue;
            }

            output.push_back(net);
        }

        return output;
    }

    std::vector<const Net*> gen_t_probe_probes(std::vector<Net*> nets)
    {
        std::vector<const Net*> output;

        for (Net* net : nets)
        {
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                continue;
            }
            output.push_back(net);
        }

        return output;
    }

    void NetlistSimulatorStudyPlugin::init_simulation(std::string input_file)
    {
        if (input_file == "")
        {
            return;
        }

        GuiApi* guiAPI = new GuiApi();

        std::vector<Net*> nets = guiAPI->getSelectedNets();

        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        std::vector<const Net*> probes;

        std::string salt     = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", SECRET_PASSWORD, "salt.encrypt");
        std::string ini_data = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", gen_salted_password(SECRET_PASSWORD, salt), "settings.ini");

        QSettings my_settings;
        q_setting_from_string(ini_data, &my_settings);

        if (!my_settings.contains("section1/probe_type"))
        {
            log_error("netlist_simulator_study", "Settings file is missing probe_type value!");
            return;
        }
        int PROBE_TYPE = my_settings.value("section1/probe_type").toInt();

        if (!my_settings.contains("section1/max_probes"))
        {
            log_error("netlist_simulator_study", "Settings file is missing max_probes value!");
            return;
        }
        int MAX_PROBES = my_settings.value("section1/max_probes").toInt();

        if (PROBE_TYPE == 0)
        {
            // scan chain
            probes = gen_scan_chain_probes(nets);
        }
        else if (PROBE_TYPE == 1)
        {
            // t probe
            probes = gen_t_probe_probes(nets);
        }
        else
        {
            // invalid type
            log_error("netlist_simulator_study", "Invalid probe_type value!");
            return;
        }

        if (probes.size() > MAX_PROBES)
        {
            log_error("netlist_simulator_study", "You selected more than " + std::to_string(MAX_PROBES) + " probes");
            return;
        }

        // add all input and output nets to the selection so the results are kept in the verilator output
        for (Net* net : gNetlist->get_nets())
        {
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                probes.push_back(net);
            }
        }

        simulate(input_file, probes);
    }
}    // namespace hal
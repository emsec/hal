#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/project_manager.h"
#include "netlist_simulator_study/netlist_simulator_study.h"

#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>

namespace hal
{
    /**
     * @brief This function creates a QSettings ini file from a string containing the content of the ini file. The resulting QSetting file is written to the QSetting object provided as a parameter
     * 
     * @param data String containing the data that should be written to the QSetting object
     * @param settings A pointer to a QSetting object where the data should be written to
     */
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

            // add all the key/value pairs to the new Settings object
            for (const QString& key : tmp_settings.allKeys())
            {
                settings->setValue(key, tmp_settings.value(key));
            }

            return;
        }

        log_error("netlist_simulator_study", "Can't create tmp ini file!");
        return;
    }

    /**
     * @brief Given a set of nets that should be probed this function filters out all the once that are not valid probes. In this case only nets that are at the output of a flip flip are valid. Also all global input and outputs are filtered out as these are probed automaticaly and do not count towards the maximum probing limit
     * 
     * @param nets List of nets that should be probed
     * @return std::vector<const Net*> List of all the allowed probes
     */
    std::vector<const Net*> gen_scan_chain_probes(std::vector<Net*> nets)
    {
        std::vector<const Net*> output;

        for (Net* net : nets)
        {
            // ignore the net if it is a global input or output
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                continue;
            }

            std::vector<hal::Endpoint*> sources = net->get_sources();
            bool dff_as_source                  = false;

            // iterate over all sources and check if one of them is a flip flop
            for (Endpoint* source : sources)
            {
                if (source->get_gate()->get_type()->has_property(GateTypeProperty::ff))
                {
                    dff_as_source = true;
                    break;
                }
            }

            // ignore the net if it is not the output net of a flip flop
            if (!dff_as_source)
            {
                continue;
            }

            output.push_back(net);
        }

        return output;
    }

    /**
     * @brief Given a set of nets that should be probed this function filters out all the once that are not valid probes. Also all global input and outputs are filtered out as these are probed automaticaly and do not count towards the maximum probing limit
     * 
     * @param nets 
     * @return std::vector<const Net*> 
     */
    std::vector<const Net*> gen_t_probe_probes(std::vector<Net*> nets)
    {
        std::vector<const Net*> output;
        for (Net* net : nets)
        {
            // ignore the net if it is a global input or output
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                continue;
            }
            output.push_back(net);
        }

        return output;
    }

    /**
     * @brief Function that is called if the python binding is used
     * 
     * @param input_file Path to the input file for the simulation
     * @param nets List of nets that should be probed
     */
    void NetlistSimulatorStudyPlugin::init_simulation_from_python(std::string input_file, std::vector<Net*> nets){
        init_simulation_internal(input_file, nets);
    }

    /**
     * @brief Function that is called if the simulation is started from the GUi popup
     * 
     * @param input_file Path to the input file for the simulation
     */
    void NetlistSimulatorStudyPlugin::init_simulation(std::string input_file)
    {
        GuiApi* guiAPI = new GuiApi();

        // get all selected nets as probes
        std::vector<Net*> nets = guiAPI->getSelectedNets();

        init_simulation_internal(input_file, nets);
    }

    /**
     * @brief This function evaluates if the submitted nets are valid probes and if the maximum number of probes is not exceeded. If everything is okay the simulation is started
     * 
     * @param input_file Path to the input file for the simulation
     * @param nets List of nets that should be probed
     */
    void NetlistSimulatorStudyPlugin::init_simulation_internal(std::string input_file, std::vector<Net*> nets)
    {
        // check if a path is submitted
        if (input_file == "")
        {
            return;
        }

        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        std::vector<const Net*> probes;

        // read the salt from the encrypted zip archive and use it to decrypt the ini file
        std::string salt     = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", SECRET_PASSWORD, "salt.encrypt");
        std::string ini_data = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", gen_salted_password(SECRET_PASSWORD, salt), "settings.ini");

        // create a QSetting object from the content of the ini file
        QSettings my_settings;
        q_setting_from_string(ini_data, &my_settings);

        // read the probe type and probe limit from the settings
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

        std::cout << PROBE_TYPE << std::endl;
        std::cout << MAX_PROBES << std::endl;

        // filter out the invalid probes depending on the probing method
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

        // test if the maximum probing limit is exceeded
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

        // start the simulation with the given input file and the probes
        simulate(input_file, probes);
    }
}    // namespace hal
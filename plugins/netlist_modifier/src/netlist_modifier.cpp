#include "netlist_modifier/netlist_modifier.h"

#include "boost/functional/hash.hpp"
#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/project_manager.h"

#include "hal_core/netlist/persistent/netlist_serializer.h"

#include <deque>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <JlCompress.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"

#define SECRET_PASSWORD "test12345"

namespace hal
{

    const char* OBFUSCATED = "_obfuscated";
    const char* GATE_LIB_TAG = "gate_library";

    extern Netlist* gNetlist;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistModifierPlugin>();
    }

    NetlistModifierPlugin::NetlistModifierPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string NetlistModifierPlugin::get_name() const
    {
        return std::string("netlist-modifier");
    }

    std::string NetlistModifierPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistModifierPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionNetlistModifier;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
    }

    void NetlistModifierPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
    }

    std::set<std::string> NetlistModifierPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("hal_gui");
        retval.insert("verilog_writer");
        retval.insert("hgl_writer");
        return retval;
    }

    void NetlistModifierPlugin::initialize()
    {
    }

    std::string NetlistModifierPlugin::obfuscated_gate_name(int num_in, int num_out, int num_io)
    {
        std::string retval = "UNKNOWN_" + std::to_string(num_in) + "IN_" + std::to_string(num_out) + "OUT";
        if (num_io)
            retval += "_" + std::to_string(num_io) + "IO";
        return retval;
    }

    bool NetlistModifierPlugin::replace_gate_in_netlist(Netlist* netlist, Gate* gate)
    {
        // get the number of input pins
        int in_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::input; }).size();
        int out_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::output; }).size();
        int in_out_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::inout; }).size();

        GateType* new_gate_type = netlist->get_gate_library()->get_gate_type_by_name(obfuscated_gate_name(in_pins,out_pins, in_out_pins));

        if (!new_gate_type)
        {
            log_error("netlist_modifier", "No gatetype called '{}' in gatelib", obfuscated_gate_name(in_pins,out_pins, in_out_pins));
            return false;
        }

        std::string gate_name = "UNKNOWN_" + std::to_string(gate->get_id());
        u32 gate_id           = gate->get_id();

        std::pair position = gate->get_location();

        Module* module = gate->get_module();

        std::vector<Net*> in_nets;
        std::vector<Net*> out_nets;

        // save the input and output nets
        for (Net* net : gate->get_fan_in_nets())
        {
            in_nets.push_back(net);
        }
        for (Net* net : gate->get_fan_out_nets())
        {
            out_nets.push_back(net);
        }

        // delete old gate and add new one
        netlist->delete_gate(gate);
        Gate* new_gate = netlist->create_gate(gate_id, new_gate_type, gate_name);

        // add old connections
        int counter = 0;
        for (Net* net : in_nets)
        {
            net->add_destination(new_gate, "I" + std::to_string(counter));
            counter++;
        }

        counter = 0;
        for (Net* net : out_nets)
        {
            net->add_source(new_gate, "O" + std::to_string(counter));
            counter++;
        }

        if (position.first > 0 && position.second > 0){
            new_gate->set_location(position);
        }

        if (module != new_gate->get_module())
            module->assign_gate(new_gate);

        return true;
    }

    std::string gen_random_string(int len){
        const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string random_string;
        random_string.reserve(len);
        
        srand((unsigned)time(NULL)); 

        for (int i = 0; i < len; i++)
        {
            random_string += characters[rand() % (characters.size())];
        }
        
        return random_string;
    }

    std::string gen_salted_password(std::string password, std::string salt){
        return password + salt;
    }

    bool create_encrypted_zip(std::string password){
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        std::string salt = gen_random_string(20);
        std::string salted_password = gen_salted_password(password, salt);

        // create original folder if missing
        if (std::filesystem::exists(project_dir_path / "original/")){
            if (!std::filesystem::is_directory(project_dir_path / "original/")) {
                log_error("netlist_modifier", "A file with the name 'original' already exists.");
                return false;
            }
        }else{
            if (!std::filesystem::create_directory(project_dir_path / "original/")) {
                log_error("netlist_modifier", "Could not create original folder!");
                return false;
            }
        }

        // create tmp folder if missing
        if (std::filesystem::exists(project_dir_path / "original/tmp/")){
            if (!std::filesystem::is_directory(project_dir_path / "original/tmp/")) {
                log_error("netlist_modifier", "A file with the name 'original/tmp' already exists.");
                return false;
            }
        }else{
            
            if (!std::filesystem::create_directory(project_dir_path / "original/tmp/")) {
                log_error("netlist_modifier", "Could not create original folder!");
                return false;
            }
        }

        // create tmp salt file
        // Open the file for writing
        std::ofstream salt_outFile((project_dir_path / "original/tmp/salt.encrypt").c_str());

        // Check if the file is successfully opened
        if (!salt_outFile.is_open()) {
            log_error("netlist_modifier", "Error opening new salt file!");
            return false;
        }

        // Write the content to the file
        salt_outFile << salt;

        // Close the file
        salt_outFile.close();

        // create tmp original netlist file
        netlist_serializer::serialize_to_file(gNetlist, project_dir_path / "original/tmp/original.hal");

        // create tmp ini file
        std::string ini_content = R"([section1]
; wether to use t probe (n arbitrary probes) or scan chain (n probes at FF output)
; true = t probe
; false = scan chain
t_probe=true
max_probes=5)";

        // Open the file for writing
        std::ofstream ini_outFile((project_dir_path / "original/tmp/settings.ini").c_str());

        // Check if the file is successfully opened
        if (!ini_outFile.is_open()) {
            log_error("netlist_modifier", "Error opening new ini file!");
            return false;
        }

        // Write the content to the file
        ini_outFile << ini_content;

        // Close the file
        ini_outFile.close();

        // create tmp zip file
        QuaZip zip(QString::fromStdString(project_dir_path / "original/original.zip"));

        if (!zip.open(QuaZip::mdCreate)) {
            log_error("netlist_modifier", "Failed to create ZIP archive!");
            return false;
        }


        QFile netlist_file(QString::fromStdString(project_dir_path / "original/tmp/original.hal"));
        if (!netlist_file.open(QIODevice::ReadOnly)) {
            log_error("netlist_modifier", "Failed to open file!");
            return false;
        }
        QuaZipFile netlist_zip_outFile(&zip);
        if (!netlist_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(netlist_file.fileName()).fileName()), salted_password.c_str())) {
            log_error("File could not be added with encryption!");
            return false;
        }

        netlist_zip_outFile.write(netlist_file.readAll());

        netlist_zip_outFile.close();
        netlist_file.close();
        
        QFile ini_file(QString::fromStdString(project_dir_path / "original/tmp/settings.ini"));
        if (!ini_file.open(QIODevice::ReadOnly)) {
            log_error("netlist_modifier", "Failed to open file!");
            return false;
        }
        QuaZipFile ini_zip_outFile(&zip);
        if (!ini_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(ini_file.fileName()).fileName()), salted_password.c_str())) {
            log_error("File could not be added with encryption!");
            return false;
        }
        
        ini_zip_outFile.write(ini_file.readAll());

        ini_zip_outFile.close();
        ini_file.close();

        QFile salt_file(QString::fromStdString(project_dir_path / "original/tmp/salt.encrypt"));
        if (!salt_file.open(QIODevice::ReadOnly)) {
            log_error("netlist_modifier", "Failed to open file!");
            return false;
        }
        QuaZipFile salt_zip_outFile(&zip);
        if (!salt_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(salt_file.fileName()).fileName()), password.c_str())) {
            log_error("File could not be added with encryption!");
            return false;
        }
        
        salt_zip_outFile.write(salt_file.readAll());

        salt_zip_outFile.close();
        salt_file.close();

        zip.close();

        // delete tmp folder
        try{
            std::filesystem::remove(project_dir_path / "original/tmp/settings.ini");
            std::filesystem::remove(project_dir_path / "original/tmp/original.hal");
            std::filesystem::remove(project_dir_path / "original/tmp/salt.encrypt");
            std::filesystem::remove(project_dir_path / "original/tmp");
        } catch (const std::filesystem::filesystem_error& e){
            log_error("netlist_modifier", "Failed to delete tmp directory");
            std::cerr << e.what() << std::endl;
        }
        
        return true;
    }

    bool NetlistModifierPlugin::modify_gatelibrary()
    {
        ProjectManager* pm = ProjectManager::instance();

        std::filesystem::path projFilePath(pm->get_project_directory());
        if (projFilePath.empty() || !std::filesystem::exists(projFilePath))
        {
            log_warning("netlist_modifier", "Cannot access project directory '{}'.", projFilePath.string());
            return false;
        }
        projFilePath.append(ProjectManager::s_project_file);

        // get location of current gate library from project file
        FILE* fp = fopen(projFilePath.string().c_str(), "rb");
        if (fp == NULL)
        {
            log_warning("netlist_modifier", "Cannot open project file '{} for reading'.", projFilePath.string());
            return false;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(fp, buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(fp);

        std::filesystem::path modifiedGateLibraryPath;
        if (doc.HasMember(GATE_LIB_TAG))
        {
            std::filesystem::path originalGateLibraryPath = doc[GATE_LIB_TAG].GetString();
            if (originalGateLibraryPath.stem().string().find(OBFUSCATED) != std::string::npos)
            {
                // using already gate library with obfuscated gates
                return true;
            }
            // generate new name for gate library with obfuscated gates
            modifiedGateLibraryPath = pm->get_project_directory().get_filename(originalGateLibraryPath.stem().string() + OBFUSCATED + ".hgl");
        }
        else
        {
            log_warning("netlist_modifier", "Cannot find mandatory '{}' tag in project file '{}'.", GATE_LIB_TAG, projFilePath.string());
            return false; // gate library entry missing in project file
        }

        // yes, we know what we are doing when casting away const ;-)
        //     Modifying attibutes of a GateType in HAL while netlist is loaded might produce unpredictable results,
        //     thus GateLibrary is declared const. However, we are only adding GateType's, this should be safe.
        GateLibrary* gl = const_cast<GateLibrary*>(gNetlist->get_gate_library());

        // map gate type categories by number of pins
        std::unordered_map<u32,int> pinCountMap;
        for (auto const& [key, gt] : gl->get_gate_types())
        {
            int count[5] = {0, 0, 0, 0, 0};
            for (const GatePin* gp : gt->get_pins())
            {
                count[(int)gp->get_direction()]++;
            }
            u32 pinCount = (count[1] & 0x3FF) | ((count[2] & 0x3FF) << 10) | ((count[3] & 0x3FF) << 20);
            ++pinCountMap[pinCount];
        }

        // create dummy gate types with appropriate number of pins
        for  (auto const& [pc,count] : pinCountMap)
        {
            int inCount  = pc & 0x3FF;
            int outCount = (pc >> 10) & 0x3FF;
            int ioCount  = (pc >> 20) & 0x3FF;
            GateType* gt = gl->create_gate_type(obfuscated_gate_name(inCount, outCount, ioCount));
            for (int i=0; i<inCount; i++)
                gt->create_pin("I" + std::to_string(i), PinDirection::input, PinType::none, true);
            for (int i=0; i<outCount; i++)
                gt->create_pin("O" + std::to_string(i), PinDirection::output, PinType::none, true);
            for (int i=0; i<ioCount; i++)
                gt->create_pin("IO" + std::to_string(i), PinDirection::inout, PinType::none, true);
        }

        if (!gate_library_writer_manager::write(gl, modifiedGateLibraryPath))
        {
            log_warning("netlist_modifier", "cannot write modified gate library to file'{}'.", modifiedGateLibraryPath.string());
            return false;
        }

        // write modified project file
        rapidjson::Value::MemberIterator gatelibMemberIterator = doc.FindMember(GATE_LIB_TAG);
        gatelibMemberIterator->value.SetString(modifiedGateLibraryPath.filename().string().c_str(), doc.GetAllocator());

        FILE* of = fopen(projFilePath.string().c_str(), "wb");
        if (of == NULL)
        {
            log_warning("netlist_modifier", "cannot open project file '{}' for rewrite.", projFilePath.string());
            return false;
        }
        rapidjson::FileWriteStream fws(of, buffer, sizeof(buffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(fws);
        doc.Accept(writer);
        fclose(of);
        gl->set_path(modifiedGateLibraryPath);
        pm->set_gate_library_path(modifiedGateLibraryPath.filename().string());

        return true;
    }

    bool NetlistModifierPlugin::modify_in_place()
    {
        UIPluginInterface* mGuiPlugin = plugin_manager::get_plugin_instance<UIPluginInterface>("hal_gui");
        if (mGuiPlugin)
            mGuiPlugin->set_layout_locker(true);        

        if (!modify_gatelibrary()){
            if (mGuiPlugin)
                mGuiPlugin->set_layout_locker(false);
            return false;
        }

        std::vector<Gate*> gates = GuiApi().getSelectedGates();

        // save original netlist if it does not contain any UNKNOWN gates
        bool contains_unknown = false;
        for (Gate* gate: gNetlist->get_gates()){
            if(gate->get_type()->get_name().find("UNKNOWN_") != std::string::npos){
                contains_unknown = true;
            }
        }
        if(!contains_unknown){
            create_encrypted_zip(SECRET_PASSWORD);
        }

        for (Gate* gate : gates)
        {
            if (!replace_gate_in_netlist(gNetlist, gate))
            {
                if (mGuiPlugin)
                    mGuiPlugin->set_layout_locker(false);
                return false;
            }
        }

        if (mGuiPlugin)
            mGuiPlugin->set_layout_locker(false);

        return true;
    }

    bool NetlistModifierPlugin::save()
    {
        ProjectManager* pm = ProjectManager::instance();

        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if (std::filesystem::exists(project_dir_path / "generated") && !std::filesystem::is_directory(project_dir_path / "generated"))
        {
            log_error("netlist_modifier", "A file called 'generated' in the project directory exists but it is expected to be a directory!");
            return false;
        }

        if (!std::filesystem::exists(project_dir_path / "generated"))
        {
            std::filesystem::create_directories(project_dir_path / "generated");
        }

        netlist_serializer::serialize_to_file(gNetlist, project_dir_path / "generated/generated_netlist_obfuscated.hal");

        std::filesystem::copy(gNetlist->get_gate_library()->get_path(), project_dir_path / "generated/generated_gatelib_obfuscated.hgl", std::filesystem::copy_options::overwrite_existing);

        return true;
    }

    GuiExtensionNetlistModifier::GuiExtensionNetlistModifier()
    {
        m_parameter.push_back(PluginParameter(PluginParameter::PushButton, "modify_in_place", "Modify selected gates in place"));
        m_parameter.push_back(PluginParameter(PluginParameter::PushButton, "save", "Save"));
    }

    std::vector<PluginParameter> GuiExtensionNetlistModifier::get_parameter() const
    {
        return m_parameter;
    }

    void GuiExtensionNetlistModifier::set_parameter(const std::vector<PluginParameter>& params)
    {
        m_parameter = params;

        bool modify_in_place = false;
        bool save            = false;

        for (PluginParameter par : m_parameter)
        {
            if (par.get_tagname() == "modify_in_place" && par.get_value() == "clicked")
            {
                modify_in_place = true;
            }
            else if (par.get_tagname() == "save" && par.get_value() == "clicked")
            {
                save = true;
            }
        }
        if (modify_in_place && m_parent)
        {
            m_parent->modify_in_place();
        }
        else if (save && m_parent)
        {
            m_parent->save();
        }
    }
}    // namespace hal

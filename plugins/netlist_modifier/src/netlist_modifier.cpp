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

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

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
        int num_of_in  = gate->get_fan_in_endpoints().size();
        int num_of_out = gate->get_fan_out_endpoints().size();

        GateType* new_gate_type = netlist->get_gate_library()->get_gate_type_by_name(obfuscated_gate_name(num_of_in,num_of_out));

        if (!new_gate_type)
        {
            log_error("netlist_modifier", "No gatetype called '{}' in gatelib", obfuscated_gate_name(num_of_in,num_of_out));
            return false;
        }

        std::string gate_name = "UNKNOWN_" + std::to_string(gate->get_id());
        u32 gate_id           = gate->get_id();

        std::vector<Net*> in_nets;
        std::vector<Net*> out_nets;

        // save the input and output nets
        for (Endpoint* ep : gate->get_fan_in_endpoints())
        {
            in_nets.push_back(ep->get_net());
        }
        for (Endpoint* ep : gate->get_fan_out_endpoints())
        {
            out_nets.push_back(ep->get_net());
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

        return true;
    }

    std::string encryptAES(const std::string& plaintext, const std::string& key) {
        if(key.size() != static_cast<int>(CryptoPP::AES::DEFAULT_KEYLENGTH)){
            log_error("netlist_modifier", "Key needs to be "+std::to_string(static_cast<int>(CryptoPP::AES::DEFAULT_KEYLENGTH))+" bytes long!");
            return "";
        }
        std::string ciphertext;

        CryptoPP::AES::Encryption aesEncryption((const CryptoPP::byte*)key.data(), CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (const CryptoPP::byte*)key.data());

        CryptoPP::StringSource encryptor(plaintext, true, new CryptoPP::StreamTransformationFilter(cbcEncryption, new CryptoPP::StringSink(ciphertext)));

        return ciphertext;
    }

    std::string decryptAES(const std::string& ciphertext, const std::string& key) {
        if(key.size() != static_cast<int>(CryptoPP::AES::DEFAULT_KEYLENGTH)){
            log_error("netlist_modifier", "Key needs to be "+std::to_string(static_cast<int>(CryptoPP::AES::DEFAULT_KEYLENGTH))+" bytes long!");
            return "";
        }
        std::string decryptedtext;
        CryptoPP::AES::Decryption aesDecryption((const CryptoPP::byte*)key.data(), CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (const CryptoPP::byte*)key.data());

        CryptoPP::StringSource decryptor(ciphertext, true, new CryptoPP::StreamTransformationFilter(cbcDecryption, new CryptoPP::StringSink(decryptedtext)));

        return decryptedtext;
    }

    bool create_encrypted_zip(){
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

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
        QuaZip zip(QString::fromStdString(project_dir_path / "original/tmp/original.zip"));

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
        netlist_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(netlist_file.fileName()).fileName()));

        netlist_zip_outFile.write(netlist_file.readAll());

        netlist_zip_outFile.close();
        netlist_file.close();
        
        QFile ini_file(QString::fromStdString(project_dir_path / "original/tmp/settings.ini"));
        if (!ini_file.open(QIODevice::ReadOnly)) {
            log_error("netlist_modifier", "Failed to open file!");
            return false;
        }
        QuaZipFile ini_zip_outFile(&zip);
        ini_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(ini_file.fileName()).fileName()));
        
        ini_zip_outFile.write(ini_file.readAll());

        ini_zip_outFile.close();
        ini_file.close();

        zip.close();

        // read content of zip file
        std::ifstream in_file_zip(project_dir_path / "original/tmp/original.zip");
        std::stringstream buffer;
        buffer << in_file_zip.rdbuf();

        std::string key = "0123456789abcdef"; // 16-byte key for AES-128
        std::string enc_content_zip = encryptAES(buffer.str(), key);

        // save back encrypted zip file
        std::ofstream out_file_zip(project_dir_path / "original/original.encrypted");

        // Check if the file is successfully opened
        if (!out_file_zip.is_open()) {
            log_error("netlist_modifier", "Error opening new encrypted zip file!");
            return false;
        }
        out_file_zip << enc_content_zip;
        out_file_zip.close(); // Close the file

        // delete tmp folder
        try{
            std::filesystem::remove(project_dir_path / "original/tmp/settings.ini");
            std::filesystem::remove(project_dir_path / "original/tmp/original.zip");
            std::filesystem::remove(project_dir_path / "original/tmp/original.hal");
            std::filesystem::remove(project_dir_path / "original/tmp");
        } catch (const std::filesystem::filesystem_error& e){
            log_error("netlist_modifier", "Failed to delete tmp directory");
            std::cerr << e.what() << std::endl;
        }

        /*std::filesystem::path tmp_directory(project_dir_path / "original/tmp");
        if (std::filesystem::exists(tmp_directory) && std::filesystem::is_directory(tmp_directory)) {
            try {
                std::filesystem::remove_all(tmp_directory); // Recursively delete directory and its contents
                std::cout << "Tmp directory successfully deleted." << std::endl;
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Failed to delete tmp directory: " << e.what() << std::endl;
            }
        }*/

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

        return true;
    }

    bool NetlistModifierPlugin::modify_in_place()
    {
        if (!modify_gatelibrary()) return false;

        std::vector<Gate*> gates = GuiApi().getSelectedGates();

        // save original netlist if it does not contain any UNKNOWN gates
        bool contains_unknown = false;
        for (Gate* gate: gNetlist->get_gates()){
            if(gate->get_type()->get_name().find("UNKNOWN_") != std::string::npos){
                contains_unknown = true;
            }
        }
        if(!contains_unknown){
            create_encrypted_zip();
        }

        for (Gate* gate : gates)
        {
            if (!replace_gate_in_netlist(gNetlist, gate))
            {
                return false;
            }
        }

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
        // netlist_writer_manager::write(gNetlist, project_dir_path / "generated/generated_netlist_obfuscated.hal");

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

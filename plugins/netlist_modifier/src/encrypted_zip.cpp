#include "netlist_modifier/netlist_modifier.h"

#include "hal_core/netlist/project_manager.h"

#include "hal_core/netlist/persistent/netlist_serializer.h"

#include <sstream>
#include <fstream>

#include <JlCompress.h>

namespace hal
{
    extern Netlist* gNetlist;

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

    std::string generate_ini_content(int probe_type, int probe_limit){
        std::ostringstream ini_oss;
        ini_oss << "[section1]\n"
                << "; select what probe type to use\n"
                << "; 0 = scan chain\n"
                << "; 1 = t probe\n"
                << "probe_type=" << probe_type << "\n"
                << "max_probes=" << probe_limit;
        return ini_oss.str();
    }

    bool NetlistModifierPlugin::create_encrypted_zip(std::string password, int probe_type, int probe_limit, std::string salt, bool existing_hal_file){
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if(salt == ""){
            salt = gen_random_string(20);
        }
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

        // create tmp original netlist file
        if(!existing_hal_file){
            netlist_serializer::serialize_to_file(gNetlist, project_dir_path / "original/original.hal");
        }
        
        // create zip file
        QuaZip zip(QString::fromStdString(project_dir_path / "original/original.zip"));

        if (!zip.open(QuaZip::mdCreate)) {
            log_error("netlist_modifier", "Failed to create ZIP archive!");
            return false;
        }


        QFile netlist_file(QString::fromStdString(project_dir_path / "original/original.hal"));
        if (!netlist_file.open(QIODevice::ReadOnly)) {
            log_error("netlist_modifier", "Failed to open original.hal file!");
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

        // create tmp ini file
        std::string ini_content = generate_ini_content(probe_type, probe_limit);
        
        QuaZipFile ini_zip_outFile(&zip);
        if (!ini_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo("settings.ini"), salted_password.c_str())) {
            log_error("File could not be added with encryption!");
            return false;
        }
        ini_zip_outFile.write(ini_content.c_str());
        ini_zip_outFile.close();

        QuaZipFile salt_zip_outFile(&zip);
        if (!salt_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo("salt.encrypt"), password.c_str())) {
            log_error("File could not be added with encryption!");
            return false;
        }
        
        salt_zip_outFile.write(salt.c_str());
        salt_zip_outFile.close();

        zip.close();

        // delete tmp file
        try{
            std::filesystem::remove(project_dir_path / "original/original.hal");
        } catch (const std::filesystem::filesystem_error& e){
            log_error("netlist_modifier", "Failed to delete tmp file");
            std::cerr << e.what() << std::endl;
            return false;
        }
        
        return true;
    }

    bool NetlistModifierPlugin::update_encrypted_zip(std::string password, int probe_type, int probe_limit){
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        QuaZip zip(QString::fromStdString(project_dir_path / "original/original.zip"));

        if (!zip.open(QuaZip::mdUnzip))
        {
            log_error("netlist_simulator_study", "Zip library cannot handle file, does it exist?");
            return false;
        }

        zip.setCurrentFile("salt.encrypt");
        QuaZipFile zipFile(&zip);
        if (!zipFile.open(QIODevice::ReadOnly, password.c_str())) {
            log_error("netlist_simulator_study", "Failed to open salt.encrypt file");
            zip.close();
            return false;
        }

        QByteArray salt_buffer = zipFile.readAll();
        std::string salt = std::string(salt_buffer.constData(), salt_buffer.size());
        std::string salted_password = gen_salted_password(password, salt);
        zipFile.close();

        zip.setCurrentFile("original.hal");
        if (!zipFile.open(QIODevice::ReadOnly, salted_password.c_str())) {
            log_error("netlist_simulator_study", "Failed to open original.hal file");
            zip.close();
            return false;
        }

        QByteArray hal_buffer = zipFile.readAll();
        std::string original_hal = std::string(hal_buffer.constData(), hal_buffer.size());
        zipFile.close();
        zip.close();

        // create tmp original_hal file
        std::ofstream hal_out_file((project_dir_path / "original/original.hal").c_str());

        // Check if the file is successfully opened
        if (!hal_out_file.is_open()) {
            log_error("netlist_modifier", "Error opening tmp original.hal file!");
            return false;
        }

        // Write the content to the file
        hal_out_file << original_hal;

        // Close the file
        hal_out_file.close();

        // delete tmp file
        try{
            std::filesystem::remove(project_dir_path / "original/original.zip");
        } catch (const std::filesystem::filesystem_error& e){
            log_error("netlist_modifier", "Failed to delete old zip");
            std::cerr << e.what() << std::endl;
            return false;
        }

        return create_encrypted_zip(password, probe_type, probe_limit, salt, true);
    }
}
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/netlist/project_manager.h"
#include "netlist_modifier/netlist_modifier.h"

#include <JlCompress.h>
#include <fstream>
#include <sstream>

/**
 * @brief This file contains the methods to encrypt the original netlist and the ini file into an encrypted zip archive
 * 
 */

namespace hal
{
    /**
     * @brief This function return a random alphanumerical string with a given length
     * 
     * @param len The length of the generated random string
     * @return std::string The generated random string
     */
    std::string gen_random_string(int len)
    {
        // list of allowed symbols
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

    /**
     * @brief This function generates a salted password based on a given password and a salt.
     * 
     * @param password The initial password
     * @param salt The salt that should be used for the salting process
     * @return std::string The salted password
     */
    std::string gen_salted_password(std::string password, std::string salt)
    {
        return password + salt;
    }

    /**
     * @brief This function creates the content of the ini file given the desired settings
     * 
     * @param probe_type The desired probe type. Currently this value can be 0 (scan chain probing) or 1 (t probing)
     * @param probe_limit The desired maximum number of allowed probes
     * @return std::string The generated content of the ini file
     */
    std::string generate_ini_content(int probe_type, int probe_limit)
    {
        std::ostringstream ini_oss;
        ini_oss << "[section1]\n"
                << "; select what probe type to use\n"
                << "; 0 = scan chain\n"
                << "; 1 = t probe\n"
                << "probe_type=" << probe_type << "\n"
                << "max_probes=" << probe_limit;
        return ini_oss.str();
    }

    /**
     * @brief Function to create the encrypted zip archive with a given password and settings like probe-type
     * 
     * @param password The password used for the encryption
     * @param probe_type the probe type that should be stored in the ini file
     * @param probe_limit the probe limit that should be stored in the ini file
     * @param salt (optional) The salt used for encryption. If a empty string is provided as the salt a random one is generated
     * @param existing_hal_file boolean whether or not a copy of the original unaltered hal file exists or if the current netlist needs to be saved
     * @return bool returns true on success
     */
    bool NetlistModifierPlugin::create_encrypted_zip(std::string password, int probe_type, int probe_limit, std::string salt, bool existing_hal_file)
    {
        // get the path to the current project directory
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        // generate salt if no salt is provided
        if (salt == "")
        {
            salt = gen_random_string(20);
        }
        std::string salted_password = gen_salted_password(password, salt);

        // create original folder if missing
        if (std::filesystem::exists(project_dir_path / "original/"))
        {
            if (!std::filesystem::is_directory(project_dir_path / "original/"))
            {
                log_error("netlist_modifier", "A file with the name 'original' already exists.");
                return false;
            }
        }
        else
        {
            if (!std::filesystem::create_directory(project_dir_path / "original/"))
            {
                log_error("netlist_modifier", "Could not create original folder!");
                return false;
            }
        }

        // create tmp original netlist file
        if (!existing_hal_file)
        {
            netlist_serializer::serialize_to_file(gNetlist, project_dir_path / "original/original.hal");
        }

        // create zip file
        QuaZip zip(QString::fromStdString(project_dir_path / "original/original.zip"));

        if (!zip.open(QuaZip::mdCreate))
        {
            log_error("netlist_modifier", "Failed to create ZIP archive!");
            return false;
        }

        // write the original netlist file to the zip archive. Use the salted password for encryption
        QFile netlist_file(QString::fromStdString(project_dir_path / "original/original.hal"));
        if (!netlist_file.open(QIODevice::ReadOnly))
        {
            log_error("netlist_modifier", "Failed to open original.hal file!");
            return false;
        }
        QuaZipFile netlist_zip_outFile(&zip);
        if (!netlist_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QFileInfo(netlist_file.fileName()).fileName()), salted_password.c_str()))
        {
            log_error("File could not be added with encryption!");
            netlist_file.close();
            return false;
        }

        netlist_zip_outFile.write(netlist_file.readAll());

        netlist_zip_outFile.close();
        netlist_file.close();

        // create tmp ini file
        std::string ini_content = generate_ini_content(probe_type, probe_limit);

        // write the ini file to the zip archive. Use the salted password for encryption
        QuaZipFile ini_zip_outFile(&zip);
        if (!ini_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo("settings.ini"), salted_password.c_str()))
        {
            log_error("File could not be added with encryption!");
            return false;
        }
        ini_zip_outFile.write(ini_content.c_str());
        ini_zip_outFile.close();

        // write the used salt to the zip archive. Use the original unsalted password for enryption
        QuaZipFile salt_zip_outFile(&zip);
        if (!salt_zip_outFile.open(QIODevice::WriteOnly, QuaZipNewInfo("salt.encrypt"), password.c_str()))
        {
            log_error("File could not be added with encryption!");
            return false;
        }
        salt_zip_outFile.write(salt.c_str());
        salt_zip_outFile.close();

        zip.close();

        // delete tmp file
        try
        {
            std::filesystem::remove(project_dir_path / "original/original.hal");
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            log_error("netlist_modifier", "Failed to delete tmp file");
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }

    /**
     * @brief This function updates an existing zip archive if just the settings of the ini got changed
     * 
     * @param password The password used for encrypting
     * @param probe_type The probe type that should be stored in the ini file
     * @param probe_limit the probe limit that should be stored in the ini file
     * @return bool Returns true on success
     */
    bool NetlistModifierPlugin::update_encrypted_zip(std::string password, int probe_type, int probe_limit)
    {
        // get the path to the current project directory
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        // open the existing zip archive
        QuaZip zip(QString::fromStdString(project_dir_path / "original/original.zip"));

        if (!zip.open(QuaZip::mdUnzip))
        {
            log_error("netlist_simulator_study", "Zip library cannot handle file, does it exist?");
            return false;
        }

        // retrieve the salt from the zip archive
        zip.setCurrentFile("salt.encrypt");
        QuaZipFile zipFile(&zip);
        if (!zipFile.open(QIODevice::ReadOnly, password.c_str()))
        {
            log_error("netlist_simulator_study", "Failed to open salt.encrypt file");
            return false;
        }

        // generate the salted password that was used to encrypt the other files
        QByteArray salt_buffer      = zipFile.readAll();
        std::string salt            = std::string(salt_buffer.constData(), salt_buffer.size());
        std::string salted_password = gen_salted_password(password, salt);
        zipFile.close();

        // retrieve the original netlist and write it to a tmp file
        zip.setCurrentFile("original.hal");
        if (!zipFile.open(QIODevice::ReadOnly, salted_password.c_str()))
        {
            log_error("netlist_simulator_study", "Failed to open original.hal file");
            zip.close();
            return false;
        }

        QByteArray hal_buffer    = zipFile.readAll();
        std::string original_hal = std::string(hal_buffer.constData(), hal_buffer.size());
        zipFile.close();
        zip.close();

        // create tmp original_hal file
        std::ofstream hal_out_file((project_dir_path / "original/original.hal").c_str());

        // Check if the file is successfully opened
        if (!hal_out_file.is_open())
        {
            log_error("netlist_modifier", "Error opening tmp original.hal file!");
            return false;
        }

        // Write the content to the file
        hal_out_file << original_hal;

        // Close the file
        hal_out_file.close();

        // delete tmp file
        try
        {
            std::filesystem::remove(project_dir_path / "original/original.zip");
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            log_error("netlist_modifier", "Failed to delete old zip");
            std::cerr << e.what() << std::endl;
            return false;
        }

        // create the new encrypted zip archive using the new settings
        return create_encrypted_zip(password, probe_type, probe_limit, salt, true);
    }
}    // namespace hal
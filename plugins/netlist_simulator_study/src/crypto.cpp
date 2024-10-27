#include "netlist_simulator_study/netlist_simulator_study.h"

#include <JlCompress.h>

/**
 * @brief This file handles the functionalities of decrypting password protected zip archives
 * 
 */

namespace hal
{
    /**
     * @brief This function generates a salted password based on a given password and a salt.
     * 
     * @param password The initial password
     * @param salt The salt that should be used for the salting process
     * @return std::string The salted password
     */
    std::string NetlistSimulatorStudyPlugin::gen_salted_password(std::string password, std::string salt)
    {
        return password + salt;
    }

    /**
     * @brief This function returns the content of a specific file from within the password protected zip archive
     * 
     * @param zip_path The path to the password protected zip archive
     * @param password The password to decrypt the zip archive
     * @param filename The filename of the file that should be read
     * @return std::string Return either the content of the specified file or an empty string of no such file exits
     */
    std::string NetlistSimulatorStudyPlugin::read_named_zip_file_decrypted(std::filesystem::path zip_path, std::string password, std::string filename)
    {
        std::vector<std::tuple<std::string, std::string>> result_data = read_all_zip_files_decrypted(zip_path, password, filename);

        if (result_data.size() > 0)
        {
            return std::get<1>(result_data[0]);
        }
        return "";
    }

    /**
     * @brief This function returns the content of all the files from within the password protected zip archive
     * 
     * @param zip_path The path to the password protected zip archive
     * @param password The password to decrypt the zip archive
     * @param filename (optional) If a filename is provided only this file is decrypted. If this is a empty string all the files are decrypted
     * @return std::vector<std::tuple<std::string, std::string>> 
     */
    std::vector<std::tuple<std::string, std::string>> NetlistSimulatorStudyPlugin::read_all_zip_files_decrypted(std::filesystem::path zip_path, std::string password, std::string filename)
    {
        std::vector<std::tuple<std::string, std::string>> read_data;
        std::vector<std::tuple<std::string, std::string>> empty_result;

        // open the zip archive
        QuaZip zip_file(QString::fromStdString(zip_path));

        if (!zip_file.open(QuaZip::mdUnzip))
        {
            log_error("netlist_simulator_study", "Zip library cannot handle file, does it exist?");
            return empty_result;
        }

        if (!zip_file.goToFirstFile())
        {
            log_error("netlist_simulator_study", "Cannot find first compressed file in zip file");
            return empty_result;
        }

        QuaZipFile toExtract(&zip_file);

        // iterate over all the files in the zip archive and decrypt the content
        for (bool more = zip_file.goToFirstFile(); more; more = zip_file.goToNextFile())
        {
            // get file info
            QuaZipFileInfo info;
            if (!zip_file.getCurrentFileInfo(&info))
            {
                log_warning("netlist_simulator_study", "Failed to get current file info");
                if(toExtract.isOpen())
                    toExtract.close();
                continue;
            }

            // if a desired filename is provided check if the current file is the one which is searched
            if (filename != "")
            {
                if (info.name.toStdString() != filename)
                {
                    if(toExtract.isOpen())
                        toExtract.close();
                    continue;
                }
            }

            // decrypt the file and read the content
            if (!toExtract.open(QIODevice::ReadOnly, password.c_str()))
            {
                log_error("netlist_simulator_study", "Cannot unzip file");
                if(toExtract.isOpen())
                    toExtract.close();
                return empty_result;
            }

            // save the content together with the filename in an tuple
            QByteArray data         = toExtract.readAll();
            std::string data_string = std::string(data.constData(), data.size());

            read_data.push_back(std::make_tuple(info.name.toStdString(), data_string));

            if(toExtract.isOpen())
                toExtract.close();
        }

        zip_file.close();

        return read_data;
    }
}    // namespace hal
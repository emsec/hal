#include "netlist_simulator_study/netlist_simulator_study.h"

#include <JlCompress.h>

namespace hal
{
    std::string NetlistSimulatorStudyPlugin::gen_salted_password(std::string password, std::string salt)
    {
        return password + salt;
    }

    std::string NetlistSimulatorStudyPlugin::read_named_zip_file_decrypted(std::filesystem::path zip_path, std::string password, std::string filename)
    {
        std::vector<std::tuple<std::string, std::string>> result_data = read_all_zip_files_decrypted(zip_path, password, filename);

        if (result_data.size() > 0)
        {
            return std::get<1>(result_data[0]);
        }
        return "";
    }

    std::vector<std::tuple<std::string, std::string>> NetlistSimulatorStudyPlugin::read_all_zip_files_decrypted(std::filesystem::path zip_path, std::string password, std::string filename)
    {
        std::vector<std::tuple<std::string, std::string>> read_data;
        std::vector<std::tuple<std::string, std::string>> empty_result;

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

        for (bool more = zip_file.goToFirstFile(); more; more = zip_file.goToNextFile())
        {
            QuaZipFileInfo info;
            if (!zip_file.getCurrentFileInfo(&info))
            {
                log_warning("netlist_simulator_study", "Failed to get current file info");
                if(toExtract.isOpen())
                    toExtract.close();
                continue;
            }

            if (filename != "")
            {
                if (info.name.toStdString() != filename)
                {
                    if(toExtract.isOpen())
                        toExtract.close();
                    continue;
                }
            }

            if (!toExtract.open(QIODevice::ReadOnly, password.c_str()))
            {
                log_error("netlist_simulator_study", "Cannot unzip file");
                if(toExtract.isOpen())
                    toExtract.close();
                return empty_result;
            }

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
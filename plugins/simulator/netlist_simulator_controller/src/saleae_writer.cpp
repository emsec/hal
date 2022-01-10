#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_file.h"
#include <fstream>
#include <sstream>
#include <QDebug>
#include <sys/resource.h>

namespace hal
{
   SaleaeWriter::SaleaeWriter(const std::string& filename)
    {
        std::filesystem::path csvpath(filename);
        mDir = csvpath.parent_path();
        std::filesystem::create_directory(mDir);

        mCsvFile = new std::ofstream(filename);
        if (!mCsvFile->good())
        {
            delete mCsvFile;
            mCsvFile = nullptr;
            return;
        }
        (*mCsvFile) << "<SALEAE>" << std::endl;
    }

    SaleaeWriter::~SaleaeWriter()
    {
        for (SaleaeOutputFile* sof : mDataFiles)
        {
            sof->close();
            delete sof;
        }
        mCsvFile->close();
        delete mCsvFile;
    }

    SaleaeOutputFile* SaleaeWriter::addWaveForm(const std::string& name)
    {
        if (!mCsvFile->good()) return nullptr;
        struct rlimit rlim;
        getrlimit(RLIMIT_NOFILE, &rlim);
        int fileIndex = mDataFiles.size();
        unsigned int required = fileIndex + 256;
        if (rlim.rlim_max < required) return nullptr;
        if (rlim.rlim_cur < required)
        {
            rlim.rlim_cur = required;
            setrlimit(RLIMIT_NOFILE, &rlim);
        }

        std::ostringstream fname;
        fname << "digital_" << fileIndex << ".bin";
        std::filesystem::path path = mDir / fname.str();

        SaleaeOutputFile* sof = new SaleaeOutputFile(path.string(), fileIndex);
        if (!sof->good())
        {
            delete sof;
            return nullptr;
        }

        (*mCsvFile) << fileIndex << ",\"" << name << '"' << std::endl;

        mDataFiles.push_back(sof);

        return sof;
    }


}

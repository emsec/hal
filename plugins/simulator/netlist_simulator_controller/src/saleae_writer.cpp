#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include <fstream>
#include <sstream>
#include <QDebug>
#include <sys/resource.h>

namespace hal
{
   SaleaeWriter::SaleaeWriter(const std::string& filename)
       : mSaleaeDirectory(filename,true)
    {
        std::filesystem::path csvpath(filename);
        mDir = csvpath.parent_path();
        std::filesystem::create_directory(mDir);
    }

    SaleaeWriter::~SaleaeWriter()
    {
        std::vector<SaleaeDirectoryFileIndex> fileIndexes;
        for (SaleaeOutputFile* sof : mDataFiles)
        {
            fileIndexes.push_back(sof->fileIndex());
            sof->close();
            delete sof;
        }
        mSaleaeDirectory.update_file_indexes(fileIndexes);
        mSaleaeDirectory.write_json();
    }

    SaleaeOutputFile* SaleaeWriter::add_or_replace_waveform(const std::string& name, uint32_t id)
    {
        std::filesystem::path path;
        SaleaeOutputFile* sof = nullptr;
        int fileIndex = -1;
        if ( (fileIndex = mSaleaeDirectory.get_datafile_index(name,id)) >= 0)
        {
            // replace existing
            path = mSaleaeDirectory.get_datafile(name, id);
            mDataFiles.at(fileIndex)->close();
            delete mDataFiles.at(fileIndex);
            sof = new SaleaeOutputFile(path.string(), fileIndex);
            if (!sof->good())
            {
                delete sof;
                return nullptr;
            }
            mDataFiles[fileIndex] = sof;
        }
        else
        {
            // add new waveform, might need to increase ulimit
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);

            fileIndex = mDataFiles.size();
            unsigned int required = fileIndex + 256;
            if (rlim.rlim_max < required) return nullptr;
            if (rlim.rlim_cur < required)
            {
                rlim.rlim_cur = required;
                setrlimit(RLIMIT_NOFILE, &rlim);
            }

            std::ostringstream fname;
            fname << "digital_" << fileIndex << ".bin";
            path = mDir / fname.str();

            sof = new SaleaeOutputFile(path.string(), fileIndex);
            if (!sof->good())
            {
                delete sof;
                return nullptr;
            }

            // create directory entry
            SaleaeDirectoryNetEntry sdne(name,id);
            sdne.addIndex(SaleaeDirectoryFileIndex(fileIndex));
            mSaleaeDirectory.add_net(sdne);

            mDataFiles.push_back(sof);

        }
        return sof;
    }


}

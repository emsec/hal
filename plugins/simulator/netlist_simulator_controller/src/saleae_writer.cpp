#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include <fstream>
#include <sstream>
#include <QDebug>
#include <sys/resource.h>
#include <iostream>

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
        std::unordered_map<int,SaleaeDirectoryFileIndex> fileIndexes;
        for (auto it = mDataFiles.begin(); it != mDataFiles.end(); ++it)
        {
            SaleaeOutputFile* sof = it->second;
            fileIndexes.insert(std::make_pair(it->first,sof->fileIndex()));
            sof->close();
            delete sof;
        }
        mSaleaeDirectory.update_file_indexes(fileIndexes);
        mSaleaeDirectory.write_json();
    }

    void SaleaeWriter::add_directory_entry(int inx, const std::string &name, uint32_t id)
    {
        SaleaeDirectoryNetEntry sdne(name,id);
        sdne.addIndex(SaleaeDirectoryFileIndex(inx));
        mSaleaeDirectory.add_or_replace_net(sdne);
    }

    SaleaeOutputFile* SaleaeWriter::add_or_replace_waveform(const std::string& name, uint32_t id)
    {
        std::filesystem::path path;
        SaleaeOutputFile* sof = nullptr;
        bool updateDirectory = false;
        int fileIndex = -1;
        if ( (fileIndex = mSaleaeDirectory.get_datafile_index(name,id)) >= 0)
        {
            // replace existing
            path = mSaleaeDirectory.get_datafile(name, id);
            auto it = mDataFiles.find(fileIndex);
            if (it != mDataFiles.end())
            {
                it->second->close();
                delete it->second;
                mDataFiles.erase(it);
            }
        }
        else
        {
            // add new waveform, might need to increase ulimit
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);

            fileIndex = mSaleaeDirectory.get_next_available_index();
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

            updateDirectory = true;
        }
        sof = new SaleaeOutputFile(path.string(), fileIndex);
        if (!sof->good())
        {
            delete sof;
            return nullptr;
        }

        mDataFiles.insert(std::make_pair(fileIndex,sof));

        if (updateDirectory)
            // create directory entry
            add_directory_entry(fileIndex,name,id);
        return sof;
    }


}

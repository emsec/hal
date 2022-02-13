#ifdef STANDALONE_PARSER
#include "saleae_directory.h"
#else
#include "netlist_simulator_controller/saleae_directory.h"
#include "hal_core/utilities/json_write_document.h"
#endif
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include <iostream>
#include <sstream>

namespace hal
{

    SaleaeDirectory::SaleaeDirectory(const std::string &path, bool create)
        : mDirectoryFile(path), mNextAvailableIndex(0)
    {
        if (create) return;
        parse_json();
        mById.clear();
        mByName.clear();
        int n = mNetEntries.size();
        for (int i=0; i<n; i++)
        {
            const SaleaeDirectoryNetEntry& sdne = mNetEntries.at(i);
            mByName.insert(std::make_pair(sdne.name(),i));
            if (sdne.id()) mById.insert(std::make_pair(sdne.id(),i));
        }
    }

    bool SaleaeDirectory::parse_json()
    {
        FILE* ff = fopen(mDirectoryFile.c_str(), "rb");
        if (!ff) return false;

        mNetEntries.clear();
        mById.clear();
        mByName.clear();

        char buffer[65536];
        rapidjson::FileReadStream frs(ff, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(ff);

        if (document.HasParseError() || !document.HasMember("saleae")) return false;
        auto jsaleae = document["saleae"].GetObject();
        if (jsaleae.HasMember("nets"))
        {
            for (auto& jnet : jsaleae["nets"].GetArray())
            {
                uint32_t id = 0;
                std::string name;
                if (jnet.HasMember("id"))   id   = jnet["id"].GetUint();
                if (jnet.HasMember("name")) name = jnet["name"].GetString();
                SaleaeDirectoryNetEntry sdne(name,id);
                if (jnet.HasMember("indexes"))
                {
                    for (auto& jinx : jnet["indexes"].GetArray())
                    {
                        int      inx  = jinx["index"].GetInt();
                        uint64_t tBeg = jinx["t_beg"].GetUint64();
                        uint64_t tEnd = jinx["t_end"].GetUint64();
                        uint64_t nVal = jinx["n_val"].GetUint64();
                        sdne.addIndex(SaleaeDirectoryFileIndex(inx, tBeg, tEnd, nVal));
                        if (inx >= mNextAvailableIndex) mNextAvailableIndex = inx + 1;
                    }
                }
                int n = mNetEntries.size();
                mNetEntries.push_back(sdne);
                if (id) mById.insert(std::make_pair(id,n));
                mByName.insert(std::make_pair(name,n));
            }
        }

        return true;
    }

#ifndef STANDALONE_PARSER
    bool SaleaeDirectory::write_json() const
    {
        JsonWriteDocument jwd;
        JsonWriteObject& jsaleae = jwd.add_object("saleae");
        JsonWriteArray& jnets = jsaleae.add_array("nets");
        for (const SaleaeDirectoryNetEntry& net : mNetEntries)
        {
            JsonWriteObject& jnet = jnets.add_object();
            jnet["id"] = (int) net.id();
            jnet["name"] = net.name();
            JsonWriteArray& jindexes = jnet.add_array("indexes");
            for (const SaleaeDirectoryFileIndex& inx : net.indexes())
            {
                JsonWriteObject& jinx = jindexes.add_object();
                jinx["index"] = inx.index();
                jinx["t_beg"] = inx.beginTime();
                jinx["t_end"] = inx.endTime();
                jinx["n_val"] = inx.numberValues();
                jinx.close();
            }
            jindexes.close();
            jnet.close();
        }
        jnets.close();
        jsaleae.close();
        return jwd.serialize(mDirectoryFile);
    }
#endif
  
    void SaleaeDirectory::dump() const
    {
        std::cout << "<" << mDirectoryFile << ">" << std::endl;
        for (const SaleaeDirectoryNetEntry& sdne : mNetEntries)
        {
            std::cout << sdne.name() << "[" << sdne.id() << "] :";
            for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
                std::cout << " <" << sdfi.index() << "," << sdfi.beginTime() << "," << sdfi.endTime() << "," << sdfi.numberValues() << ">";
            std::cout << std::endl;
        }
        std::cout << "---------------------" << std::endl;
    }

    void SaleaeDirectory::update_file_indexes(std::unordered_map<int,SaleaeDirectoryFileIndex>& fileIndexes)
    {
        for (SaleaeDirectoryNetEntry& sdne : mNetEntries)
        {
            if (sdne.mFileIndexes.empty()) continue;
            int inx = sdne.mFileIndexes.back().index();
            auto it = fileIndexes.find(inx);
            if (it != fileIndexes.end())
            {
                sdne.mFileIndexes.pop_back();
                sdne.mFileIndexes.push_back(it->second);
            }
        }
    }

    void SaleaeDirectory::add_or_replace_net(SaleaeDirectoryNetEntry& sdne)
    {
        int inetentry = -1;
        if (sdne.id())
        {
            auto it = mById.find(sdne.id());
            if (it != mById.end()) inetentry = it->second;
        }
        if (inetentry<0)
        {
            auto it = mByName.find(sdne.name());
            if (it != mByName.end())
            {
                inetentry = it->second;
                if (sdne.id() > 0)
                    mById.insert(std::make_pair(sdne.id(),inetentry));
            }
        }
        if (inetentry<0)
        {
            int n = mNetEntries.size();
            mByName.insert(std::make_pair(sdne.name(),n));
            if (sdne.id()) mById.insert(std::make_pair(sdne.id(),n));
            mNetEntries.push_back(sdne);
        }
        else
        {
            mNetEntries[inetentry] = sdne;
        }
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
            if (sdfi.index() >= mNextAvailableIndex) mNextAvailableIndex = sdfi.index() + 1;
    }

    std::string SaleaeDirectory::dataFilename(const SaleaeDirectoryNetEntry& sdnep) const
    {
        return get_directory() + folderSeparator + sdnep.dataFilename();
    }

    int SaleaeDirectory::getIndex(const SaleaeDirectoryNetEntry& sdnep) const
    {
        return sdnep.dataFileIndex();
    }

    std::string SaleaeDirectory::get_datafile(const std::string& nam, uint32_t id) const
    {
        if (id)
        {
            auto it = mById.find(id);
            if (it != mById.end()) return dataFilename(mNetEntries.at(it->second));
        }

        auto jt = mByName.find(nam);
        if (jt == mByName.end()) return std::string();
        return dataFilename(mNetEntries.at(jt->second));
    }

    int SaleaeDirectory::get_datafile_index(const std::string &nam, uint32_t id) const
    {
        if (id)
        {
            auto it = mById.find(id);
            if (it != mById.end()) return getIndex(mNetEntries.at(it->second));
        }
        auto jt = mByName.find(nam);
        if (jt == mByName.end()) return -1;
        return getIndex(mNetEntries.at(jt->second));
    }

    uint64_t SaleaeDirectory::get_max_time() const
    {
        uint64_t retval = 0;
        for (const SaleaeDirectoryNetEntry& sdne : mNetEntries)
        {
            for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
            {
                if (sdfi.endTime() > retval) retval = sdfi.endTime();
            }
        }
        return retval;
    }

    std::vector<SaleaeDirectory::ListEntry> SaleaeDirectory::get_net_list() const
    {
        std::vector<ListEntry> retval;
        for (const SaleaeDirectoryNetEntry& sdne : mNetEntries)
        {
            uint64_t size = 0;
            for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
                size += sdfi.numberValues();
            retval.push_back({sdne.id(),sdne.name(),size});
        }
        return retval;
    }

    int SaleaeDirectoryNetEntry::dataFileIndex() const
    {
        if (mFileIndexes.empty()) return -1;
        return mFileIndexes.back().index();
    }

    std::string SaleaeDirectoryNetEntry::dataFilename() const
    {
        if (mFileIndexes.empty()) return std::string();
        std::ostringstream fname;
        fname << "digital_" << mFileIndexes.back().index() << ".bin";
        return fname.str();
    }

    std::string SaleaeDirectory::get_directory() const
    {
        size_t pos = mDirectoryFile.find_last_of(folderSeparator);
        if (pos==std::string::npos) return std::string();
        return mDirectoryFile.substr(0,pos);
    }

    std::string SaleaeDirectory::get_filename() const
    {
        return mDirectoryFile;
    }
}

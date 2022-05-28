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
        : mDirectoryFile(path), mNextAvailableIndex(0), mStoreRequest(0)
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
        if (!ff)
        {
            std::cerr << "cannot open SALEAE directory file <" << mDirectoryFile << ">" << std::endl;
            return false;
        }

        mNetEntries.clear();
        mById.clear();
        mByName.clear();

        char buffer[65536];
        rapidjson::FileReadStream frs(ff, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(ff);

        if (document.HasParseError() || !document.HasMember("saleae"))
        {
            std::cerr << "cannot parse SALEAE directory file <" << mDirectoryFile << ">" << std::endl;
            return false;
        }
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

        // be backward compatible and read groups tag
        if (jsaleae.HasMember("groups"))
        {
            for (auto& jgrp : jsaleae["groups"].GetArray())
            {
                uint32_t gid = 0;
                std::string gname;
                if (jgrp.HasMember("id"))   gid   = jgrp["id"].GetUint();
                if (jgrp.HasMember("name")) gname = jgrp["name"].GetString();
                SaleaeDirectoryComposedEntry sdce(gname,gid,SaleaeDirectoryNetEntry::Group);
                if (jgrp.HasMember("nets"))
                {
                    for (auto& jgnet : jgrp["nets"].GetArray())
                    {
                        uint32_t nid  = jgnet["id"].GetInt();
                        sdce.add_child(nid);
                    }
                }
                mComposedEntryMap[sdce.uniqueKey()] = sdce;
            }
        }

        if (jsaleae.HasMember("composed"))
        {
            for (auto& jcmpsd : jsaleae["composed"].GetArray())
            {
                bool ok = true;
                uint32_t id = 0;
                std::string  name;
                SaleaeDirectoryNetEntry::Type tp = SaleaeDirectoryNetEntry::None;
                if (jcmpsd.HasMember("id"))      id = jcmpsd["id"].GetUint();
                if (jcmpsd.HasMember("name"))    name = jcmpsd["name"].GetString();
                if (jcmpsd.HasMember("type"))    tp = (SaleaeDirectoryNetEntry::Type) jcmpsd["type"].GetUint();
                if (id && tp != SaleaeDirectoryNetEntry::None)
                {
                    SaleaeDirectoryComposedEntry sdce(name,id,tp);
                    if (jcmpsd.HasMember("nets"))
                    {
                        for (auto& jgnet : jcmpsd["nets"].GetArray())
                        {
                            uint32_t nid  = jgnet["id"].GetInt();
                            sdce.add_child(nid);
                        }
                    }
                    if (jcmpsd.HasMember("children"))
                    {
                        for (auto& jchld : jcmpsd["children"].GetArray())
                        {
                            sdce.add_child(jchld.GetInt());
                        }
                    }
                    if (jcmpsd.HasMember("data"))
                    {
                        std::vector<int> vdata;
                        for (auto& jdata : jcmpsd["data"].GetArray())
                            vdata.push_back(jdata.GetInt());
                        sdce.set_data(vdata);
                    }
                    else
                    {
                        if (tp == SaleaeDirectoryNetEntry::Trigger || tp == SaleaeDirectoryNetEntry::Boolean)
                            ok = false;
                    }
                    if (jcmpsd.HasMember("filter"))
                    {
                        sdce.set_filter_entry(jcmpsd["filter"].GetInt());
                    }
                    mComposedEntries.push_back(sdce);
                }
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
        if (!mComposedEntryMap.empty())
        {
            JsonWriteArray& jcmpsds = jsaleae.add_array(("composed"));
            for (auto it=mComposedEntryMap.begin(); it!=mComposedEntryMap.end(); ++it)
            {
                const SaleaeDirectoryComposedEntry& cmpsd = it->second;
                JsonWriteObject& jcmpsd = jcmpsds.add_object();
                jcmpsd["id"] = (int) cmpsd.id();
                jcmpsd["name"] = cmpsd.name();
                jcmpsd["type"] = (int) cmpsd.type();
                JsonWriteArray& jgnets = jcmpsd.add_array("children");
                for (int childKey : cmpsd.get_children())
                {
                    jgnets << childKey;
                }
                jgnets.close();
                if (!cmpsd.get_data().empty())
                {
                    JsonWriteArray& jdata = jcmpsd.add_array("data");
                    for (int dat : cmpsd.get_data())
                        jdata << dat;
                    jdata.close();
                }
                if (cmpsd.get_filter_entry())
                {
                    jcmpsd["filter"] = cmpsd.get_filter_entry();
                }
                jcmpsd.close();
            }
            jcmpsds.close();
        }
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

    std::string SaleaeDirectory::get_datafile_path(int index) const
    {
        return get_directory() + folderSeparator + get_datafile_name(index);
    }

    int SaleaeDirectory::getIndex(const SaleaeDirectoryNetEntry& sdnep) const
    {
        return sdnep.dataFileIndex();
    }

    std::string SaleaeDirectory::get_datafile_path(const std::string& nam, uint32_t id) const
    {
        int index = get_datafile_index(nam, id);
        if (index < 0) return std::string();
        return get_datafile_path(index);
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

    void SaleaeDirectory::rename_net(uint32_t id, const std::string& nam)
    {
#ifndef STANDALONE_PARSER
        SaleaeDirectoryStoreRequest save(this);
#endif
        auto it = mById.find(id);
        if (it == mById.end()) return;
        mNetEntries[it->second].rename(nam);
    }

    std::vector<SaleaeDirectory::ListEntry> SaleaeDirectory::get_net_list() const
    {
        std::vector<ListEntry> retval;
        for (const SaleaeDirectoryNetEntry& sdne : mNetEntries)
        {
            int inx = -1;
            uint64_t size = 0;
            for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
            {
                if (inx < 0) inx = sdfi.index();
                size += sdfi.numberValues();
            }
            retval.push_back({sdne.id(),sdne.name(),inx,size});
        }
        return retval;
    }

    int SaleaeDirectoryNetEntry::dataFileIndex() const
    {
        if (mFileIndexes.empty()) return -1;
        return mFileIndexes.back().index();
    }

    std::string SaleaeDirectory::get_datafile_name(int index) const
    {
        if (index < 0) return std::string();
        std::ostringstream fname;
        fname << "digital_" << index << ".bin";
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

    SaleaeDirectoryComposedEntry::SaleaeDirectoryComposedEntry(const SaleaeDirectoryComposedEntry& other)
        : SaleaeDirectoryNetEntry(other), mChildKeys(other.mChildKeys), mData(other.mData), mFilterEntry(other.mFilterEntry)
    {;}

    SaleaeDirectoryComposedEntry::~SaleaeDirectoryComposedEntry()
    {;}

    void SaleaeDirectoryComposedEntry::set_data(const std::vector<int>& dat)
    {
        mData = dat;
    }

    void SaleaeDirectoryComposedEntry::remove_child(int key)
    {
        auto it = mChildKeys.begin();
        while (it != mChildKeys.end())
        {
            if (*it == key)
                it = mChildKeys.erase(it);
            else
                ++it;
        }
    }

    void SaleaeDirectoryComposedEntry::dump() const
    {
        const char* ctype[] = {"None", "Group", "Boolean", "Trigger" };
        std::cout << ctype[mType] << ": " << mId << " <" << mName << ">\n";
        for (const SaleaeDirectoryNetEntry& sdne : mNetEntries)
            std::cout << "     " << sdne.id() << " <" << sdne.name() << ">\n";
        std::cout << "data:";
        for (int x : mData) std::cout << " " << x;
        std::cout << "\nfilter:";
        if (mFilterEntry)
            std::cout << " " << mFilterEntry->id() << " <" << mFilterEntry->name() << ">\n";
        else
            std::cout << "---\n";
        std::cout << "=============" << std::endl;
    }

    int SaleaeDirectoryNetEntry::uniqueKey() const
    {
        if (!mId) return 0;
        return mType * sComposedBaseKey + mId;
    }

    void SaleaeDirectory::add_or_replace_composed(SaleaeDirectoryComposedEntry sdce)
    {
#ifndef STANDALONE_PARSER
        SaleaeDirectoryStoreRequest save(this);
#endif
        mComposedEntryMap[sdce.uniqueKey()] = sdce;
    }

    void SaleaeDirectory::remove_composed(uint32_t id, SaleaeDirectoryNetEntry::Type tp)
    {
#ifndef STANDALONE_PARSER
        SaleaeDirectoryStoreRequest save(this);
#endif
        SaleaeDirectoryComposedEntry needle("search",id,tp);
        auto it = mComposedEntryMap.find(needle.uniqueKey());
        if (it != mComposedEntryMap.end())
            mComposedEntryMap.erase(it);
    }

    SaleaeDirectoryComposedEntry SaleaeDirectory::get_composed(uint32_t id, SaleaeDirectoryNetEntry::Type tp) const
    {
        SaleaeDirectoryComposedEntry needle("search",id,tp);
        auto it = mComposedEntryMap.find(needle.uniqueKey());
        if (it == mComposedEntryMap.end()) return SaleaeDirectoryComposedEntry();
        return it->second;
    }

    std::vector<SaleaeDirectoryComposedEntry> SaleaeDirectory::get_composed_list() const
    {
        std::vector<SaleaeDirectoryComposedEntry> retval;
        for (auto it=mComposedEntryMap.begin(); it!=mComposedEntryMap.end(); ++it)
            retval.push_back(it->second);
        return retval;
    }


#ifndef STANDALONE_PARSER
    bool SaleaeDirectoryStoreRequest::sWriteDisabled = false;

    SaleaeDirectoryStoreRequest::SaleaeDirectoryStoreRequest(SaleaeDirectory* sd)
        : mSaleaeDirectory(sd)
    {
        ++mSaleaeDirectory->mStoreRequest;
    }

    SaleaeDirectoryStoreRequest::~SaleaeDirectoryStoreRequest()
    {
        if (--mSaleaeDirectory->mStoreRequest <= 0)
        {
            if (!sWriteDisabled)
                mSaleaeDirectory->write_json();
        }
    }
#endif
}

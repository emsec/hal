#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// unfortunately std::filesystem::path is not available for all platforms
#ifdef _WIN32
const char folderSeparator = '\\';
#else
const char folderSeparator = '/';
#endif

namespace hal
{
    class SaleaeDirectory;

#ifndef STANDALONE_PARSER
    class SaleaeDirectoryStoreRequest
    {
    private:
        SaleaeDirectory* mSaleaeDirectory;
        SaleaeDirectoryStoreRequest(const SaleaeDirectoryStoreRequest&) {;} // disable copy constructor
    public:
        SaleaeDirectoryStoreRequest(SaleaeDirectory* sd);
        ~SaleaeDirectoryStoreRequest();
    };
#endif

    class SaleaeDirectoryFileIndex
    {
        int      mIndex;
        uint64_t mBeginTime;
        uint64_t mEndTime;
        uint64_t mNumberValues;
    public:
        SaleaeDirectoryFileIndex(int inx, uint64_t tBeg=0, uint64_t tEnd=0, uint64_t nval=0)
            : mIndex(inx), mBeginTime(tBeg), mEndTime(tEnd), mNumberValues(nval) {;}
        int index() const { return mIndex; }
        uint64_t beginTime() const { return mBeginTime; }
        uint64_t endTime() const { return mEndTime; }
        uint64_t numberValues() const { return mNumberValues; }
    };

    class SaleaeDirectoryNetEntry
    {
        friend class SaleaeDirectory;
        uint32_t   mId;
        std::string mName;
        std::vector<SaleaeDirectoryFileIndex> mFileIndexes;
    public:
        SaleaeDirectoryNetEntry(const std::string nam, uint32_t id_=0)
            : mId(id_), mName(nam) {;}
        uint32_t id() const { return mId; }
        std::string name() const { return mName; }
        const std::vector<SaleaeDirectoryFileIndex>& indexes() const { return mFileIndexes; }
        void addIndex(const SaleaeDirectoryFileIndex& sdfe) { mFileIndexes.push_back(sdfe); }
        void rename(const std::string nam) { mName = nam; }
        int dataFileIndex() const; // TODO : time as argument
    };

    class SaleaeDirectoryGroupEntry
    {
        friend class SaleaeDirectory;
        uint32_t   mId;
        std::string mName;
        std::vector<SaleaeDirectoryNetEntry> mNetEntries;
    public:
        SaleaeDirectoryGroupEntry(const std::string nam, uint32_t id_=0)
            : mId(id_), mName(nam) {;}
        uint32_t id() const { return mId; }
        std::string name() const { return mName; }
        std::vector<SaleaeDirectoryNetEntry>& get_nets() { return mNetEntries; }
        void add_net(SaleaeDirectoryNetEntry sdne) { mNetEntries.push_back(sdne); }
        void remove_net(const SaleaeDirectoryNetEntry& sdne);
        void rename(const std::string nam) { mName = nam; }
        const std::vector<SaleaeDirectoryNetEntry>& get_nets() const { return mNetEntries; }
    };

    class SaleaeDirectory
    {
        friend class SaleaeDirectoryStoreRequest;
    public:
        struct ListEntry
        {
            uint32_t id;
            std::string name;
            int fileIndex;
            uint64_t size;
        };
    private:
        std::string mDirectoryFile;
        std::vector<SaleaeDirectoryNetEntry> mNetEntries;
        std::vector<SaleaeDirectoryGroupEntry> mGroupEntries;

        std::unordered_map<uint32_t,int> mById;
        std::unordered_map<std::string, int> mByName;

        int getIndex(const SaleaeDirectoryNetEntry& sdnep) const;
        int mNextAvailableIndex;
        int mStoreRequest;
#ifndef STANDALONE_PARSER
        bool write_json() const;
#endif
    public:
        SaleaeDirectory(const std::string& path, bool create=false);
        bool parse_json();
        void add_or_replace_net(SaleaeDirectoryNetEntry& sdne);
        void dump() const;
        void update_file_indexes(std::unordered_map<int, SaleaeDirectoryFileIndex>& fileIndexes);
        void rename_net(uint32_t id, const std::string& nam);

        std::string get_datafile_name(int index) const;
        std::string get_datafile_path(int index) const;
        std::string get_datafile_path(const std::string& nam, uint32_t id) const;
        int get_datafile_index(const std::string& nam, uint32_t id) const;

        int get_next_available_index() const { return mNextAvailableIndex; }
        uint64_t get_max_time() const;
        std::string get_directory() const;
        std::string get_filename() const;
        std::vector<ListEntry> get_net_list() const;

        void add_group(SaleaeDirectoryGroupEntry sdge);
        void remove_group(uint32_t group_id);
        SaleaeDirectoryGroupEntry* get_group(uint32_t group_id);
        const std::vector<SaleaeDirectoryGroupEntry>& get_groups() const { return mGroupEntries; }
    };
}

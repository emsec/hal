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
        std::string dataFilename() const; // TODO : time as argument
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
        const std::vector<SaleaeDirectoryNetEntry>& get_nets() const { return mNetEntries; }
        void add_net(const SaleaeDirectoryNetEntry& sdne) { mNetEntries.push_back(sdne); }
    };

    class SaleaeDirectory
    {
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

        std::string dataFilename(const SaleaeDirectoryNetEntry& sdnep) const;
        int getIndex(const SaleaeDirectoryNetEntry& sdnep) const;
        int mNextAvailableIndex;
    public:
        SaleaeDirectory(const std::string& path, bool create=false);
        bool parse_json();
#ifndef STANDALONE_PARSER
        bool write_json() const;
#endif
        void add_or_replace_net(SaleaeDirectoryNetEntry& sdne);
        void dump() const;
        void update_file_indexes(std::unordered_map<int, SaleaeDirectoryFileIndex>& fileIndexes);

        std::string get_datafile(const std::string& nam, uint32_t id) const;
        int get_datafile_index(const std::string& nam, uint32_t id) const;

        int get_next_available_index() const { return mNextAvailableIndex; }
        uint64_t get_max_time() const;
        std::string get_directory() const;
        std::string get_filename() const;
        std::vector<ListEntry> get_net_list() const;
    };
}

// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    /**
     * @brief The SaleaeDirectoryStoreRequest class is useful to bundle requests for updating SALEAE directory.
     * Many operations such as renaming waveform, creating groups, assigning waveform to groups require update
     * of SALEAE directory. To avoid multiple write_json() calls during the same operation it is recommended to
     * create an instance of SaleaeDirectoryStoreRequest rather than calling write_json() directly. When leaving
     * the scope the destructor of the last instance will perform the write operation.
     */
    class SaleaeDirectoryStoreRequest
    {
    private:
        SaleaeDirectory* mSaleaeDirectory;
        SaleaeDirectoryStoreRequest(const SaleaeDirectoryStoreRequest&) {;} // disable copy constructor
    public:
        /**
         * SaleaeDirectoryStoreRequest constructor
         * @param[in] sd The SALEAE directory which needs to be persisted to disk eventually.
         */
        SaleaeDirectoryStoreRequest(SaleaeDirectory* sd);

        /**
         * SaleaeDirectoryStoreRequest destructor. Last destructor on stack will peform write operation.
         */
        ~SaleaeDirectoryStoreRequest();

        static bool sWriteDisabled;
    };
#endif

    /**
     * @brief The SaleaeDirectoryFileIndex class represents a single SALEAE data file.
     * The class comprises the index and header information from binary data file.
     */
    class SaleaeDirectoryFileIndex
    {
        int      mIndex;
        uint64_t mBeginTime;
        uint64_t mEndTime;
        uint64_t mNumberValues;
    public:
        /// Constructor
        SaleaeDirectoryFileIndex(int inx, uint64_t tBeg=0, uint64_t tEnd=0, uint64_t nval=0)
            : mIndex(inx), mBeginTime(tBeg), mEndTime(tEnd), mNumberValues(nval) {;}

        /// Getter for SALEAE data file index.
        int index() const { return mIndex; }

        /// Getter for first time value in SALEAE file
        uint64_t beginTime() const { return mBeginTime; }

        /// Getter for last time value in SALEAE file
        uint64_t endTime() const { return mEndTime; }

        /// Getter for number of transitions + 1 (start value)
        uint64_t numberValues() const { return mNumberValues; }
    };

    /**
     * @brief The SaleaeDirectoryNetEntry class represents a regular waveform for a single simulated net.
     * There is the possibility that no simulation data is available for the net (yet), so the number of
     * associated binary files might be zero. The design allows to split the data into several files. This
     * option however is currently not used.
     * The ID must match the ID of the simulated net. The waveform name can be altered by user using the
     * rename() method.
     */
    class SaleaeDirectoryNetEntry
    {
        friend class SaleaeDirectory;
    public:
        enum Type { None, Group, Boolean, Trigger };
    protected:
        uint32_t   mId;
        std::string mName;
        Type mType;
        std::vector<SaleaeDirectoryFileIndex> mFileIndexes;
    public:
        /// Constructor
        SaleaeDirectoryNetEntry(const std::string nam, uint32_t id_=0, Type tp=None)
            : mId(id_), mName(nam), mType(tp) {;}

        virtual ~SaleaeDirectoryNetEntry() {;}

        /// Getter for waveform ID = simulated net ID
        uint32_t id() const { return mId; }

        /// Getter for waveform name
        std::string name() const { return mName; }

        /// Getter for composed type, None for regular waveform
        Type type() const { return mType; }

        /// Getter for list of associated binary files indexes
        const std::vector<SaleaeDirectoryFileIndex>& indexes() const { return mFileIndexes; }

        /// Add index for binary file to net entry instance
        void addIndex(const SaleaeDirectoryFileIndex& sdfe) { mFileIndexes.push_back(sdfe); }

        /// Rename waveform
        void rename(const std::string nam) { mName = nam; }

        /// Return last data file index (currently there should be no more than one)
        int dataFileIndex() const; // TODO : time as argument

        /// Unique key as reference (used e.g. in mComposedEntryMap)
        int uniqueKey() const;

        static const int sComposedBaseKey = 1 << 16;
    };

    /**
     * @brief The SaleaeDirectoryComposedEntry class represents a composed waveform.
     * There are currently three types of composed waveforms: net groups, boolean combinations and trigger time sets
     * which are all calculated from a set of regular waveform. Additional data like data or filter are dependend
     * of the used type.
     */
    class SaleaeDirectoryComposedEntry : public SaleaeDirectoryNetEntry
    {
        friend class SaleaeDirectory;
    private:
        std::vector<int> mChildKeys;
        std::vector<int> mData;
        int mFilterEntry;
    public:
        SaleaeDirectoryComposedEntry(const std::string nam=std::string(), uint32_t id_=0, Type tp=None)
            : SaleaeDirectoryNetEntry(nam,id_,tp), mFilterEntry(0) {;}
        SaleaeDirectoryComposedEntry(const SaleaeDirectoryComposedEntry& other);
        ~SaleaeDirectoryComposedEntry();

        /// Add an additional child key to composed
        void add_child(int key) { mChildKeys.push_back(key); }

        /// Remove child key from composed
        void remove_child(int key);

        /// Access to child keys
        const std::vector<int>& get_children() const {return mChildKeys; }

        /// Getter for data list which is used  a) Boolean: list of accepted(=true) net combinations b) Trigger: target value when to trigger
        const std::vector<int>& get_data() const { return mData; }

        /// Setter for data list described above
        void set_data(const std::vector<int>& dat);

        /// Only used for trigger: Pointer to filter (suppress trigger unless value 1), null if no filter used
        int get_filter_entry() const { return mFilterEntry; }

        /// Setter for filter pointer described above.
        void set_filter_entry(int filt) { mFilterEntry = filt; }

        /// Test for null
        bool isNull() const { return !mId || !mType; }

        /// Dump to stdout
        void dump() const;
    };

    /**
     * @brief The SaleaeDirectory class provides the API for the SALEAE directory file.
     * The SALEAE directory file with the name "saleae.json" provides the links between waveform
     * data stored in binary SALEAE files and nets from simulated netlist. Additionally composed
     * data like groups, boolean waveforms, or trigger time sets can be persisted in directory
     * file. The file format is JSON.
     */
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
        std::unordered_map<int,SaleaeDirectoryComposedEntry> mComposedEntryMap;

        std::unordered_map<uint32_t,int> mById;
        std::unordered_map<std::string, int> mByName;

        int getIndex(const SaleaeDirectoryNetEntry& sdnep) const;
        int mNextAvailableIndex;
        int mStoreRequest;
#ifndef STANDALONE_PARSER
        bool write_json() const;
#endif
    public:
        /// constructor
        SaleaeDirectory(const std::string& path, bool create=false);

        /// Parse .json file into class instance
        bool parse_json();

        /// Add net entry if not yet existing, replace existing entry otherwise
        void add_or_replace_net(SaleaeDirectoryNetEntry& sdne);

        /// Dump content of class instance to console for debugging purpose
        std::vector<SaleaeDirectoryNetEntry> dump() const;

        /// Update file index information (like number of transitions, end time ...) after writing SALEAE files
        void update_file_indexes(std::unordered_map<int, SaleaeDirectoryFileIndex>& fileIndexes);

        /// Change waveform name entry for net identified by id
        void rename_net(uint32_t id, const std::string& nam);

        /// Get waveform datafile name without path (digital_XXX.bin)
        std::string get_datafile_name(int index) const;

        /// Get full path to waveform data file
        std::string get_datafile_path(int index) const;

        /// Get full path to waveform data file for net identified by name and id
        std::string get_datafile_path(const std::string& nam, uint32_t id) const;

        /// Get waveform datafile index for net identified by name and id
        int get_datafile_index(const std::string& nam, uint32_t id) const;

        /// Getter for next available file index ('digital_XXX.bin' with lowest number XXX not in use)
        int get_next_available_index() const { return mNextAvailableIndex; }

        /// Getter for maximum time
        uint64_t get_max_time() const;

        /// Get path to SALEAE directory without filename
        std::string get_directory() const;

        /// Get full path to saleae directory file including filename (/.../saleae.json)
        std::string get_filename() const;

        /// Getter for all net entries
        std::vector<ListEntry> get_net_list() const;

        /// Add composed waveform entry (Group, Boolean, Trigger)
        void add_or_replace_composed(SaleaeDirectoryComposedEntry sdce);

        /// Remove composed waveform entry identified by id an type
        void remove_composed(uint32_t id, SaleaeDirectoryNetEntry::Type tp);

        /// Getter for pointer to composed waveform entry allowing modifications
        SaleaeDirectoryComposedEntry get_composed(uint32_t id, SaleaeDirectoryNetEntry::Type tp) const;

        /// Getter to list of all composed waveform entries
        std::vector<SaleaeDirectoryComposedEntry> get_composed_list() const;
    };
}

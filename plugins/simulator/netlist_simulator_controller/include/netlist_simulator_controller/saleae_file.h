#pragma once

#include <fstream>
#include <functional>
#ifdef STANDALONE_PARSER
#include "saleae_directory.h"
#else
#include "netlist_simulator_controller/saleae_directory.h"
#endif

namespace hal
{
    class SaleaeStatus
    {
    public:
        /**
         * Error return code for SALEAE file operations
         */
        enum ErrorCode { ErrorOpenFile = -4,
                         BadIndentifier = -3,
                         UnsupportedType = -2,
                         UnexpectedEof = -1,
                         Ok = 0};
    };

    class SaleaeHeader
    {
    public:
        /// SALEAE storage format for transition time values
        enum StorageFormat { Double = 0,          /// Double values,
                             Uint64 = 0x206c6168,
                             Coded = 0x786c6168 };
        char mIdent[9];
        int32_t mVersion;
        StorageFormat mStorageFormat;
        int32_t mValue;
        uint64_t mBeginTime;
        uint64_t mEndTime;
        uint64_t mNumTransitions;

        static const char* sIdent;
    public:
        SaleaeHeader();

        /**
         * Read SALEAE header from open input stream
         * @param ff The file stream to read header from
         * @return Ok=0 on success, negative error code otherwise
         */
        SaleaeStatus::ErrorCode read(std::ifstream& ff);

        /**
         * Write SALEAE header into open output stream
         * @param of The file stream to write header into
         * @return currently always Ok=0
         */
        SaleaeStatus::ErrorCode write(std::ofstream& of) const;

        /**
         * @brief storageFormat
         * @return
         */
        StorageFormat storageFormat() const { return mStorageFormat; }
        void setStorageFormat(StorageFormat sf) { mStorageFormat = sf; }

        int32_t value() const { return mValue; }
        void setValue(int32_t v) { mValue = v; }

        uint64_t beginTime() const { return mBeginTime; }
        void setBeginTime(uint64_t t) { mBeginTime = t; }

        uint64_t endTime() const { return mEndTime; }
        void setEndTime(uint64_t t) { mEndTime = t; }

        uint64_t numTransitions() const { return mNumTransitions; }
        void setNumTransitions(uint64_t n) { mNumTransitions = n; }
        void incrementTransitions() { ++mNumTransitions; }
    };

    class SaleaeDataBuffer
    {
    public:
        uint64_t mCount;
        uint64_t* mTimeArray;
        int* mValueArray;
        bool isNull() const { return mCount == 0; }
        SaleaeDataBuffer(uint64_t cnt = 0);
        ~SaleaeDataBuffer();
        void convertCoded();
        void dump() const;
    };

    class SaleaeDataTuple
    {
    public:
        static const int sReadError = -99;
        uint64_t mTime;
        int mValue;
        bool readError() const { return mValue == sReadError; }
    };

    class SaleaeInputFile : public std::ifstream
    {
        SaleaeHeader mHeader;
        uint64_t mNumRead;
        SaleaeStatus::ErrorCode mStatus;

        std::function<uint64_t(bool*)> mReader;

    public:
        SaleaeInputFile(const std::string& filename);
        int startValue() const { return mHeader.value(); }

        SaleaeDataTuple nextValue(int lastValue);
        std::string get_last_error() const;
        SaleaeDataBuffer get_data();

        const SaleaeHeader* header() const { return &mHeader; }
    };


    class SaleaeOutputFile : public std::ofstream
    {
        int mIndex;
        std::string mFilename;
        SaleaeHeader mHeader;
        SaleaeStatus::ErrorCode mStatus;
        bool mFirstValue;
        int mLastWrittenValue;
        uint64_t mLastWrittenTime;

        void convertToCoded();
    public:
        SaleaeOutputFile(const std::string& filename, int index_);
        ~SaleaeOutputFile();
        void writeTimeValue(uint64_t t, int32_t val);
        void close();
        int index() const { return mIndex; }

        void put_data(SaleaeDataBuffer& buf);

        SaleaeDirectoryFileIndex fileIndex() const;
    };
}

#pragma once

#include <fstream>
#include <functional>

namespace hal
{
    class SaleaeStatus
    {
    public:
        enum ErrorCode { ErrorOpenFile = -4,
                         BadIndentifier = -3,
                         UnsupportedType = -2,
                         UnexpectedEof = -1,
                         Ok = 0};
    };

    class SaleaeHeader
    {

        char mIdent[9];
        int32_t mVersion;
        int32_t mType;
        uint32_t mValue;
        uint64_t mBeginTime;
        uint64_t mEndTime;
        uint64_t mNumTransitions;

        static const char* sIdent;
    public:
        SaleaeHeader();

        SaleaeStatus::ErrorCode read(std::ifstream& ff);
        SaleaeStatus::ErrorCode write(std::ofstream& of) const;

        bool hasIntValues() const;

        uint32_t value() const { return mValue; }
        void setValue(uint32_t v) { mValue = v; }

        uint64_t beginTime() const { return mBeginTime; }
        void setBeginTime(uint64_t t) { mBeginTime = t; }

        uint64_t endTime() const { return mEndTime; }
        void setEndTime(uint64_t t) { mEndTime = t; }

        uint64_t numTransitions() const { return mNumTransitions; }
        void incrementTransitions() { ++mNumTransitions; }
    };

    class SaleaeInputFile : public std::ifstream
    {
        SaleaeHeader mHeader;
        uint64_t mNumRead;
        SaleaeStatus::ErrorCode mStatus;

        std::function<uint64_t()> mReader;

    public:
        SaleaeInputFile(const std::string& filename);
        int startValue() const { return mHeader.value(); }

        uint64_t nextTimeValue();
        std::string get_last_error() const;

        const SaleaeHeader* header() const { return &mHeader; }
    };


    class SaleaeOutputFile : public std::ofstream
    {
        int mIndex;
        SaleaeHeader mHeader;
        SaleaeStatus::ErrorCode mStatus;
        bool mFirstValue;
    public:
        SaleaeOutputFile(const std::string& filename, int index_);
        ~SaleaeOutputFile();
        void writeTimeValue(uint64_t t, int32_t val);
        void close();
        int index() const { return mIndex; }
    };
}

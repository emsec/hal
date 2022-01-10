#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

namespace hal
{

    const char* SaleaeHeader::sIdent = "<SALEAE>";
    SaleaeHeader::SaleaeHeader()
        : mVersion(0), mType(0x206c6168), // "hal "
          mValue(0), mBeginTime(0), mEndTime(0), mNumTransitions(0)
    {
        strcpy(mIdent,sIdent);
    }

    SaleaeStatus::ErrorCode SaleaeHeader::read(std::ifstream& ff)
    {
        int32_t intType = mType;
        ff.read(mIdent,8);
        mIdent[8] = 0;
        if (mIdent != std::string(sIdent))
            return SaleaeStatus::BadIndentifier;

        ff.read((char*)&mVersion,sizeof(mVersion));
        ff.read((char*)&mType,sizeof(mVersion));

        if (mType != 0 && mType != intType)
            return SaleaeStatus::UnsupportedType;

        ff.read((char*)&mValue,sizeof(mValue));

        if (hasIntValues())
        {
            ff.read((char*)&mBeginTime,sizeof(mBeginTime));
            ff.read((char*)&mEndTime,sizeof(mEndTime));
        }
        else
        {
            double tmp;
            ff.read((char*)&tmp,sizeof(tmp));
            mBeginTime = floor(tmp * SaleaeParser::sTimeScaleFactor + 0.5);
            ff.read((char*)&tmp,sizeof(tmp));
            mEndTime = floor(tmp * SaleaeParser::sTimeScaleFactor + 0.5);
        }
        ff.read((char*)&mNumTransitions,sizeof(mNumTransitions));

        return SaleaeStatus::Ok;
    }

    SaleaeStatus::ErrorCode SaleaeHeader::write(std::ofstream& of) const
    {
        of.write(mIdent,8);
        of.write((char*)&mVersion,sizeof(mVersion));
        of.write((char*)&mType,sizeof(mVersion));
        of.write((char*)&mValue,sizeof(mValue));
        of.write((char*)&mBeginTime,sizeof(mBeginTime));
        of.write((char*)&mEndTime,sizeof(mEndTime));
        of.write((char*)&mNumTransitions,sizeof(mNumTransitions));
        return SaleaeStatus::Ok;
    }

    bool SaleaeHeader::hasIntValues() const
    {
        return (mType != 0);
    }

    SaleaeInputFile::SaleaeInputFile(const std::string &filename)
        : std::ifstream(filename, std::ios::binary), mNumRead(0),
          mStatus(SaleaeStatus::Ok)
    {
        if (good())
            mStatus = mHeader.read(*this);
        else
            mStatus = SaleaeStatus::ErrorOpenFile;

        if (mStatus)
            setstate(failbit);

        if (mHeader.hasIntValues())
            mReader = [this]() {
                uint64_t val;
                this->read((char*)&val,sizeof(val));
                return val;
            };
        else
            mReader = [this]() {
                double val;
                this->read((char*)&val,sizeof(val));
                return (uint64_t) floor(val * SaleaeParser::sTimeScaleFactor + 0.5) - this->mHeader.beginTime();
            };
        // printf("<%s> %d %d %d %.7f %.7f %lu\n", mIdent, mVersion, mType, mValue, mBeginTime, mEndTime, mNumTransitions );
    }


    std::string SaleaeInputFile::get_last_error() const
    {
        switch (mStatus) {
        case SaleaeStatus::ErrorOpenFile:
            return "Error opening SALEAE file";
        case SaleaeStatus::BadIndentifier:
            return "No <SALEAE> identifier found";
        case SaleaeStatus::UnsupportedType:
            return "Expected SALEAE type 0 (digital data)";
        default:
            break;
        }
        return std::string();
    }

    uint64_t SaleaeInputFile::nextTimeValue()
    {
        uint64_t tval = mReader();
        ++mNumRead;
        if (mNumRead >= mHeader.numTransitions()) setstate(eofbit);
        return tval;
    }

    SaleaeOutputFile::SaleaeOutputFile(const std::string &filename, int index_)
        : std::ofstream(filename, std::ios::binary), mIndex(index_), mStatus(SaleaeStatus::Ok)
    {
        if (!good())
            mStatus = SaleaeStatus::ErrorOpenFile;
        else
            mHeader.write(*this);
    }

    void SaleaeOutputFile::writeTimeValue(uint64_t t, int32_t val)
    {
        if (mFirstValue)
        {
            mHeader.setValue(val);
            mHeader.setBeginTime(t);
            mFirstValue = false;
        }
        else
        {
            write((char*)&t,sizeof(t));
            mHeader.incrementTransitions();
            mHeader.setEndTime(t);
        }
    }

    SaleaeOutputFile::~SaleaeOutputFile()
    {
        if (good()) close();
    }

    void SaleaeOutputFile::close()
    {
        if (!good()) return;
        seekp(std::ios_base::beg);
        mHeader.write(*this);
        std::ofstream::close();
    }
}

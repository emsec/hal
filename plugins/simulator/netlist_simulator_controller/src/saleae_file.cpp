#ifdef STANDALONE_PARSER
#include "saleae_file.h"
#include "saleae_parser.h"
#else
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"
#endif

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>

namespace hal
{

    const char* SaleaeHeader::sIdent = "<SALEAE>";

    SaleaeDataBuffer::SaleaeDataBuffer(uint64_t cnt)
        : mCount(cnt), mTimeArray(nullptr), mValueArray(nullptr)
    {
        if (mCount)
        {
            mTimeArray = new uint64_t[mCount];
            mValueArray = new int[mCount];
        }
    }

    SaleaeDataBuffer::~SaleaeDataBuffer()
    {
        if (mTimeArray)  delete [] mTimeArray;
        if (mValueArray) delete [] mValueArray;
    }

    SaleaeHeader::SaleaeHeader()
        : mVersion(0), mStorageFormat(Uint64),
          mValue(-1), mBeginTime(0), mEndTime(0), mNumTransitions(0)
    {
        strcpy(mIdent,sIdent);
    }

    SaleaeStatus::ErrorCode SaleaeHeader::read(std::ifstream& ff)
    {
        ff.read(mIdent,8);
        mIdent[8] = 0;
        if (mIdent != std::string(sIdent))
            return SaleaeStatus::BadIndentifier;

        ff.read((char*)&mVersion,sizeof(mVersion));

        uint32_t type;
        ff.read((char*)&type,sizeof(type));

        switch (type)
        {
        case Double:
        case Uint64:
        case Coded:
            mStorageFormat = (StorageFormat) type;
            break;
        default:
            return SaleaeStatus::UnsupportedType;
        }

        ff.read((char*)&mValue,sizeof(mValue));

        if (mStorageFormat != Double)
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
        uint32_t type = mStorageFormat;
        of.write(mIdent,8);
        of.write((char*)&mVersion,sizeof(mVersion));
        of.write((char*)&type,sizeof(type));
        of.write((char*)&mValue,sizeof(mValue));
        of.write((char*)&mBeginTime,sizeof(mBeginTime));
        of.write((char*)&mEndTime,sizeof(mEndTime));
        of.write((char*)&mNumTransitions,sizeof(mNumTransitions));
        return SaleaeStatus::Ok;
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

        switch (mHeader.storageFormat())
        {
        case SaleaeHeader::Double:
            mReader = [this](bool* ok) {
                double timeVal;
                if (this->read((char*)&timeVal,sizeof(timeVal)))
                    *ok = true;
                else
                    *ok = false;
                return (uint64_t) floor(timeVal * SaleaeParser::sTimeScaleFactor + 0.5) - this->mHeader.beginTime();
            };
            break;
        case SaleaeHeader::Uint64:
        case SaleaeHeader::Coded:
            mReader = [this](bool* ok) {
                uint64_t timeVal;
                if (this->read((char*)&timeVal,sizeof(timeVal)))
                    *ok = true;
                else
                    *ok = false;
                return timeVal;
            };
            break;
        }

        // printf("<%s> %d %d %d %.7f %.7f %lu\n", mIdent, mVersion, mType, mValue, mBeginTime, mEndTime, mNumTransitions );
    }

    SaleaeDataBuffer* SaleaeInputFile::get_buffered_data()
    {
        int n = mHeader.numTransitions();
        int val = mHeader.value();
        SaleaeDataBuffer* retval = new SaleaeDataBuffer;

        retval->mCount = n + 1;
        retval->mTimeArray  = new uint64_t[retval->mCount];
        retval->mValueArray = new int[retval->mCount];
        retval->mTimeArray[0] = mHeader.beginTime();
        retval->mValueArray[0] = val;
        if (!n) return retval;
        switch (mHeader.storageFormat())
        {
        case SaleaeHeader::Double:
        {
            double* tmp = new double[n];
            this->read((char*)tmp,sizeof(double)*n);
            for (int i=0; i<n; i++)
            {
                val = val ? 0 : 1;
                retval->mValueArray[i+1] = val;
                retval->mTimeArray[i+1] = floor(tmp[i] * SaleaeParser::sTimeScaleFactor + 0.5) - mHeader.beginTime();
            }
            delete [] tmp;
        }
            break;
        case SaleaeHeader::Uint64:
            this->read((char*)(retval->mTimeArray+1),sizeof(uint64_t)*n);
            for (int i=0; i<n; i++)
            {
                val = val ? 0 : 1;
                retval->mValueArray[i+1] = val;
            }
            break;
        case SaleaeHeader::Coded:
            this->read((char*)(retval->mTimeArray+1),sizeof(uint64_t)*n);
            for (int i=0; i<n; i++)
            {
                retval->mValueArray[i+1] = ((retval->mTimeArray[i+1] >> 62) & 0x3) -2;
                retval->mTimeArray[i+1] &= 0x3fffffffffffffffull;
            }
            break;
        }

        return retval;
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

    SaleaeDataTuple SaleaeInputFile::nextValue(int lastValue)
    {
        SaleaeDataTuple retval;
        bool ok;
        retval.mTime = mReader(&ok);
        if (!ok)
        {
            retval.mValue = SaleaeDataTuple::sReadError;
            return retval;
        }
        ++mNumRead;
        if (mNumRead >= mHeader.numTransitions()) setstate(eofbit);

        if (mHeader.storageFormat() == SaleaeHeader::Coded)
        {
            retval.mValue = ((retval.mTime >> 62) & 0x3) - 2;
            retval.mTime  &= 0x3fffffffffffffffull;
        }
        else
            retval.mValue = lastValue ? 0 : 1;
        return retval;
    }

    int SaleaeInputFile::get_int_value(double t)
    {
        uint64_t max = mHeader.numTransitions();
        uint64_t t0 = mHeader.beginTime();
        uint64_t t1 = mHeader.endTime();
        if (t < t0) return -1;
        if (t == t0) return mHeader.value();
        if (t > t1) return (max%2==0) ? mHeader.value() : 1 - mHeader.value();

        uint64_t tuple1;
        int64_t pos   = 0;
        int64_t delta = max > 4 ? max / 4 : 1;
        bool loop = true;
        bool ok = true;
        while (loop)
        {
            pos += delta;
            if (pos < 0) pos = 0;
            if (pos >= (int64_t) max-1) pos = max-1;
            if (pos)
            {
                seekTransition(pos-1);
                t0 = mReader(&ok) & 0x3fffffffffffffffull;
            }
            else
            {
                seekTransition(pos);
                t0 = mHeader.beginTime();
            }
            tuple1 = mReader(&ok);
            t1 = tuple1 & 0x3fffffffffffffffull;
            if (t0 < t && t1 >= t)
            {
                if (mHeader.storageFormat() == SaleaeHeader::Coded)
                    return ((tuple1 >> 62) & 0x3) - 2;
                else
                    return (pos%2==0) ? mHeader.value() : 1 - mHeader.value();
            }
            else if (t1 < t)
            {
                if (delta < 0) delta = - delta/2;
                if (!delta) delta = 1;
            }
            else if (t0 > t)
            {
                if (delta > 0) delta = - delta/2;
                if (!delta) delta = -1;
            }
        }
        return -1;
    }

    SaleaeOutputFile::SaleaeOutputFile(const std::string &filename, int index_)
        : std::ofstream(filename, std::ios::binary), mIndex(index_), mFilename(filename), mStatus(SaleaeStatus::Ok),
          mFirstValue(true), mLastWrittenValue(0), mLastWrittenTime(0)
    {
        if (!good())
            mStatus = SaleaeStatus::ErrorOpenFile;
        else
            mHeader.write(*this);
    }

    void SaleaeOutputFile::convertToCoded()
    {
        flush();
        seekp(std::ios_base::beg);
        mHeader.write(*this);
        flush();
        SaleaeInputFile sif(mFilename);
        SaleaeDataBuffer* sdf = sif.get_buffered_data();
        sif.close();
        seekp(std::ios_base::beg);
        mHeader.setStorageFormat(SaleaeHeader::Coded);
        mHeader.write(*this);
        put_data(sdf);
        delete sdf;
    }

    void SaleaeOutputFile::put_data(SaleaeDataBuffer *buf)
    {
        if (!buf->mCount) return;
        SaleaeHeader::StorageFormat sf = SaleaeHeader::Uint64;
        for (uint64_t i = 0; i<buf->mCount; i++)
        {
            if (buf->mValueArray[i] < 0)
            {
                sf = SaleaeHeader::Coded;
                break;
            }
        }
        uint64_t n = buf->mCount - 1;
        mHeader.setStorageFormat(sf);
        mHeader.setValue(buf->mValueArray[0]);
        mHeader.setEndTime(buf->mTimeArray[n]);
        mHeader.setNumTransitions(n);
        if (sf == SaleaeHeader::Coded)
            buf->convertCoded();
        this->write((char*) (buf->mTimeArray+1), n * sizeof(uint64_t));
        // close will write header info
    }

    void SaleaeOutputFile::writeTimeValue(uint64_t t, int32_t val)
    {
        if (mFirstValue)
        {
            mHeader.setValue(val);
            mHeader.setBeginTime(t);
            mHeader.setEndTime(t);
            mFirstValue = false;
            mLastWrittenValue = val;
            mLastWrittenTime = t;
        }
        else
        {
            if (val < 0 && mHeader.storageFormat() == SaleaeHeader::Uint64)
                convertToCoded();

            if(t < mLastWrittenTime || val == mLastWrittenValue) return;
            mLastWrittenValue = val;
            mLastWrittenTime  = t;

            if (mHeader.storageFormat() == SaleaeHeader::Coded)
            {
                uint64_t buf = val + 2;
                buf <<= 62;
                buf |= (t & 0xfffffffffffffffull);
                write((char*)&buf,sizeof(buf));
            }
            else
                write((char*)&t,sizeof(t));
            mHeader.incrementTransitions();
            mHeader.setEndTime(t);
        }
    }

    SaleaeOutputFile::~SaleaeOutputFile()
    {
        //  qDebug()  << good() << "~SaleaeOutputFile" << hex << (quintptr) this;
        if (good()) close();
    }

    void SaleaeOutputFile::close()
    {
        if (!good()) return;
        seekp(std::ios_base::beg);
        mHeader.write(*this);
        std::ofstream::close();
    }

    SaleaeDirectoryFileIndex SaleaeOutputFile::fileIndex() const
    {
        return SaleaeDirectoryFileIndex(
                    mIndex,
                    mHeader.beginTime(),
                    mHeader.endTime(),
                    mHeader.numTransitions()+1);
    }

    void SaleaeDataBuffer::convertCoded()
    {
        for (uint64_t i = 1; i<mCount; i++)
        {
            uint64_t mask = mValueArray[i] + 2;
            mask <<= 62;
            mTimeArray[i] = (mTimeArray[i] & 0xfffffffffffffffull) | mask;
        }
    }

    void SaleaeDataBuffer::dump() const
    {
        std::cout << mCount << " :";
        for (uint64_t i = 0; i<mCount; i++)
        {
            std::cout << " <" << mTimeArray[i] << "," << (mValueArray[i] < 0 ? 'X' : ((char) ('0'+mValueArray[i]))) << ">";
        }
        std::cout << std::endl;
    }
}

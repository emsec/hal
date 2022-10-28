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

        /// Getter for storage format, see above
        StorageFormat storageFormat() const { return mStorageFormat; }

        /// Setter for storage format, see above
        void setStorageFormat(StorageFormat sf) { mStorageFormat = sf; }

        /// Getter for initial value
        int32_t value() const { return mValue; }

        /// Setter for initial value
        void setValue(int32_t v) { mValue = v; }

        /// Getter for time of first event
        uint64_t beginTime() const { return mBeginTime; }

        /// Setter for time of first event
        void setBeginTime(uint64_t t) { mBeginTime = t; }

        /// Getter for time of last event
        uint64_t endTime() const { return mEndTime; }

        /// Setter for time of last event
        void setEndTime(uint64_t t) { mEndTime = t; }

        /// Getter for number of transitions stored in file. Remember that the total number of events is numTransitions()+1 (start value).
        uint64_t numTransitions() const { return mNumTransitions; }

        /// Setter for number of transitions stored in file.
        void setNumTransitions(uint64_t n) { mNumTransitions = n; }

        /// Increment number of transitions by one.
        void incrementTransitions() { ++mNumTransitions; }
    };

    class SaleaeDataBuffer
    {
    public:
        /// Constructor. Generates null instance when no argument is given.
        SaleaeDataBuffer(uint64_t cnt = 0);

        /// Destructor takes care of disposing allocated memory.
        ~SaleaeDataBuffer();

        /// Number of elements in buffer
        uint64_t mCount;

        /// Buffer for <nCount> transition time values
        uint64_t* mTimeArray;

        /// Buffer for <nCount> data values
        int* mValueArray;

        /// Returns whether buffer has data. Will be false when e.g. reading behind EOF
        bool isNull() const { return mCount == 0; }

        /// Convert data buffer from "hal " format to "halx"
        void convertCoded();

        /// Dump buffer content to stdout
        void dump() const;
    };

    class SaleaeDataTuple
    {
    public:
        /// Constructor. Generates null/read error instance when no arguments are given.
        SaleaeDataTuple(uint64_t t=0, int val=sReadError) : mTime(t), mValue(val) {;}

        /// Fake data value to indicate all kind of errors
        static const int sReadError = -99;

        /// Transition time
        uint64_t mTime;

        /// Data value
        int mValue;

        /// Indicates that there was an error and no data was returned
        bool readError() const { return mValue == sReadError; }
    };

    class SaleaeInputFile : public std::ifstream
    {
        SaleaeHeader mHeader;
        uint64_t mReadPointer;
        SaleaeStatus::ErrorCode mStatus;

        std::function<uint64_t(bool*)> mReader;

        void seekTransition(uint64_t pos) { seekg(pos*sizeof(uint64_t) + 44); }
    public:
        SaleaeInputFile(const std::string& filename);

        /// Getter for next (time,value) tuple in sequential read
        SaleaeDataTuple get_next_value();

        /// Get verbose error message based on internal status, empty string if no error
        std::string get_last_error() const;

        /// Returns pointer to data buffer reading from current file position up to <nread> events. It is the callers responsibility to dispose the buffer eventually.
        SaleaeDataBuffer* get_buffered_data(uint64_t nread);


        int64_t get_file_position(double t, bool successor=false);

        /// Goto position for next read access. Pos=0 is start value from header.
        void set_file_position(int64_t pos);

        /// Goto position for next read access, skip number of transitions indicated by 'delta', delta can be negative
        void skip_transitions(int64_t delta);

        /// Get waveform value for time t
        int get_int_value(double t);

        /// Getter for header information
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

        /// Write single data tuple to disk
        void writeTimeValue(uint64_t t, int32_t val);

        /// Update header on disk and close file
        void close();

        /// Getter for data file index (XXX in digital_XXX.bin)
        int index() const { return mIndex; }

        /// Write buffered data
        void put_data(SaleaeDataBuffer* buf);

        /// File index summary for saleae directory
        SaleaeDirectoryFileIndex fileIndex() const;
    };
}

//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <QObject>
#include <QList>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QDir>

#include <unordered_map>
#include "hal_core/defines.h"

namespace hal {

    class Net;
    class WaveData;
    class WaveSaleaeData;
    class SaleaeOutputFile;
    class SaleaeWriter;

    class VcdSerializerElement
    {
        int mIndex;
        const WaveData* mData;
        u64 mTime;
        int mValue;
    public:
        VcdSerializerElement(int inx, const WaveData* wd);
        u64 time() const {return mTime; }
        int value() const { return mValue; }
        void setEvent(u64 t, int val) { mTime = t; mValue = val; }
        bool hasData() const;
        void reset();
        QString name() const;
        QByteArray charCode() const;
    };

    class VcdSerializer : public QObject
    {
        Q_OBJECT
        int mTime;
        u64 mFirstTimestamp;
        u64 mLastTimestamp;
        u64 mTimeShift;
        QMap<QString,SaleaeOutputFile*> mSaleaeFiles;
        SaleaeWriter* mSaleaeWriter;
        QList<VcdSerializerElement*> mWriteElements;
        QString mWorkdir;
        QString mSaleaeDirectoryFilename;
        QMap<QString,QString> mAbbrevByName;
        QVector<int> mLastValue;
        int mErrorCount[9];
        bool mSaleae;
        int mLastProgress;

        bool parseVcdDataline(char* buf, int len);
        bool parseVcdDataNonDecimal(const QByteArray& line, int base);
        void storeValue(int val, const QByteArray& abrev);
        bool parseCsvHeader(char* buf);
        bool parseCsvDataline(char* buf, int dataLineIndex);
        bool parseVcdInternal(QFile& ff, const QList<const Net *>& onlyNets);
        bool parseCsvInternal(QFile& ff, const QList<const Net *>& onlyNets);

        void writeVcdEvent(QFile& of);

        void deleteFiles();
        void createSaleaeDirectory();
        void emitProgress(double step, double max);
        void emitImportDone();

    public:
        VcdSerializer(const QString& workdir=QString(), bool saleae_cli=false, QObject* parent = nullptr);
        std::string get_saleae_directory_filename() const { return mSaleaeDirectoryFilename.toStdString(); }
        bool exportVcd(const QString& filename, const QList<const WaveData*>& waves, u32 startTime, u32 endTime, u32 timeShift=0);
        bool importVcd(const QString& vcdFilename, const QString& workdir=QString(), const QList<const Net*>& onlyNets = QList<const Net*>());
        bool importCsv(const QString& csvFilename, const QString& workdir=QString(), const QList<const Net*>& onlyNets = QList<const Net*>(), u64 timeScale = 1000000000);
        bool importSaleae(const QString& saleaeDirecotry, const std::unordered_map<Net*,int>& lookupTable, const QString& workdir=QString(),  u64 timeScale = 1000000000);
//        QList<WaveData*> waveList() const { return mWaves.values(); }
        u64 maxTime() const { return mTime; }
    };

}

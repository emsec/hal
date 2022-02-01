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
        QMap<u64,int>::const_iterator mIterator;
    public:
        VcdSerializerElement(int inx, const WaveData* wd);
        void hasNextElement() const;
        int time() const;
        int value() const;
        bool next();
        int priorityCode(int n) const;
        QString name() const;
        QByteArray charCode() const;
    };

    class VcdSerializer : public QObject
    {
        Q_OBJECT
        u64 mTime;
        u64 mFirstTimestamp;
        QMap<QString,SaleaeOutputFile*> mSaleaeFiles;
        SaleaeWriter* mSaleaeWriter;
        QString mWorkdir;
        QString mSaleaeDirectoryFilename;
        QMap<QString,QString> mAbbrevByName;
        QVector<int> mLastValue;
        int mErrorCount[9];
        bool mSaleae;

        bool parseVcdDataline(char* buf, int len);
        bool parseVcdDataNonDecimal(const QByteArray& line, int base);
        void storeValue(int val, const QByteArray& abrev);
        bool parseCsvHeader(char* buf);
        bool parseCsvDataline(char* buf, int dataLineIndex);
        bool parseVcdInternal(QFile& ff, const QList<const Net *>& onlyNets);
        bool parseCsvInternal(QFile& ff, const QList<const Net *>& onlyNets);

        void deleteFiles();
        void createSaleaeDirectory();

    Q_SIGNALS:
        void importDone();

    public:
        std::string get_saleae_directory_filename() const { return mSaleaeDirectoryFilename.toStdString(); }
        VcdSerializer(QObject* parent = nullptr);
        bool serialize(const QString& filename, const QList<const WaveData*>& waves) const;
        bool importVcd(const QString& vcdFilename, const QString& workdir=QString(), const QList<const Net*>& onlyNets = QList<const Net*>());
        bool importCsv(const QString& csvFilename, const QString& workdir=QString(), const QList<const Net*>& onlyNets = QList<const Net*>(), u64 timeScale = 1000000000);
        bool importSaleae(const QString& saleaeDirecotry, const std::unordered_map<Net*,int>& lookupTable, const QString& workdir=QString(),  u64 timeScale = 1000000000);
//        QList<WaveData*> waveList() const { return mWaves.values(); }
        u64 maxTime() const { return mTime; }
    };

}

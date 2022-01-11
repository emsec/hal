#pragma once

#include <QObject>
#include <QList>
#include <QVector>
#include <QMap>
#include "hal_core/defines.h"

namespace hal {

    class WaveData;

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
        QMap<QString,QString> mDictionary;
        QMap<QString,WaveData*> mWaves;
        QVector<int> mLastValue;
        int mErrorCount;
        bool mSaleae;

        bool parseDataline(const QByteArray& line);
        bool parseDataNonDecimal(const QByteArray& line, int base);
        void storeValue(int val, const QByteArray& abrev);
        bool parseCsvHeader(char* buf);
        bool parseCsvDataline(char* buf, int dataLineIndex, u64 timeScale);
        bool parseSalea(const QString& filenameStub, const QString& abbrev, u64 timeScale);
    public:
        VcdSerializer(QObject* parent = nullptr);
        ~VcdSerializer();
        bool serialize(const QString& filename, const QList<const WaveData*>& waves) const;
        bool deserializeVcd(const QString& filename, const QStringList& netNames = QStringList());
        bool deserializeCsv(const QString& filename, u64 timeScale = 1000000000);
        QList<WaveData*> waveList() const { return mWaves.values(); }
        WaveData* waveByName(const QString& name);
        WaveData* waveById(u32 id);
        u64 maxTime() const { return mTime; }
    };

}

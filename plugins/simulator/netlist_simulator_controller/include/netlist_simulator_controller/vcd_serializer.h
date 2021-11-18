#pragma once

#include <QObject>
#include <QList>
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

        bool parseDataline(const QByteArray& line);
        bool parseDataNonDecimal(const QByteArray& line, int base);
        void storeValue(int val, const QByteArray& abrev);
        bool parseCsvHeader(const QByteArray& line);
        bool parseCsvDataline(const QByteArray& line, int dataLineIndex, u64 timeScale);
    public:
        VcdSerializer(QObject* parent = nullptr);
        bool serialize(const QString& filename, const QList<const WaveData*>& waves) const;
        bool deserializeVcd(const QString& filename);
        bool deserializeCsv(const QString& filename, u64 timeScale = 1000000000);
        QList<WaveData*> waveList() const { return mWaves.values(); }
        WaveData* waveByName(const QString& name) const;
        WaveData* waveById(u32 id) const;
        u64 maxTime() const { return mTime; }
    };

}

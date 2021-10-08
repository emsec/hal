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
        char charCode() const;
    };

    class VcdSerializer : public QObject
    {
        Q_OBJECT
        int mTime;
        QMap<QString,WaveData*> mWaves;

        void parseDataline(const QByteArray& line);
        void storeValue(int val, const QByteArray& abrev);
    public:
        VcdSerializer(QObject* parent = nullptr);
        bool serialize(const QString& filename, const QList<const WaveData*>& waves) const;
        QList<WaveData*> deserialize(const QString& filename);
    };

}

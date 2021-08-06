#pragma once

#include <QMap>
#include <QString>
namespace hal {

    class WaveData : public QMap<int,int>
    {
        QString mName;
    public:
        WaveData(const QString& nam, const QMap<int,int>& other = QMap<int,int>() );
        QString name() const { return mName; }
        int tValue(float t) const;
        void setStartvalue(int val);
        bool insertToggleTime(int t);

        static WaveData* clockFactory(int start, int cycle, int duration);
    };
}

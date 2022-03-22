#pragma once
#include <QList>
#include <QMap>
#include <QMutex>
#include "waveform_viewer/wave_transform.h"

class QPainter;

namespace hal {
    class WaveData;
    class WaveItemHash;
    class WaveFormPrimitive;
    class WaveFormPrimitiveFilled;

    class WaveScrollbar;
    class WaveTransform;
    class WaveDataProvider;

    class TimeInterval
    {
    public:
        double mCenterTime;
        double mDuration;
        TimeInterval(quint64 t0 = 0, quint64 t1 = 0) : mCenterTime(0.5*(t0+t1)), mDuration(t1-t0) {;}
        bool operator<(const TimeInterval& other) const;
    };

    class WaveFormPainted
    {
        QList<WaveFormPrimitive*> mPrimitives;
        WaveZoomShift mValidity;
        TimeInterval mShortestToggle;
        double mCursorTime;
        int mCursorXpos;
        int mCursorValue;
        QMutex mMutex;

    public:
        WaveFormPainted();
        ~WaveFormPainted();
        void clearPrimitives();

        void paint(int y0, QPainter& painter);

        void generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool* loop);
        bool generateGroup(const WaveData* wd, const WaveItemHash *hash);

        int numberPrimitives() const { return mPrimitives.size(); }

        float x0() const;
        float x1() const;

        int valueXpos(int xpos);

        const WaveZoomShift& validity() const { return mValidity; }
        bool isEmpty() const { return mPrimitives.isEmpty(); }
        QMap<float,int> primitiveValues();
        TimeInterval shortestToggle() const { return mShortestToggle; }
        void setCursorValue(double tCursor, int xpos, int val);
        int cursorValueStored(double tCursor, int xpos) const;
        int cursorValuePainted(double tCursor, int xpos);
    };
}

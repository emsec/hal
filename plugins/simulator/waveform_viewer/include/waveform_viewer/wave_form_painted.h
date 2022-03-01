#pragma once
#include <QList>
#include <QMap>
#include <QMutex>

class QPainter;

namespace hal {
    class WaveData;
    class WaveItemHash;
    class WaveFormPrimitive;
    class WaveFormPrimitiveFilled;

    class WaveScrollbar;
    class WaveTransform;
    class WaveDataProvider;

    class WaveFormPaintValidity
    {
        double mScale;
        quint64 mTleft;
        int mWidth;
    public:
        WaveFormPaintValidity(const WaveTransform* trans = nullptr, const WaveScrollbar* sbar = nullptr);
        bool operator==(const WaveFormPaintValidity& other) const;
        bool operator!=(const WaveFormPaintValidity& other) const { return ! operator==(other); }
        bool isNull() const;
        int width() const { return mWidth; }
    };

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
        WaveFormPaintValidity mValidity;
        TimeInterval mShortestToggle;
        float mCursorTime;
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

        const WaveFormPaintValidity& validity() const { return mValidity; }
        bool isEmpty() const { return mPrimitives.isEmpty(); }
        QList<float> intervalLimits();
        TimeInterval shortestToggle() const { return mShortestToggle; }
        void setCursorValue(float tCursor, int xpos, int val);
        int cursorValueStored(float tCursor, int xpos) const;
        int cursorValuePainted(float tCursor, int xpos);
    };
}

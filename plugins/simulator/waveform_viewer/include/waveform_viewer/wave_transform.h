#pragma once

#include <QtGlobal>

namespace hal {
    class WaveScrollbar;

    class WaveTransform
    {
        quint64 mTmin;
        quint64 mTmax;
        double mMag;
    public:
        WaveTransform(quint64 t0=0, quint64 t1=1000, double m=1) : mTmin(t0), mTmax(t1), mMag(m) { if (mMag>10) mMag=10; }
        quint64 tMin() const { return mTmin; }
        quint64 tMax() const { return mTmax; }
        quint64 deltaT() const { return mTmax - mTmin; }
        double vPos(double t) const;
        double vMax() const { return vPos(mTmax); }
        double vMin() const { return vPos(mTmin); }
        double tPos(quint64 v) const;
        double scale() const { return mMag; }
        void setTmin(quint64 t0) { mTmin = t0; }
        void setTmax(quint64 t1) { mTmax = t1; }
        void setScale(double m11) { mMag = m11; }
    };

    class WaveZoomShift
    {
        double mScale;
        quint64 mTleft;
        int mWidth;
    public:
        WaveZoomShift(double sc, quint64 tl, int w) : mScale(sc), mTleft(tl), mWidth(w) {;}
        WaveZoomShift(const WaveTransform* trans = nullptr, const WaveScrollbar* sbar = nullptr);
        bool operator==(const WaveZoomShift& other) const;
        bool operator!=(const WaveZoomShift& other) const { return ! operator==(other); }
        bool isNull() const;
        bool sameHistory(const WaveZoomShift& other) const;
        int width() const { return mWidth; }
        double scale() const { return mScale; }
        quint64 leftTime() const { return mTleft; }
        QString dumpString() const;
    };
}

#pragma once

#include <QtGlobal>

namespace hal {
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
        double tPos(quint64 v) const;
        double scale() const { return mMag; }
        void setTmin(quint64 t0) { mTmin = t0; }
        void setTmax(quint64 t1) { mTmax = t1; }
        void setScale(double m11) { mMag = m11; }
    };
}

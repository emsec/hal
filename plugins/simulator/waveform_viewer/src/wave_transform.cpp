#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "math.h"

namespace hal {
    double WaveTransform::vPos(double t) const
    {
        if (t < mTmin) return 0;
        return mMag*(t-mTmin);
    }

    double WaveTransform::tPos(quint64 v) const
    {
        return v/mMag + mTmin;
    }

    //-------------------------------------------------------

    WaveZoomShift::WaveZoomShift(const WaveTransform* trans, const WaveScrollbar* sbar)
        : mScale(0), mTleft(0), mWidth(0)
    {
        if (!trans || !sbar) return;
        mScale = trans->scale();
        mTleft = sbar->tLeftI();
        mWidth = sbar->viewportWidth();
    }

    bool WaveZoomShift::sameHistory(const WaveZoomShift& other)const
    {
        if (isNull() || other.isNull())
            return false;

        return (mScale == other.mScale && mTleft == other.mTleft);
    }

    bool WaveZoomShift::operator==(const WaveZoomShift& other) const
    {
        if (isNull() || other.isNull())
            return false;

        return (mScale == other.mScale && mTleft == other.mTleft && mWidth == other.mWidth);
    }

    bool WaveZoomShift::isNull() const
    {
        return (mWidth == 0 || mScale <= 0);
    }

}

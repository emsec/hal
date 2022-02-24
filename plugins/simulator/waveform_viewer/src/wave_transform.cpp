#include "waveform_viewer/wave_transform.h"
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
}

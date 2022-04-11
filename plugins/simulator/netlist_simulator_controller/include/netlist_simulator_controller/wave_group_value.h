#pragma once
#include "hal_core/defines.h"

namespace hal {

    class WaveGroupValue
    {
        u32 mHasTransition;
        u32 mValue;
        u32 mTooManyTransitions;
        u32 mUndefined;
    public:
        WaveGroupValue() : mHasTransition(0), mValue(0), mTooManyTransitions(0), mUndefined(0) {;}
        WaveGroupValue(int nbits);
        void setValue(u32 mask, int val);
        WaveGroupValue mergePrevious(const WaveGroupValue& previous) const;
        int value() const;

        static const int sTooManyTransitions = -96;
    };
}

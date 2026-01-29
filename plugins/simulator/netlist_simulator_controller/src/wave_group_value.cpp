#include "netlist_simulator_controller/wave_group_value.h"

namespace hal {
    WaveGroupValue::WaveGroupValue(int nbits) : mHasTransition(0), mValue(0), mTooManyTransitions(0), mUndefined(0)
    {
        for (int i=0; i<nbits; i++)
        {
            u32 mask = 1<<(nbits-i-1);
            mHasTransition |= mask;
            mUndefined  |= mask;
        }
    }

    void WaveGroupValue::setValue(u32 mask, int val)
    {
        mHasTransition |= mask;
        if (val == sTooManyTransitions)
        {
            mTooManyTransitions    |=   mask;
            mUndefined &= (~mask);
        }
        else if (val < 0)
        {
            mTooManyTransitions    &= (~mask);
            mUndefined |=   mask;
        }
        else
        {
            mTooManyTransitions    &= (~mask);
            mUndefined &= (~mask);
            if (val)
                mValue |= mask;
            else
                mValue &= (~mask);
        }
    }

    WaveGroupValue WaveGroupValue::mergePrevious(const WaveGroupValue& previous) const
    {
        WaveGroupValue retval(previous);
        u32 mask = 1;
        while (mask)
        {
            if (mHasTransition & mask)
            {
                if (mUndefined & mask)
                    retval.setValue(mask,-1);
                else if (mTooManyTransitions & mask)
                    retval.setValue(mask,sTooManyTransitions);
                else
                    retval.setValue(mask, (mValue&mask) ? 1 : 0);
            }
            mask <<= 1;
        }
        return retval;
    }

    int WaveGroupValue::value() const
    {
        if (mUndefined > 0)
            return -1;
        if (mTooManyTransitions > 0)
            return sTooManyTransitions;
        return mValue;
    }


}

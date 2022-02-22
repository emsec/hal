#include "waveform_viewer/wave_form_painted.h"
#include "netlist_simulator_controller/wave_data.h"
#include "waveform_viewer/wave_item.h"
#include "waveform_viewer/wave_form_primitive.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_data_provider.h"

namespace hal {

    bool TimeInterval::operator< (const TimeInterval& other) const
    {
        if (other.mCenterTime == 0) return true; // other invalid, this is better (=smaller)
        if (mCenterTime == 0) return false;   // this invalid, other is better (=smaller)
        return mDuration < other.mDuration;   // both valid, smaller wins
    }

    WaveFormPaintValidity::WaveFormPaintValidity(const WaveTransform* trans, const WaveScrollbar* sbar)
        : mScale(0), mTleft(0), mWidth(0)
    {
        if (!trans || !sbar) return;
        mScale = trans->scale();
        mTleft = sbar->tLeftI();
        mWidth = sbar->viewportWidth();
    }

    bool WaveFormPaintValidity::operator==(const WaveFormPaintValidity& other) const
    {
        if (isNull() || other.isNull())
            return false;

        return (mScale == other.mScale && mTleft == other.mTleft && mWidth == other.mWidth);
    }

    bool WaveFormPaintValidity::isNull() const
    {
        return (mWidth == 0 || mScale <= 0);
    }

    WaveFormPainted::WaveFormPainted()  {;}

    WaveFormPainted::~WaveFormPainted()
    {
        clearPrimitives();
    }

    void  WaveFormPainted::clearPrimitives()
    {
        for (WaveFormPrimitive* wfp : mPrimitives)
            delete wfp;
        mPrimitives.clear();
        mValidity = WaveFormPaintValidity();
        mShortestToggle = TimeInterval();
    }

    void WaveFormPainted::paint(int y0, QPainter& painter)
    {
        for (WaveFormPrimitive* wfp : mPrimitives)
            wfp->paint(y0,painter);
    }

    int WaveFormPainted::valueXpos(int xpos) const
    {
        for (const WaveFormPrimitive* wfp : mPrimitives)
            if (wfp->isInRange(xpos))
                return wfp->value();
        return SaleaeDataTuple::sReadError;
    }

    QList<float> WaveFormPainted::intervalLimits() const
    {
        QList<float> retval;
        for (const WaveFormPrimitive* wfp : mPrimitives)
        {
            if (wfp->x0() == wfp->x1()) continue;
            retval.append(wfp->x0());
            retval.append(wfp->x1());
        }
        return retval;
    }

    void WaveFormPainted::generateGroup(const WaveData* wd, const WaveItemHash *hash)
    {
        clearPrimitives();
        QList<const WaveFormPainted*> wirePainted;
        QMap<float,int> transitionPos;
        const WaveDataGroup* grp = dynamic_cast<const WaveDataGroup*>(wd);
        if (!grp) return;
        for (int iwave : grp->childrenWaveIndex())
        {
            if (iwave<0) return;
            WaveItemIndex wii(iwave, WaveItemIndex::Wire, grp->id());
            const WaveItem* wi = hash->value(wii);
            if (!wi || wi->mPainted.isEmpty()) return;
            wirePainted.append(&(wi->mPainted));
            for (float x : wi->mPainted.intervalLimits())
                transitionPos[x]++;
        }

        float lastX = -1;
        int value = -1;
        for (auto it = transitionPos.constBegin(); it != transitionPos.constEnd(); ++it)
        {
            int nextX = it.key();
            if (lastX >= 0)
            {
                if (value == WaveFormPrimitiveFilled::sFilledPrimitive)
                    mPrimitives.append(new WaveFormPrimitiveFilled(lastX,nextX,0));
                else if (value < 0)
                    mPrimitives.append(new WaveFormPrimitiveUndefined(lastX,nextX));
                else
                    mPrimitives.append(new WaveFormPrimitiveValue(lastX,nextX,value));
            }
            value = 0;
            lastX = nextX;
            int mask = 1;
            for (const WaveFormPainted* wfp : wirePainted)
            {
                int childVal = wfp->valueXpos(nextX);
                if (childVal == WaveFormPrimitiveFilled::sFilledPrimitive)
                {
                    value = WaveFormPrimitiveFilled::sFilledPrimitive;
                    break;
                }
                else if (childVal < 0)
                {
                    value = -1;
                    break;
                }
                else if (childVal)
                    value |= mask;
                mask <<= 1;
            }
        }
    }

    void WaveFormPainted::generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool *loop)
    {
        mValidity = WaveFormPaintValidity(trans, sbar);
        *loop = true;
        WaveFormPrimitive* pendingTransition = nullptr;

        quint64 tleft = sbar->tLeftI();
        int width = sbar->viewportWidth();

        int valNext = wdp->startValue(tleft);
        float xNext = 0;
        quint64 tNext = trans->tMin();
        if (valNext == SaleaeDataTuple::sReadError) *loop = false;
        float xMax = width;

        // max time within visibible t window
        if (sbar->xPosF(trans->tMax()) < xMax)
            xMax = sbar->xPosF(trans->tMax());

        while (*loop && xNext <= xMax)
        {
            quint64 tLast = tNext;
            float xLast = xNext;
            int valLast = valNext;

            SaleaeDataTuple sdt = wdp->nextPoint();

            if (sdt.readError())
            {
                xNext = xMax;
                *loop = false;
            }
            else
            {
                tNext = sdt.mTime;
                xNext = sbar->xPosF(tNext);
                valNext = sdt.mValue;
                if (xNext > xMax)
                {
                    xNext = xMax;
                    *loop = false;
                }
            }

            if (tNext > tLast)
            {
                TimeInterval dt(tLast, tNext);
                if (dt < mShortestToggle) mShortestToggle = dt;
            }

            if (xNext - xLast > 2.)
            {
                if (pendingTransition)
                {
                    mPrimitives.append(pendingTransition);
                    pendingTransition = nullptr;
                }
                if (valLast < 0)
                    mPrimitives.append(new WaveFormPrimitiveUndefined(xLast,xNext));
                else if (wdp->isGroup())
                    mPrimitives.append(new WaveFormPrimitiveValue(xLast,xNext,valLast));
                else
                    mPrimitives.append(new WaveFormPrimitiveHline(xLast,xNext,valLast));
            }
            else
            {

                WaveFormPrimitiveFilled* filled
                        = new WaveFormPrimitiveFilled(pendingTransition?pendingTransition->x0():xLast, xNext, valLast);
                if (pendingTransition)
                {
                    const WaveFormPrimitiveFilled* lastFilled = dynamic_cast<const WaveFormPrimitiveFilled*>(pendingTransition);
                    if (lastFilled) filled->add(*lastFilled);
                    delete pendingTransition;
                }
                pendingTransition = filled;
            }
            if (loop && valLast >= 0 && valNext >= 0 && valLast != valNext && !pendingTransition && !wdp->isGroup())
                pendingTransition = new WaveFormPrimitiveTransition(xNext);
        }
        if (pendingTransition)
            mPrimitives.append(pendingTransition);
    }

    float WaveFormPainted::x0() const
    {
        if (mPrimitives.isEmpty()) return -1;
        auto it = mPrimitives.constBegin();
        float retval = (*it)->x0();
        while (++it != mPrimitives.constEnd())
            if ((*it)->x0() < retval)
                retval = (*it)->x0();
        return retval;
    }

    float WaveFormPainted::x1() const
    {
        if (mPrimitives.isEmpty()) return -1;
        auto it = mPrimitives.constBegin();
        float retval = (*it)->x1();
        while (++it != mPrimitives.constEnd())
            if ((*it)->x1() > retval)
                retval = (*it)->x1();
        return retval;
    }
}

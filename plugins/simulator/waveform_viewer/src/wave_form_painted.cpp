#include "waveform_viewer/wave_form_painted.h"
#include "netlist_simulator_controller/wave_data.h"
#include "waveform_viewer/wave_item.h"
#include "waveform_viewer/wave_form_primitive.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_data_provider.h"
#include <QDebug>

namespace hal {

    bool TimeInterval::operator< (const TimeInterval& other) const
    {
        if (other.mCenterTime == 0) return true; // other invalid, this is better (=smaller)
        if (mCenterTime == 0) return false;   // this invalid, other is better (=smaller)
        return mDuration < other.mDuration;   // both valid, smaller wins
    }

    WaveFormPainted::WaveFormPainted()
        : mCursorTime(-1), mCursorValue(SaleaeDataTuple::sReadError) {;}

    WaveFormPainted::~WaveFormPainted()
    {
        clearPrimitives();
    }

    void  WaveFormPainted::clearPrimitives()
    {
        QMutexLocker lock(&mMutex);
        for (WaveFormPrimitive* wfp : mPrimitives)
            delete wfp;
        mPrimitives.clear();
        mValidity = WaveZoomShift();
        mShortestToggle = TimeInterval();
    }

    void WaveFormPainted::paint(int y0, QPainter& painter)
    {
        QMutexLocker lock(&mMutex);
        for (WaveFormPrimitive* wfp : mPrimitives)
            wfp->paint(y0,painter);
    }

    int WaveFormPainted::valueXpos(int xpos)
    {
        QMutexLocker lock(&mMutex);
        for (const WaveFormPrimitive* wfp : mPrimitives)
            if (wfp->isInRange(xpos))
            {
                qDebug() << "valx" << xpos << wfp->x0() << wfp->x1() << wfp->value();
                return wfp->value();
            }
        return SaleaeDataTuple::sReadError;
    }

    QMap<float,int> WaveFormPainted::primitiveValues()
    {
        QMap<float,int> retval;
        QMutexLocker lock(&mMutex);
        for (const WaveFormPrimitive* wfp : mPrimitives)
        {
            if (wfp->x0() == wfp->x1()) continue;
            retval.insert(wfp->x0(),wfp->value());
        }
        return retval;
    }

    bool WaveFormPainted::generateGroup(const WaveData* wd, const WaveItemHash *hash)
    {
        clearPrimitives();

        class PrimitiveValue
        {
            u32 mTransition;
            u32 mValue;
            u32 mFilled;
            u32 mUndefined;
        public:
            PrimitiveValue() : mTransition(0), mValue(0), mFilled(0), mUndefined(0) {;}
            PrimitiveValue(int nbits) : mTransition(0), mValue(0), mFilled(0), mUndefined(0) {
                for (int i=0; i<nbits; i++)
                {
                    u32 mask = 1<<i;
                    mTransition |= mask;
                    mUndefined  |= mask;
                }
            }

            void setValue(u32 mask, int val)
            {
                mTransition |= mask;
                if (val == WaveFormPrimitiveFilled::sFilledPrimitive)
                {
                    mFilled    |=   mask;
                    mUndefined &= (~mask);
                }
                else if (val < 0)
                {
                    mFilled    &= (~mask);
                    mUndefined |=   mask;
                }
                else
                {
                    mFilled    &= (~mask);
                    mUndefined &= (~mask);
                    if (val)
                        mValue |= mask;
                    else
                        mValue &= (~mask);
                }
            }

            PrimitiveValue mergePrevious(const PrimitiveValue& previous) const
            {
                PrimitiveValue retval(previous);
                u32 mask = 1;
                while (mask)
                {
                    if (mTransition & mask)
                    {
                        if (mUndefined & mask)
                            retval.setValue(mask,-1);
                        else if (mFilled & mask)
                            retval.setValue(mask,WaveFormPrimitiveFilled::sFilledPrimitive);
                        else
                            retval.setValue(mask, (mValue&mask) ? 1 : 0);
                    }
                    mask <<= 1;
                }
                return retval;
            }

            int value() const
            {
                if (mUndefined > 0)
                    return -1;
                if (mFilled > 0)
                    return WaveFormPrimitiveFilled::sFilledPrimitive;
                return mValue;
            }
        };

        QMap<float,PrimitiveValue> transitionValue;
        const WaveDataGroup* grp = dynamic_cast<const WaveDataGroup*>(wd);
        if (!grp) return false;
        bool firstWave = true;

        int nbits = 0;
        for (int iwave : grp->childrenWaveIndex())
        {
            if (iwave<0) return false;
            WaveItemIndex wii(iwave, WaveItemIndex::Wire, grp->id());
            WaveItem* wi = hash->value(wii);
            if (!wi || wi->mPainted.isEmpty()) return false;

            // validity must be the same for all child elements
            if (firstWave)
            {
                mValidity = wi->mPainted.validity();
                firstWave = false;
            }
            else
            {
                if (mValidity != wi->mPainted.validity()) return false;
            }
            QMap<float,int> pValues = wi->mPainted.primitiveValues();
            for (auto it = pValues.constBegin(); it != pValues.constEnd(); ++it)
            {
                transitionValue[it.key()].setValue((1<<nbits),it.value());
            }
            ++nbits;
        }

        float lastX = -1;
        PrimitiveValue lastValue(nbits);

        bool refreshCursor = transitionValue.isEmpty() ? false : mCursorTime >= transitionValue.constBegin().key();
        for (auto it = transitionValue.constBegin(); it != transitionValue.constEnd(); ++it)
        {
            int nextX = it.key();
            PrimitiveValue nextValue = it.value().mergePrevious(lastValue);
            bool updateX = true;
            if (lastX >= 0) // not first loop -> valid lastX value
            {
                int val = lastValue.value();
                if (val == WaveFormPrimitiveFilled::sFilledPrimitive)
                    mPrimitives.append(new WaveFormPrimitiveFilled(lastX,nextX,0));
                else if (val < 0)
                    mPrimitives.append(new WaveFormPrimitiveUndefined(lastX,nextX));
                else if (val != nextValue.value())
                    mPrimitives.append(new WaveFormPrimitiveValue(lastX,nextX,val,grp->bits(),grp->valueBase()));
                else
                    updateX = false; // same value, will be painted at next transition

                if (refreshCursor && nextX > mCursorTime)
                {
                    if (val != WaveFormPrimitiveFilled::sFilledPrimitive)
                        mCursorValue = val;
                    refreshCursor = false;
                }

            }
            if (updateX) lastX = nextX;
            lastValue = nextValue;
        }
        return true;
    }

    void WaveFormPainted::generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool *loop)
    {
        mValidity = WaveZoomShift(trans, sbar);
        *loop = true;
        WaveFormPrimitive* pendingTransition = nullptr;

        quint64 tleft = sbar->tLeftI();
        bool refreshCursor = (mCursorTime >= tleft);
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

            if (refreshCursor && (tNext > mCursorTime || !loop))
            {
                mCursorValue = valLast;
                refreshCursor = false;
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
                    mPrimitives.append(new WaveFormPrimitiveValue(xLast,xNext,valLast,wdp->bits(),wdp->valueBase()));
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

    void WaveFormPainted::setCursorValue(double tCursor, int xpos, int val)
    {
        mCursorTime = tCursor;
        mCursorXpos = xpos;
        mCursorValue = val;
    }

    int WaveFormPainted::cursorValueStored(double tCursor, int xpos) const
    {
        // stored value not valid
        if (tCursor != mCursorTime || xpos != mCursorXpos)
            return SaleaeDataTuple::sReadError;

        // can deliver value - might be invalid though
        return mCursorValue;
    }

    int WaveFormPainted::cursorValuePainted(double tCursor, int xpos)
    {
        // try get from painted primitives
        mCursorValue = valueXpos(xpos);
        if (mCursorValue == WaveFormPrimitiveFilled::sFilledPrimitive)
            mCursorValue = SaleaeDataTuple::sReadError;
        mCursorTime = tCursor;
        mCursorXpos = xpos;
        return mCursorValue;
    }
}

#include "waveform_viewer/wave_form_painted.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/wave_group_value.h"
#include "netlist_simulator_controller/wave_data_provider.h"
#include "waveform_viewer/wave_item.h"
#include "waveform_viewer/wave_form_primitive.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include <math.h>
#include <QDebug>

namespace hal {

    bool TimeInterval::operator< (const TimeInterval& other) const
    {
        if (other.mCenterTime == 0) return true; // other invalid, this is better (=smaller)
        if (mCenterTime == 0) return false;   // this invalid, other is better (=smaller)
        return mDuration < other.mDuration;   // both valid, smaller wins
    }

    WaveFormPainted::WaveFormPainted()
        : mCursorTime(0), mCursorValue(SaleaeDataTuple::sReadError)
    {;}

    WaveFormPainted::WaveFormPainted(const WaveFormPainted& other)
    {
        clonePrimitives(other);
    }

    void WaveFormPainted::clonePrimitives(const WaveFormPainted &other)
    {
        for (const WaveFormPrimitive* wfp : other.mPrimitives)
        {
            if (const WaveFormPrimitiveHline* wfph = dynamic_cast<const WaveFormPrimitiveHline*>(wfp); wfph)
                mPrimitives.append(new WaveFormPrimitiveHline(*wfph));
            else if (const WaveFormPrimitiveTransition* wfpt = dynamic_cast<const WaveFormPrimitiveTransition*>(wfp); wfpt)
                mPrimitives.append(new WaveFormPrimitiveTransition(*wfpt));
            else if (const WaveFormPrimitiveTrigger* wfpg = dynamic_cast<const WaveFormPrimitiveTrigger*>(wfp); wfpg)
                mPrimitives.append(new WaveFormPrimitiveTrigger(*wfpg));
            else if (const WaveFormPrimitiveUndefined* wfpu = dynamic_cast<const WaveFormPrimitiveUndefined*>(wfp); wfpu)
                mPrimitives.append(new WaveFormPrimitiveUndefined(*wfpu));
            else if (const WaveFormPrimitiveFilled* wfpf = dynamic_cast<const WaveFormPrimitiveFilled*>(wfp); wfpf)
                mPrimitives.append(new WaveFormPrimitiveFilled(*wfpf));
            else if (const WaveFormPrimitiveValue* wfpv = dynamic_cast<const WaveFormPrimitiveValue*>(wfp); wfpv)
                mPrimitives.append(new WaveFormPrimitiveValue(*wfpv));
            else
                qDebug() << "Programming error, copy constructor failed due to unknown primitive type";
        }
    }

    WaveFormPainted::~WaveFormPainted()
    {
        clearPrimitives();
    }

    WaveFormPainted& WaveFormPainted::operator=(const WaveFormPainted& other)
    {
        clonePrimitives(other);
        mValidity       = other.mValidity;
        mShortestToggle = other.shortestToggle();
        mCursorTime     = other.mCursorTime;
        mCursorXpos     = other.mCursorXpos;
        mCursorValue    = other.mCursorValue;
        return *this;
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
            if (wfp->x1() <= wfp->x0()) continue;
            retval.insert(wfp->x0(),wfp->value());
        }
        return retval;
    }

    bool WaveFormPainted::generateGroup(const WaveData* wd, const WaveItemHash *hash)
    {
        clearPrimitives();

        QMap<float,WaveGroupValue> transitionValue;
        const WaveDataGroup* grp = dynamic_cast<const WaveDataGroup*>(wd);
        if (!grp) return false;
        bool firstWave = true;

        float xmax = 0;
        int nbits = 0;
        for (int iwave : grp->childrenWaveIndex())
        {
            if (iwave<0) return false;
            WaveItemIndex wii(iwave, WaveItemIndex::Wire, grp->id());
            WaveItem* wi = hash->value(wii);
            if (!wi || wi->mPainted.isEmpty()) return false;

            QMutexLocker lock(&wi->mMutex);
            // validity must be the same for all child elements
            if (firstWave)
            {
                mValidity = wi->mPainted.validity();
                xmax = wi->mPainted.x1();
                firstWave = false;
            }
            else
            {
                float testXmax = wi->mPainted.x1();
                if (testXmax > xmax) xmax = testXmax;
                if (mValidity != wi->mPainted.validity())
                    return false;
            }
            QMap<float,int> pValues = wi->mPainted.primitiveValues();
            u32 mask = 1<<nbits;
            for (auto it = pValues.constBegin(); it != pValues.constEnd(); ++it)
            {
                transitionValue[it.key()].setValue(mask,it.value());
            }
            ++nbits;
        }

        float lastX = -1;
        bool firstLoop = true;
        WaveGroupValue lastValue(nbits);

        transitionValue[xmax] = lastValue;

        bool refreshCursor = transitionValue.isEmpty() ? false : mCursorTime >= transitionValue.constBegin().key();
        for (auto it = transitionValue.constBegin(); it != transitionValue.constEnd(); ++it)
        {
            int nextX = it.key();
            WaveGroupValue nextValue = it.value().mergePrevious(lastValue);
            bool updateX = true;
            if (!firstLoop) // not first loop -> valid lastX value
            {
                int val = lastValue.value();
                if (val == WaveGroupValue::sTooManyTransitions)
                    mPrimitives.append(new WaveFormPrimitiveFilled(lastX,nextX,0));
                else if (val < 0)
                    mPrimitives.append(new WaveFormPrimitiveUndefined(lastX,nextX));
                else if (val != nextValue.value())
                    mPrimitives.append(new WaveFormPrimitiveValue(lastX,nextX,val,grp->bits(),grp->valueBase()));
                else
                    updateX = false; // same value, will be painted at next transition

                if (refreshCursor && nextX > 0 && (u64) nextX > mCursorTime)
                {
                    if (val != WaveGroupValue::sTooManyTransitions)
                        mCursorValue = val;
                    refreshCursor = false;
                }

            }
            if (updateX) lastX = nextX;
            lastValue = nextValue;
            firstLoop = false;
        }
        return true;
    }

    bool WaveFormPainted::generateBoolean(const WaveData* wd, const WaveDataList *wdList, const WaveItemHash *hash)
    {
        clearPrimitives();

        QMap<float,WaveGroupValue> transitionValue;
        const WaveDataBoolean* wdBool = dynamic_cast<const WaveDataBoolean*>(wd);
        if (!wdBool) return false;
        bool firstWave = true;

        float xmax = 0;
        int nbits = 0;
        for (WaveData* wdChild : wdBool->children())
        {
            int iwave = wdChild->dataIndex();
            if (iwave<0) return false;
            WaveItem* wi = nullptr;
            for (u32 grpId = 0; grpId <= wdList->maxGroupId(); ++grpId)
            {
                WaveItemIndex wii(iwave, WaveItemIndex::Wire, grpId);
                WaveItem* wi = hash->value(wii);
                if (wi) break;
            }
            if (!wi || wi->mPainted.isEmpty()) return false;

            QMutexLocker lock(&wi->mMutex);
            // validity must be the same for all child elements
            if (firstWave)
            {
                mValidity = wi->mPainted.validity();
                xmax = wi->mPainted.x1();
                firstWave = false;
            }
            else
            {
                float testXmax = wi->mPainted.x1();
                if (testXmax > xmax) xmax = testXmax;
                if (mValidity != wi->mPainted.validity())
                    return false;
            }
            QMap<float,int> pValues = wi->mPainted.primitiveValues();
            u32 mask = 1<<nbits;
            for (auto it = pValues.constBegin(); it != pValues.constEnd(); ++it)
            {
                transitionValue[it.key()].setValue(mask,it.value());
            }
            ++nbits;
        }

        float lastX = -1;
        bool firstLoop = true;
        WaveGroupValue lastValue(nbits);

        transitionValue[xmax] = lastValue;

        bool refreshCursor = transitionValue.isEmpty() ? false : mCursorTime >= transitionValue.constBegin().key();
        for (auto it = transitionValue.constBegin(); it != transitionValue.constEnd(); ++it)
        {
            int nextX = it.key();
            WaveGroupValue nextValue = it.value().mergePrevious(lastValue);
            bool updateX = true;
            if (!firstLoop) // not first loop -> valid lastX value
            {
                int val = lastValue.value();
                if (val == WaveGroupValue::sTooManyTransitions)
                    mPrimitives.append(new WaveFormPrimitiveFilled(lastX,nextX,0));
                else if (val < 0)
                    mPrimitives.append(new WaveFormPrimitiveUndefined(lastX,nextX));
                else if (val != nextValue.value())
                {
                    const char* tt = wdBool->truthTable();
                    int j = val/8;
                    int k = val%8;
                    int val = (tt[j] & (1<<k)) ? 1 : 0;
                    mPrimitives.append(new WaveFormPrimitiveHline(lastX,nextX,val));
                }
                else
                    updateX = false; // same value, will be painted at next transition

                if (refreshCursor && nextX > 0 && (u64) nextX > mCursorTime)
                {
                    if (val != WaveGroupValue::sTooManyTransitions)
                        mCursorValue = val;
                    refreshCursor = false;
                }

            }
            if (updateX) lastX = nextX;
            lastValue = nextValue;
            firstLoop = false;
        }
        return true;
    }

    void WaveFormPainted::generateTrigger(WaveDataProvider *wdp, const WaveTransform *trans, const WaveScrollbar *sbar, bool *loop)
    {
        mValidity = WaveZoomShift(trans, sbar);
        *loop = true;

        quint64 tleft = sbar->tLeftI();
        bool refreshCursor = (mCursorTime >= tleft);
        int width = sbar->viewportWidth();

        SaleaeDataTuple tuple = wdp->startValue(tleft);
        float xpos = sbar->xPosF(tuple.mTime);

        while(*loop && tuple.mValue != SaleaeDataTuple::sReadError && xpos <= width && tuple.mTime <= trans->tMax())
        {
            if (xpos >= 0 && tuple.mValue==1) mPrimitives.append(new WaveFormPrimitiveTrigger(xpos));
            if (refreshCursor && tuple.mTime >= mCursorTime)
            {
                if (tuple.mTime == mCursorTime)
                    mCursorValue = 1;
                else
                    mCursorValue = 0;
                refreshCursor = false;
            }
            tuple = wdp->nextPoint();
            xpos = sbar->xPosF(tuple.mTime);
        }
    }

    void WaveFormPainted::generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool *loop)
    {
        if (wdp->isTrigger())
        {
            generateTrigger(wdp,trans,sbar,loop);
            return;
        }
        mValidity = WaveZoomShift(trans, sbar);
        *loop = true;
        WaveFormPrimitive* pendingTransition = nullptr;

        quint64 tleft = sbar->tLeftI();
        bool refreshCursor = (mCursorTime >= tleft);
        int width = sbar->viewportWidth();

        SaleaeDataTuple startval = wdp->startValue(tleft);
        int valNext = startval.mValue;
        float xNext = sbar->xPosF(startval.mTime);
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
            {
                pendingTransition = new WaveFormPrimitiveTransition(xNext);
            }
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

    void WaveFormPainted::setCursorValue(u64 tCursor, int xpos, int val)
    {
        mCursorTime = tCursor;
        mCursorXpos = xpos;
        mCursorValue = val;
    }

    int WaveFormPainted::cursorValueStored(u64 tCursor, int xpos) const
    {
        // stored value not valid
        if (tCursor != mCursorTime || xpos != mCursorXpos)
            return SaleaeDataTuple::sReadError;

        // can deliver value - might be invalid though
        return mCursorValue;
    }

    int WaveFormPainted::cursorValueTrigger(u64 tCursor, int xpos)
    {
        QMutexLocker lock(&mMutex);
        mCursorValue = 0;
        for (const WaveFormPrimitive* wfp : mPrimitives)
        {
            if (fabs(wfp->x0()-xpos)<0.5)
                mCursorValue = 1;
        }
        mCursorTime = tCursor;
        mCursorXpos = xpos;
        return mCursorValue;
    }

    int WaveFormPainted::cursorValuePainted(u64 tCursor, int xpos)
    {
        // try get from painted primitives
        int tmpValue = valueXpos(xpos);
        if (tmpValue == WaveGroupValue::sTooManyTransitions)
            tmpValue = SaleaeDataTuple::sReadError;

        if (tmpValue >= BooleanFunction::Z)
        {
            mCursorTime = tCursor;
            mCursorXpos = xpos;
            mCursorValue = tmpValue;
        }
        return tmpValue;
    }
}

#include "waveform_viewer/wave_form_painted.h"
#include "waveform_viewer/wave_form_primitive.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_data_provider.h"

namespace hal {


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

    void WaveFormPainted::generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool *loop)
    {
        mValidity = WaveFormPaintValidity(trans, sbar);
        *loop = true;
        WaveFormPrimitive* pendingTransition = nullptr;

        quint64 tleft = sbar->tLeftI();
        int width = sbar->viewportWidth();

        int valNext = wdp->startValue(tleft);
        float xNext = 0;
        if (valNext == SaleaeDataTuple::sReadError) *loop = false;
        float xMax = width;

        // max time within visibible t window
        if (sbar->xPosF(trans->tMax()) < xMax)
            xMax = sbar->xPosF(trans->tMax());

        while (*loop && xNext <= xMax)
        {
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
                xNext = sbar->xPosF(sdt.mTime);
                valNext = sdt.mValue;
                if (xNext > xMax)
                {
                    xNext = xMax;
                    *loop = false;
                }
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
                int xOld = xLast;
                if (pendingTransition)
                {
                    xOld = pendingTransition->x0();
                    delete pendingTransition;
                }
                pendingTransition = new WaveFormPrimitiveFilled(xOld,xNext);
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

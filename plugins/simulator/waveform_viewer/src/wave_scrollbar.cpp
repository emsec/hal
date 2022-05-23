#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_transform.h"
#include <math.h>
#include <QDebug>

namespace hal {
    WaveScrollbar::WaveScrollbar(const WaveTransform* trans, QWidget* parent)
      : QScrollBar(parent), mTransform(trans), mVleft(0), mVmaxScroll(0), mVieportWidth(0), mHandleSliderChange(true)
    {
        setSingleStep(1);
    }

    int WaveScrollbar::toUInt(double v)
    {
        if (v < 0) return 0;
        return (int) floor(v+0.5);
    }

    void WaveScrollbar::adjust(quint64 visibleWidth)
    {
        setMinimum(0);
        mVieportWidth = visibleWidth;
        quint64 vmax = mTransform->vMax();
        if (vmax <= visibleWidth)
        {
            mVmaxScroll = 0;
            setMaximum(0);
            return;
        }
        mVmaxScroll = vmax - visibleWidth;
        if (mVmaxScroll < 4096)
            setMaximum(toUInt(mVmaxScroll));
        else
            setMaximum(4096);
    }

    void WaveScrollbar::setVleftIntern(double v)
    {
        if (v > mVmaxScroll)
            v = mVmaxScroll;
        mVleft = v;
    }

    void WaveScrollbar::updateScale(double deltaScale, double tEvent, quint64 visibleWidth)
    {
        double oldVleft = mVleft;
        adjust(visibleWidth);
        if (!mVmaxScroll) return;
        double prod = deltaScale * (tEvent - mTransform->tMin());
        double newVleft = prod + oldVleft;
        setVleft(newVleft);
    }

    void WaveScrollbar::setVleft(double v)
    {
        if (v < 0 || !maximum())
        {
            setValue(0);
            setVleftIntern(0);
        }
        else
        {
            setVleftIntern(v);
            mHandleSliderChange = false;
            if (maximum() < 4096)
                setValue(toUInt(mVleft));
            else
                setValue(toUInt(mVleft * 4096. / mVmaxScroll));
            mHandleSliderChange = true;
        }
    }

    double WaveScrollbar::vLeft() const
    {
        return mVleft;
    }

    void WaveScrollbar::sliderChange(SliderChange change)
    {
        if (change == SliderChange::SliderValueChange &&
                (!mHandleSliderChange || value() == mLastValue)) return;
        if (!maximum())
            setVleftIntern(0);
        else if (maximum() < 4096)
            setVleftIntern(value());
        else
        {
            int dx = mLastValue - value();
            if (abs(dx)==1)
            {
                double v = mVleft - dx * mVieportWidth / 2;
                if (v<0) v=0;
                setVleftIntern(v);
                setValue(toUInt(mVleft * 4096. / mVmaxScroll));
            }
            else
                setVleftIntern(toUInt(value() * mVmaxScroll / 4096.));
        }
        mLastValue = value();
    }

    double WaveScrollbar::xPosF(double t) const
    {
        return mTransform->vPos(t) - mVleft;
    }

    int WaveScrollbar::xPosI(double t) const
    {
       double v = mTransform->vPos(t);
       if (v < mVleft) return -1;
       return toUInt(v - mVleft);
    }

    double WaveScrollbar::tPosF(int x) const
    {
        return mTransform->tPos(vLeft() + x);
    }

    quint64 WaveScrollbar::tPosI(int x) const
    {
        return floor(tPosF(x)+0.5);
    }

    double WaveScrollbar::tLeftF() const
    {
        return mTransform->tPos(vLeft());
    }

    quint64 WaveScrollbar::tLeftI() const
    {
        return floor(tLeftF()+0.5);
    }
}

#pragma once

#include <QScrollBar>

namespace hal {
    class WaveTransform;

    class WaveScrollbar : public QScrollBar
    {
        const WaveTransform* mTransform;
        double mVleft;
        double mVmaxScroll;
        int mVieportWidth;
        int mLastValue;
        bool mHandleSliderChange;
        using QScrollBar::value;
        using QScrollBar::setValue;
        using QScrollBar::setMaximum;
        void setVleftIntern(double v);
        static int toUInt(double v);
    protected:
        void sliderChange(SliderChange change) override;
    public:
        WaveScrollbar(const WaveTransform* trans, QWidget* parent = nullptr);
        void adjust(quint64 visibleWidth);
        double vLeft() const;
        void setVleft(double v);
        double tLeftF() const;
        quint64 tLeftI() const;
        void updateScale(double deltaScale, double tEvent, quint64 visibleWidth);
        int xPosI(double t) const;
        double xPosF(double t) const;
        double tPos(int x) const;
        int viewportWidth() const { return mVieportWidth; }
    };
}

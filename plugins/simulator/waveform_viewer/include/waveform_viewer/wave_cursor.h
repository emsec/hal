#pragma once

#include <QWidget>

namespace hal {

    class WaveTransform;
    class WaveScrollbar;

    class WaveCursor : public QWidget
    {
        const WaveTransform* mTransform;
        const WaveScrollbar* mScrollbar;
        QPoint mPosition;
        double mTimeValue;

        static const int sWidth = 91;
        static const int sLabHeight = 31;

    protected:
        void paintEvent(QPaintEvent *event) override;
    public:
        WaveCursor(const WaveTransform* trans, const WaveScrollbar* scroll, QWidget* parent = nullptr);
        void setCursorPosition(const QPoint& pos);
        void setCursorToTime(double t);
        void setViewportHeight(int height);
        void recalcTime();
        QRect labelRect() const;
    };
}

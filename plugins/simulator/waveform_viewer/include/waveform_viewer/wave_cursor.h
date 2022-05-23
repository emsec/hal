#pragma once
#include "hal_core/defines.h"
#include <QWidget>

namespace hal {

    class WaveTransform;
    class WaveScrollbar;

    class WaveCursor : public QWidget
    {
        const WaveTransform* mTransform;
        const WaveScrollbar* mScrollbar;
        QPoint mPosition;
        u64 mTimeValue;

        static const int sLabHeight = 31;

    protected:
        void paintEvent(QPaintEvent *event) override;
    public:
        WaveCursor(const WaveTransform* trans, const WaveScrollbar* scroll, QWidget* parent = nullptr);
        void setCursorPosition(const QPoint& pos);
        void setCursorToTime(u64 t);
        void setViewportHeight(int height);
        void recalcTime();
        QRect labelRect() const;
        static const int sWidth = 91;
    };
}

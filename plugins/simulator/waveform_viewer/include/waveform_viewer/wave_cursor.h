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

        static const int sWidth = 91;
        static const int sLabHeight = 31;

    protected:
        void paintEvent(QPaintEvent *event) override;
    public:
        WaveCursor(const WaveTransform* trans, const WaveScrollbar* scroll, QWidget* parent = nullptr);
        void setCursorPosition(const QPoint& pos);
        void setViewportHeight(int height);
        QRect labelRect() const;
    };
}

#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>

namespace hal {

    class WaveLabel;
    class WaveData;

    class WaveView : public QGraphicsView
    {
        Q_OBJECT

        float mXmag, mYmag, mDx, mDy, mXmagMin;

        float mLastCursorPos;
        int mLastWidth;
        int mCursorPixelPos;

        void restoreCursor();
    Q_SIGNALS:
        void changedXscale(float m11);
        void relativeYScroll(int dy);

    public Q_SLOTS:
        void handleCursorMoved(float xpos);

    protected:
        void wheelEvent(QWheelEvent *event) override;
        void scrollContentsBy(int dx, int dy) override;
        void resizeEvent(QResizeEvent *event) override;

    public:
        WaveView(QWidget* parent=nullptr);
    };

}

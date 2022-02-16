#pragma once
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <math.h>
#include "wave_transform.h"

namespace hal {

    class WaveRenderEngine;
    class WaveScrollbar;
    class WaveTimescale;
    class WaveCursor;
    class WaveDataList;
    class WaveItemHash;

    class WaveGraphicsCanvas : public QAbstractScrollArea
    {
        Q_OBJECT
        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;
        WaveTransform mTranform;
        WaveRenderEngine* mRenderEngine;
        WaveScrollbar* mScrollbar;
        WaveTimescale* mTimescale;
        WaveCursor* mCursor;
        bool mMoveCursor;
        QWidget* mDragZoom;
    public Q_SLOTS:
        void updateRequest();
    protected:
        void resizeEvent(QResizeEvent *) override;
        void wheelEvent(QWheelEvent *) override;
        void mousePressEvent(QMouseEvent *) override;
        void mouseMoveEvent(QMouseEvent *) override;
        void mouseReleaseEvent(QMouseEvent *) override;
        void mouseDoubleClickEvent(QMouseEvent *) override;
        void scrollContentsBy(int dx, int dy) override;
    public:
        WaveGraphicsCanvas(WaveDataList* wdlist, WaveItemHash* wHash, QWidget* parent = nullptr);
        const WaveTransform* transform() const { return &mTranform; }
    };
}

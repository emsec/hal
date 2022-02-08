#pragma once

#include <QGraphicsView>
#include <QSize>

namespace hal {
    class WaveTimescale;
    class WaveDataTimeframe;

    class WaveGraphicsView : public QGraphicsView
    {
        Q_OBJECT

        float mXmag, mXmagMin, mSceneLeft;
        WaveTimescale* mTimescale;
        int mMinViewportHeight;

        static const float sYmag;

        void adjustTimescale();
    protected:
        void scrollContentsBy(int dx, int dy) override;
        void wheelEvent(QWheelEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void mouseDoubleClickEvent(QMouseEvent *event) override;

    Q_SIGNALS:
        void changedXscale(float m11);

    public Q_SLOTS:
        void handleViewportHeightChanged(int height);
        void handleSizeChanged(int treeViewportHeight, int scrollbarMax, int scrollbarPos);
        void handleNumberVisibileChanged(int nVisible, int scrollbarMax, int scrollbarPos);
        void handelTimeframeChanged(const WaveDataTimeframe* tframe);

    public:
        WaveGraphicsView(QWidget* parent=nullptr);
        void setDefaultTransform();
        void setCursorPos(const QPoint& pos);
    };
}

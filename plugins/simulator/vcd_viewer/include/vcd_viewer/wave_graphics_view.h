#pragma once

#include <QGraphicsView>
#include <QSize>

namespace hal {
    class WaveTimescale;

    class WaveGraphicsView : public QGraphicsView
    {
        Q_OBJECT

        float mXmag, mXmagMin;
        WaveTimescale* mTimescale;

        static const float sYmag;
    protected:
        void scrollContentsBy(int dx, int dy) override;
        void wheelEvent(QWheelEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void mouseDoubleClickEvent(QMouseEvent *event) override;

    Q_SIGNALS:
        void changedXscale(float m11);

    public Q_SLOTS:
        void handleViewportHeightChanged(int height);
        void handleSizeChanged(int widgetHeight, int viewportHeight);

    public:
        WaveGraphicsView(QWidget* parent=nullptr);
        void setDefaultTransform();
        void setCursorPos(const QPoint& pos);
    };
}

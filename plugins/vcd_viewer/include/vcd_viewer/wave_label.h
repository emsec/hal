#pragma once

#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QPoint>
#include <QBrush>
namespace hal {
    class WaveLabel : public QWidget
    {
        Q_OBJECT

        int mDataIndex;
        QString mName;
        int mValue;
        QPoint mMousePoint;
        QPoint mMouseRelative;
        QRectF mDeleteRect;
        int mState;
        bool mHighlight;

        WaveLabel* mGhostShape;
        static QPixmap* sXdelete;

        QBrush valueBackground() const;
        QString valueString() const;
    protected:
        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

    Q_SIGNALS:
        void doubleClicked(int inx);
        void triggerDelete(int inx);
        void triggerSwap(int inx, int iypos);
        void triggerMove(int inx, int iypos);

    public:
        explicit WaveLabel(int inx, const QString& nam, QWidget *parent = nullptr);
        void setDataIndex(int inx) { mDataIndex = inx; }
        void setValue(int val);
        void setHighlight(bool hl);
        void paintEvent(QPaintEvent *event) override;
        int state() const { return mState; }
        void setState(int v) { mState = v; }
        static QPixmap* piXdelete();
        QString text() const { return mName; }
    };
}

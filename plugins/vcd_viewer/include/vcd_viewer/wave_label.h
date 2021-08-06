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
        QRectF mDeleteRect;

        static QPixmap* sXdelete;

        QBrush valueBackground() const;
        QString valueString() const;
    protected:
        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    Q_SIGNALS:
        void doubleClicked(int inx);
        void triggerDelete(int inx);

    public:
        explicit WaveLabel(int inx, const QString& nam, QWidget *parent = nullptr);
        void setDataIndex(int inx) { mDataIndex = inx; }
        void setValue(int val);
        void paintEvent(QPaintEvent *event) override;
        static QPixmap* piXdelete();
    };
}

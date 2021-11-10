#pragma once
#include <QWidget>

namespace hal {

    class WaveTimescale : public QWidget
    {
        Q_OBJECT

        double mXmag;
        int mWidth;
        int mXscrollValue;

    protected:
        void paintEvent(QPaintEvent *event) override;

    public:
        WaveTimescale(QWidget *parent = nullptr);
        void setScale(float m11, float scWidth, int xScrollValue);
    };
}

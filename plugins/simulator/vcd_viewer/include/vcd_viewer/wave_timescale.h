#pragma once
#include <QWidget>

namespace hal {

    class WaveTimescale : public QWidget
    {
        Q_OBJECT

        double mXmag;
        double mSceneLeft;
        double mSceneRight;
        int mWidth;

    protected:
        void paintEvent(QPaintEvent *event) override;

    public:
        WaveTimescale(QWidget *parent = nullptr);
        void setScale(float m11, int width, float scLeft, float scRight);
    };
}

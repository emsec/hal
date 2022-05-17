#pragma once
#include <QWidget>

namespace hal {

    class WaveTransform;
    class WaveScrollbar;

    class WaveLogicTimescale
    {
        quint64 mTminor;
        quint64 mTmajor;
        quint64 mTlabel;
    public:
        WaveLogicTimescale(double scale);
        quint64 minorInterval() const { return mTminor; }
        quint64 majorInterval() const { return mTmajor; }
        quint64 labelInterval() const { return mTlabel; }
    };

    class WaveTimescale : public QWidget
    {
        Q_OBJECT

        const WaveTransform* mTransform;
        const WaveScrollbar* mScrollbar;

    protected:
        void paintEvent(QPaintEvent *event) override;

    public:
        WaveTimescale(const WaveTransform* trans, const WaveScrollbar* scroll, QWidget *parent = nullptr);
        void setScale(int viewportWidth);
    };
}

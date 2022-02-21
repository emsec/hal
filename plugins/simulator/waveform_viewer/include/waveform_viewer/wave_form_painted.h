#pragma once
#include <QList>
#include <QMap>

class QPainter;

namespace hal {
    class WaveFormPrimitive;

    class WaveScrollbar;
    class WaveTransform;
    class WaveDataProvider;

    class WaveFormPaintValidity
    {
        double mScale;
        quint64 mTleft;
        int mWidth;
    public:
        WaveFormPaintValidity(const WaveTransform* trans = nullptr, const WaveScrollbar* sbar = nullptr);
        bool operator==(const WaveFormPaintValidity& other) const;
        bool operator!=(const WaveFormPaintValidity& other) const { return ! operator==(other); }
        bool isNull() const;
        int width() const { return mWidth; }
    };

    class WaveFormPainted
    {
        QList<WaveFormPrimitive*> mPrimitives;
        WaveFormPaintValidity mValidity;
    public:
        WaveFormPainted();
        ~WaveFormPainted();
        void clearPrimitives();

        void paint(int y0, QPainter& painter);

        void generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool* loop);

        int numberPrimitives() const { return mPrimitives.size(); }

        float x0() const;
        float x1() const;

        int valueXpos(int xpos) const;

        const WaveFormPaintValidity& validity() const { return mValidity; }
    };
}

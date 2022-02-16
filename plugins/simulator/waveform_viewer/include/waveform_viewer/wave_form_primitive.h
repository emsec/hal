#pragma once
#include <QPolygonF>

class QPainter;

namespace hal {
    class WaveFormPrimitive
    {
    protected:
        float mX0;
        float mX1;
    public:
        WaveFormPrimitive(float x0, float x1) : mX0(x0), mX1(x1) {;}
        virtual ~WaveFormPrimitive() {;}
        virtual void paint(int y0, QPainter& painter) = 0;
        float x0() const { return mX0; }
        float x1() const { return mX1; }
    };

    class WaveFormPrimitiveHline : public WaveFormPrimitive
    {
        int mValue;
    public:
        WaveFormPrimitiveHline(float x0, float x1, int val) : WaveFormPrimitive(x0,x1), mValue(val) {;}
        void paint(int y0, QPainter& painter);
    };

    class WaveFormPrimitiveTransition : public WaveFormPrimitive
    {
    public:
        WaveFormPrimitiveTransition(float x) : WaveFormPrimitive(x,x) {;}
        void paint(int y0, QPainter& painter);
    };

    class WaveFormPrimitiveUndefined : public WaveFormPrimitive
    {
    public:
        WaveFormPrimitiveUndefined(float x0, float x1) : WaveFormPrimitive(x0,x1) {;}
        void paint(int y0, QPainter& painter);
    };

    class WaveFormPrimitiveFilled : public WaveFormPrimitive
    {
    public:
        WaveFormPrimitiveFilled(float x0, float x1) : WaveFormPrimitive(x0,x1) {;}
        void paint(int y0, QPainter& painter);
    };

    class WaveFormPrimitiveValue : public WaveFormPrimitive
    {
        int mValue;
        QPolygonF mPolygon;
    public:
        WaveFormPrimitiveValue(float x0, float x1, int val);
        void paint(int y0, QPainter& painter);
    };
}

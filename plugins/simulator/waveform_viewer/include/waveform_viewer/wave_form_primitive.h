#pragma once
#include <QPolygonF>
#include "netlist_simulator_controller/saleae_file.h"

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
        bool isInRange(int x) const { return mX0 <= x && x < mX1; }
        virtual int value() const { return SaleaeDataTuple::sReadError; }
    };

    class WaveFormPrimitiveHline : public WaveFormPrimitive
    {
        int mValue;
    public:
        WaveFormPrimitiveHline(float x0, float x1, int val) : WaveFormPrimitive(x0,x1), mValue(val) {;}
        void paint(int y0, QPainter& painter);
        int value() const { return mValue; }
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
        int value() const { return -1; }
    };

    class WaveFormPrimitiveFilled : public WaveFormPrimitive
    {
        double mAccumTime[2];
    public:
        static const int sFilledPrimitive = -96;
        WaveFormPrimitiveFilled(float x0, float x1, int val);
        void paint(int y0, QPainter& painter);
        void add(const WaveFormPrimitiveFilled& other);
        int value() const { return sFilledPrimitive; }
    };

    class WaveFormPrimitiveValue : public WaveFormPrimitive
    {
        int mValue;
        int mBits;
        int mBase;
        QPolygonF mPolygon;
    public:
        WaveFormPrimitiveValue(float x0, float x1, int val, int bits, int base);
        void paint(int y0, QPainter& painter);
        int value() const { return mValue; }
    };
}

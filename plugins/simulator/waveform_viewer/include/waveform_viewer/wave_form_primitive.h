//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once
#include <QPolygonF>
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/wave_group_value.h"

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

    class WaveFormPrimitiveTrigger : public WaveFormPrimitive
    {
    public:
        WaveFormPrimitiveTrigger(float x) : WaveFormPrimitive(x,x) {;}
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
        WaveFormPrimitiveFilled(float x0, float x1, int val);
        void paint(int y0, QPainter& painter);
        void add(const WaveFormPrimitiveFilled& other);
        int value() const { return WaveGroupValue::sTooManyTransitions; }
    };

    class WaveFormPrimitiveValue : public WaveFormPrimitive
    {
    private:
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

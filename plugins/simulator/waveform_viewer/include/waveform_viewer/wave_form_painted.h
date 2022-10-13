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
#include <QList>
#include <QMap>
#include <QMutex>
#include "waveform_viewer/wave_transform.h"
#include "hal_core/defines.h"

class QPainter;

namespace hal {
    class WaveData;
    class WaveItemHash;
    class WaveDataList;
    class WaveFormPrimitive;
    class WaveFormPrimitiveFilled;

    class WaveScrollbar;
    class WaveTransform;
    class WaveDataProvider;

    class TimeInterval
    {
    public:
        double mCenterTime;
        double mDuration;
        TimeInterval(quint64 t0 = 0, quint64 t1 = 0) : mCenterTime(0.5*(t0+t1)), mDuration(t1-t0) {;}
        bool operator<(const TimeInterval& other) const;
    };

    class WaveFormPainted
    {
        QList<WaveFormPrimitive*> mPrimitives;
        WaveZoomShift mValidity;
        TimeInterval mShortestToggle;
        u64 mCursorTime;
        int mCursorXpos;
        int mCursorValue;
        QMutex mMutex;

        void clonePrimitives(const WaveFormPainted& other);
    public:
        WaveFormPainted();
        WaveFormPainted(const WaveFormPainted& other);
        ~WaveFormPainted();
        void clearPrimitives();

        void paint(int y0, QPainter& painter);

        void generate(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool* loop);
        void generateTrigger(WaveDataProvider* wdp, const WaveTransform* trans, const WaveScrollbar* sbar, bool* loop);
        bool generateGroup(const WaveData* wd, const WaveItemHash *hash);
        bool generateBoolean(const WaveData* wd, const WaveDataList* wdList, const WaveItemHash *hash);

        int numberPrimitives() const { return mPrimitives.size(); }

        float x0() const;
        float x1() const;

        int valueXpos(int xpos);

        const WaveZoomShift& validity() const { return mValidity; }
        bool isEmpty() const { return mPrimitives.isEmpty(); }
        QMap<float,int> primitiveValues();
        TimeInterval shortestToggle() const { return mShortestToggle; }
        void setCursorValue(u64 tCursor, int xpos, int val);
        int cursorValueStored(u64 tCursor, int xpos) const;
        int cursorValuePainted(u64 tCursor, int xpos);
        int cursorValueTrigger(u64 tCursor, int xpos);
        WaveFormPainted& operator=(const WaveFormPainted& other);
    };
}

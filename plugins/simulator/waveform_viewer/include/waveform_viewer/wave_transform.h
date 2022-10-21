// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <QtGlobal>

namespace hal {
    class WaveScrollbar;

    class WaveTransform
    {
        quint64 mTmin;
        quint64 mTmax;
        double mMag;
    public:
        WaveTransform(quint64 t0=0, quint64 t1=1000, double m=1) : mTmin(t0), mTmax(t1), mMag(m) { if (mMag>10) mMag=10; }
        quint64 tMin() const { return mTmin; }
        quint64 tMax() const { return mTmax; }
        quint64 deltaT() const { return mTmax - mTmin; }
        double vPos(double t) const;
        double vMax() const { return vPos(mTmax); }
        double vMin() const { return vPos(mTmin); }
        double tPos(quint64 v) const;
        double scale() const { return mMag; }
        void setTmin(quint64 t0) { mTmin = t0; }
        void setTmax(quint64 t1) { mTmax = t1; }
        void setScale(double m11) { mMag = m11; }
    };

    class WaveZoomShift
    {
        double mScale;
        quint64 mTleft;
        int mWidth;
    public:
        WaveZoomShift(double sc, quint64 tl, int w) : mScale(sc), mTleft(tl), mWidth(w) {;}
        WaveZoomShift(const WaveTransform* trans = nullptr, const WaveScrollbar* sbar = nullptr);
        bool operator==(const WaveZoomShift& other) const;
        bool operator!=(const WaveZoomShift& other) const { return ! operator==(other); }
        bool isNull() const;
        bool sameHistory(const WaveZoomShift& other) const;
        int width() const { return mWidth; }
        double scale() const { return mScale; }
        quint64 leftTime() const { return mTleft; }
        QString dumpString() const;
    };
}

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

#include <QWidget>
#include <QMap>
#include <QList>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include "waveform_viewer/wave_transform.h"
#include <QDir>
#include <QTimer>
#include <QTextStream>

namespace hal {
    class WaveItem;
    class WaveTransform;
    class WaveScrollbar;
    class WaveFormPainted;
    class WaveLoaderThread;
    class WaveGraphicsCanvas;
    class WaveDataList;
    class WaveItemHash;
    class WaveDataTimeframe;

    class WaveLoaderThread : public QThread
    {
        Q_OBJECT
        WaveItem* mItem;
        QDir mWorkDir;
        const WaveTransform* mTransform;
        const WaveScrollbar* mScrollbar;
        const WaveDataTimeframe& mTimeframe;
    public:
        WaveLoaderThread(WaveItem* parentItem, const QString& workdir,
                         const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe);
        void run() override;
    };

    class WaveLoaderBackbone : public QThread
    {
        Q_OBJECT
        QList<WaveItem*> mTodoList;
        QDir mWorkDir;
        const WaveTransform* mTransform;
        const WaveScrollbar* mScrollbar;
        const WaveDataTimeframe& mTimeframe;
    public:
        bool mLoop;
        WaveLoaderBackbone(const QList<WaveItem*>& todo, const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe, QObject* parent = nullptr)
            : QThread(parent), mTodoList(todo), mWorkDir(workdir), mTransform(trans), mScrollbar(sbar), mTimeframe(tframe), mLoop(true) {;}
        void run() override;
    };

    class WaveRenderEngine : public QWidget
    {
        Q_OBJECT

        WaveGraphicsCanvas* mWaveGraphicsCanvas;
        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;
        WaveZoomShift mValidity;
        int mY0;
        int mHeight;

        QTimer* mTimer;
        int mTimerTick;
        WaveLoaderBackbone* mBackbone;
        QList<WaveZoomShift> mZoomHistory;
        bool mOmitHistory;
    Q_SIGNALS:
        void updateSoon();
    private Q_SLOTS:
        void callUpdate();
        void handleTimeout();
        void handleBackboneFinished();
    protected:
        void paintEvent(QPaintEvent *event) override;
    public:
        WaveRenderEngine(WaveGraphicsCanvas *wsa, WaveDataList* wdlist, WaveItemHash* wHash, QWidget* parent = nullptr);

        int maxHeight() const;
        int y0Entry(int irow) const;

        QList<WaveZoomShift>& zoomHistory() { return mZoomHistory; }
        void setZoomHistory(const QList<WaveZoomShift>& hist) { mZoomHistory = hist; }
        void omitHistory() { mOmitHistory = true; }
    };
}

#pragma once

#include <QWidget>
#include <QMap>
#include <QList>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include "netlist_simulator_controller/saleae_directory.h"
#include "waveform_viewer/wave_form_painted.h"
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
        void omitHistory() { mOmitHistory = true; }
    };
}

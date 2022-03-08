#include "waveform_viewer/wave_render_engine.h"
#include "waveform_viewer/wave_graphics_canvas.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_form_painted.h"
#include <QDebug>
#include <QDir>
#include <QPaintEvent>
#include "netlist_simulator_controller/saleae_file.h"
#include "waveform_viewer/wave_data_provider.h"
#include "waveform_viewer/wave_item.h"

namespace hal {


    WaveLoaderThread::WaveLoaderThread(WaveItem* parentItem, const QString &workdir,
                                       const WaveTransform* trans, const WaveScrollbar* sbar)
        : QThread(parentItem), mItem(parentItem), mWorkDir(workdir), mTransform(trans), mScrollbar(sbar)
    {;}


    void WaveLoaderThread::run()
    {
        mItem->setState(WaveItem::Loading);
        const WaveData* wd = mItem->wavedata();
        switch (wd->netType()) {
        case WaveData::ClockNet:
        {
            try {
                SimulationInput::Clock clk = static_cast<const WaveDataClock*>(wd)->clock();
                WaveDataProviderClock wdpClk(clk);
                mItem->mPainted.generate(&wdpClk,mTransform,mScrollbar,&mItem->mLoop);
                mItem->setState(WaveItem::Finished);
            } catch (...) {
                mItem->setState(WaveItem::Failed);
            }
            break;
        case WaveData::NetGroup:
            if (wd->isLoadable())
            {
                try {
                    const WaveDataGroup* grp = static_cast<const WaveDataGroup*>(wd);
                    for (WaveData* wdChild : grp->children())
                    {
                        if ((u64)wdChild->data().size() < wdChild->fileSize())
                        {
                            mItem->setState(WaveItem::Aborted);
                            break;
                        }
                    }
                    mItem->mPainted.clearPrimitives();
                    if (mItem->isAborted()) return;
                    WaveDataProviderMap wdpMap(wd->data());
                    wdpMap.setGroup(true,wd->bits(),wd->valueBase());
                    mItem->mPainted.generate(&wdpMap,mTransform,mScrollbar,&mItem->mLoop);
                    mItem->setState(WaveItem::Finished);
                } catch (...) {
                    mItem->setState(WaveItem::Failed);
                }
            }
            break;
       default:
            SaleaeInputFile sif(mWorkDir.absoluteFilePath(QString("digital_%1.bin").arg(mItem->wavedata()->fileIndex())).toStdString());
            if (sif.good())
            {
                if (mItem->wavedata()->isLoadable())
                {
                    try {

                        if ((u64)wd->data().size() < wd->fileSize())
                            mItem->loadSaleae(sif);
                        mItem->mPainted.clearPrimitives();
                        if (mItem->isAborted()) return;
                        WaveDataProviderMap wdpMap(wd->data());
                        wdpMap.setGroup(mItem->isGroup(),mItem->wavedata()->bits(),mItem->wavedata()->valueBase());
                        mItem->mPainted.generate(&wdpMap,mTransform,mScrollbar,&mItem->mLoop);
                        mItem->setState(WaveItem::Finished);
                    } catch (...) {
                        mItem->setState(WaveItem::Failed);
                    }
                }
                else
                {
                    try {
                        WaveDataProviderFile wdpFile(sif);
                        // TODO : test group
                        mItem->mPainted.generate(&wdpFile,mTransform,mScrollbar,&mItem->mLoop);
                        mItem->setState(WaveItem::Finished);
                    } catch (...) {
                        mItem->setState(WaveItem::Failed);
                    }
                }
            }
            else
                mItem->setState(WaveItem::Failed);
            break;
            }
        }
    }

    //------------------------

    void WaveLoaderBackbone::run()
    {
        while (mLoop)
        {
            int needStillWork = 0;
            for (WaveItem* wree : mTodoList)
            {
                if (!wree->isNull()) continue;
                ++ needStillWork;
                if (!wree->mMutex.tryLock()) continue;
                if (wree->isNull())
                {
                    wree->setState(WaveItem::Loading);
                    wree->mMutex.unlock();
                    const WaveData* wd = wree->wavedata();
                    if (wd->netType() == WaveData::ClockNet)
                    {
                        try {
                            SimulationInput::Clock clk = static_cast<const WaveDataClock*>(wd)->clock();
                            WaveDataProviderClock wdpClk(clk);
                            wree->mPainted.generate(&wdpClk,mTransform,mScrollbar,&wree->mLoop);
                            wree->setState(WaveItem::Finished);
                        } catch (...) {
                            wree->setState(WaveItem::Failed);
                        }
                    }
                    else
                    {
                        SaleaeInputFile sif(mWorkDir.absoluteFilePath(QString("digital_%1.bin").arg(wree->wavedata()->fileIndex())).toStdString());
                        if (wree->wavedata()->isLoadable())
                        {
                            try {
                                if ((u64)wd->data().size() < wd->fileSize())
                                    wree->loadSaleae(sif);
                                wree->mPainted.clearPrimitives();
                                WaveDataProviderMap wdpMap(wd->data());
                                wdpMap.setGroup(wree->isGroup(),wree->wavedata()->bits(),wree->wavedata()->valueBase());
                                wree->mPainted.generate(&wdpMap,mTransform,mScrollbar,&wree->mLoop);
                                wree->setState(WaveItem::Painted);
                            } catch (...) {
                                wree->setState(WaveItem::Failed);
                            }
                        }
                        else
                        {
                            try {
                                WaveDataProviderFile wdpFile(sif);
                                // TODO : test group
                                wree->mPainted.generate(&wdpFile,mTransform,mScrollbar,&wree->mLoop);
                                wree->setState(WaveItem::Painted);
                            } catch (...) {
                                wree->setState(WaveItem::Failed);
                            }
                        }
                    }
                }
                else
                    wree->mMutex.unlock();
                if (!mLoop) break;;
            }
            if (!needStillWork) break;
        }
    }

    //------------------------
    WaveRenderEngine::WaveRenderEngine(WaveGraphicsCanvas *wsa, WaveDataList *wdlist, WaveItemHash *wHash, QWidget *parent)
        : QWidget(parent), mWaveGraphicsCanvas(wsa), mWaveDataList(wdlist), mWaveItemHash(wHash),
          mY0(0), mHeight(0), mTimerTick(0), mBackbone(nullptr)
    {
        connect(this,&WaveRenderEngine::updateSoon,this,&WaveRenderEngine::callUpdate,Qt::QueuedConnection);
        mTimer = new QTimer(this);
        connect(mTimer,&QTimer::timeout,this,&WaveRenderEngine::handleTimeout);
        mTimer->start(50);
    }

    void WaveRenderEngine::handleTimeout()
    {
        mWaveItemHash->emptyTrash();
        bool needUpdate = false;
        for (WaveItem* wree : mWaveItemHash->values())
        {
            if ((wree->isLoading() || wree->isThreadBusy()) && wree->mVisibleRange)
            {
                wree->incrementLoadProgress();
                needUpdate = true;
            }
        }
        if (!needUpdate) return;
        if (++mTimerTick > 2)
        {
            update();
        }
    }

    void WaveRenderEngine::handleBackboneFinished()
    {
        WaveLoaderBackbone* toDelete = mBackbone;
        mBackbone = nullptr;
        toDelete->deleteLater();
    }

    void WaveRenderEngine::callUpdate()
    {
        update();
    }

    void WaveRenderEngine::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);

        int top = mWaveGraphicsCanvas->verticalScrollBar()->value();
        const WaveTransform* trans = mWaveGraphicsCanvas->transform();
        const WaveScrollbar* sbar  = static_cast<const WaveScrollbar*>(mWaveGraphicsCanvas->horizontalScrollBar());
        mTimerTick = 0;

        WaveFormPaintValidity testValid(trans,sbar);

        if (testValid != mValidity)
        {
            if (mBackbone)
                mBackbone->mLoop = false;
            mValidity = testValid;
        }

        mY0 = top;
        mHeight = height();

        QString workdir = QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory());

        for (WaveItem* wree : mWaveItemHash->values())
        {
//            if (wree->yPosition() < 3)
//                qDebug() << "paint" << wree->yPosition() << wree->wavedata()->name() << wree->wavedata()->fileSize() << wree->wavedata()->data().size();
            if (wree->hasRequest(WaveItem::AddRequest))
            {
                wree->setParent(this);
                connect(wree,&WaveItem::doneLoading,this,&WaveRenderEngine::callUpdate);
                wree->clearRequest(WaveItem::AddRequest);
            }

            int y0 = y0Entry(wree->yPosition());

            if (y0 < 0 || y0 > mHeight)
            {
                wree->mVisibleRange = false;
                // not in painted range
                if (wree->isThreadBusy()) continue;

                if (wree->isLoading() && wree->hasLoader())
                {
                    if (!wree->mMutex.tryLock())
                        continue;
                    else
                    {
                        wree->abortLoader();
                        wree->mMutex.unlock();
                    }
                }
                continue;
            }

            wree->mVisibleRange = true;
            if (!wree->mMutex.tryLock()) continue;
            else
                {
                if (wree->isNull())
                {
                    if (wree->isGroup() && !wree->wavedata()->isLoadable())
                    {
                        if (wree->mPainted.generateGroup(wree->wavedata(),mWaveItemHash))
                        {
                            wree->setState(WaveItem::Painted);
                            Q_EMIT updateSoon();
                        }
                    }
                    else
                        wree->startGeneratePainted(workdir,trans, sbar);
                }
                else if (wree->isLoading() || wree->isThreadBusy())
                {
                    QColor barColor = QColor::fromRgb(255,80,66,40);
                    if (wree->isLoading())
                    {
                        if (wree->mLoadValidity == mValidity)
                            barColor = QColor::fromRgb(50,255,66,40);
                        else
                            wree->abortLoader();
                    }
                    painter.fillRect(QRectF(0,y0,wree->loadeProgress(),14),
                                     QBrush(barColor));
                }
                else if (wree->isPainted())
                {
                    if (mValidity == wree->mPainted.validity() && !wree->wavedata()->isDirty() &&!wree->hasRequest(WaveItem::DataChanged))
                    {
                        if (wree->isVisibile())
                        {
                            if (wree->isSelected())
                            {
                                painter.fillRect(QRectF(0,y0-4,width(),28), QBrush(QColor(WaveItem::sLineColor[WaveItem::Background])));
                                painter.setPen(QPen(QColor(WaveItem::sLineColor[WaveItem::HiLight]),0));
                            }
                            else
                                painter.setPen(QPen(QColor(WaveItem::sLineColor[WaveItem::Solid]),0));
                            wree->mPainted.paint(y0,painter);
                        }
                    }
                    else
                    {
                        wree->deletePainted();
                        if (wree->isGroup() && !wree->wavedata()->isLoadable())
                            wree->mPainted.generateGroup(wree->wavedata(),mWaveItemHash);
                        else
                            wree->startGeneratePainted(workdir,trans, sbar);
                        wree->clearRequest(WaveItem::DataChanged);
                    }
                }
                wree->mMutex.unlock();
            }
        }

        QList<WaveItem*> todoList;

        for (WaveItem* wree : mWaveItemHash->values())
        {
            if (wree->isNull() && !wree->isGroup()) todoList.append(wree);
        }

        if (!todoList.isEmpty() && !mBackbone)
        {
            mBackbone = new WaveLoaderBackbone(todoList, QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory()), trans, sbar, this);
            connect(mBackbone, &QThread::finished, this, &WaveRenderEngine::handleBackboneFinished);
            mBackbone->start();
        }

//        qDebug() << "render" << y0 << mScale << mTleft << mWidth;
    }

    int WaveRenderEngine::y0Entry(int irow) const
    {
        return 32 + irow*28 - mY0;
    }

    int WaveRenderEngine::maxHeight() const
    {
        return (mWaveItemHash->size()+2) * 28;
    }
}


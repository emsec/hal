#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>

namespace hal {

    WaveWidget::WaveWidget(QWidget *parent)
        : QSplitter(parent)
    {
        mFrame = new QFrame(this);
        mFrame->setLineWidth(3);
        mFrame->setFrameStyle(QFrame::Sunken);
        mFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        addWidget(mFrame);
        mWaveView = new WaveView(this);
        mWaveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mWaveScene = new WaveScene(mWaveView);

        connect(mWaveView,&WaveView::changedXscale,mWaveScene,&WaveScene::xScaleChanged);
        mWaveView->setScene(mWaveScene);

        connect(mWaveScene,&WaveScene::cursorMoved,this,&WaveWidget::handleCursorMoved);
        connect(mWaveView,&WaveView::relativeYScroll,this,&WaveWidget::handleYScroll);

    }

    void WaveWidget::updateIndices()
    {
        int n = mValues.size();
        float xpos = mWaveScene->cursorPos();
        for (int i=0; i<n; i++)
        {
            mValues.at(i)->setDataIndex(i);
            updateLabel(i,xpos);
        }
        mWaveIndices.clear();
        for (int i=0; i<n; i++)
            mWaveIndices.insert(mWaveScene->waveData(i)->name(),i);
    }

    void WaveWidget::deleteWave(int dataIndex)
    {
        WaveLabel* wl = mValues.at(dataIndex);
        mValues.removeAt(dataIndex);
        mWaveScene->deleteWave(dataIndex);
        updateIndices();
        wl->deleteLater();
    }

    void WaveWidget::addOrReplaceWave(WaveData* wd)
    {
        auto it = mWaveIndices.find(wd->name());
        if (it != mWaveIndices.end())
        {
            mWaveScene->setWaveData(it.value(),wd);
            return;
        }
        int inx = mWaveScene->addWave(wd);
        Q_ASSERT(mValues.size() == inx);
        WaveLabel* wl = new WaveLabel(inx,mWaveScene->waveData(inx)->name(),mFrame);
        connect(wl,&WaveLabel::doubleClicked,this,&WaveWidget::editWaveData);
        connect(wl,&WaveLabel::triggerDelete,this,&WaveWidget::deleteWave);
        connect(wl,&WaveLabel::triggerSwap,this,&WaveWidget::handleLabelSwap);
        connect(wl,&WaveLabel::triggerMove,this,&WaveWidget::handleLabelMove);
        wl->setFixedWidth(250);
        wl->show();
        mValues.append(wl);
        mWaveIndices.insert(wd->name(),inx);
        updateLabel(inx,mWaveScene->cursorPos());
        resizeEvent(nullptr);
    }

    void WaveWidget::resizeEvent(QResizeEvent *event)
    {
        int w = event ? event->size().width() : width();
        int maxw = w / 2;
        int minw = 10;
        int n = mValues.size();
        for (int i=0; i<n; i++)
            if (mValues.at(i)->width()>minw)
                minw = mValues.at(i)->width();
        minw += 6;
        if (minw > maxw) minw = maxw;
        setSizes(QList<int>() << minw << w - minw );
    }

    const WaveData* WaveWidget::waveDataByName(const QString& name) const
    {
        int inx = mWaveIndices.value(name,-1);
        if (inx < 0) return nullptr;
        return mWaveScene->waveData(inx);
    }

    void WaveWidget::editWaveData(int dataIndex)
    {
        const WaveData* editorInput = mWaveScene->waveData(dataIndex);
        if (!editorInput) return;
        WaveData wd(*editorInput);
        WaveEditDialog wed(wd,this);
        if (wed.exec() != QDialog::Accepted) return;
        mWaveScene->setWaveData(dataIndex,wed.dataFactory()); // TODO : sanitize input data
    }

    void WaveWidget::updateLabel(int dataIndex, float xpos)
    {
        WaveLabel* wl = mValues.at(dataIndex);
        if (wl->state()>=2) return;
        QPoint pos = mWaveView->mapFromScene(QPointF(xpos,mWaveScene->yPosition(dataIndex)-1));
        pos.setX(3);
        wl->setValue(mWaveScene->waveData(dataIndex)->tValue(xpos));
        wl->move(pos);
    }

    void WaveWidget::handleYScroll(int dy)
    {
        for (WaveLabel* wl : mValues)
            wl->move(3,wl->pos().y() + dy);
    }

    void WaveWidget::handleCursorMoved(float xpos)
    {
        mWaveView->handleCursorMoved(xpos);

        for (int i=0; i<mWaveScene->numberWaves(); i++)
            updateLabel(i,xpos);
    }

    void WaveWidget::handleLabelMove(int isource, int ypos)
    {
        int itarget = targetIndex(ypos);
        int n = mValues.size();
        for (int i=0; i<n; i++)
        {
            if (i == isource) continue;
            WaveLabel* wl = mValues.at(i);
            wl->setState(i == itarget ? 2 : 0);
            wl->update();
        }
        update();
    }

    int WaveWidget::targetIndex(int ypos)
    {
        int n = mValues.size() -1;
        if (n <= 0) return -1;
        int labh = mValues.at(0)->height();
        int y0 = + mValues.at(0)->pos().y();
        int yn = + mValues.at(n)->pos().y();

        int retval = (int) floor ( (1.5 + labh + ypos - y0) * n / (yn - y0));
        if (retval > n) return -1;
        return retval;
    }

    void WaveWidget::handleLabelSwap(int isource, int ypos)
    {

        int n = mValues.size();
        for (int i=0; i<n; i++)
            mValues.at(i)->setState(0);

        int itarget = targetIndex(ypos);
        if (itarget >= 0 && itarget != isource)
        {
            WaveLabel* wl = mValues.at(isource);
            mValues.removeAt(isource);
            mValues.insert(itarget < isource ? itarget : itarget-1, wl);
            updateIndices();
            mWaveScene->moveToIndex(isource,itarget);
        }
        for (int i=0; i<n; i++)
            mValues.at(i)->update();
        update();
    }

}

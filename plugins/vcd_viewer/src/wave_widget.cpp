#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QScrollBar>

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
    }

    /*
    void WaveWidget::deleteWave(int dataIndex)
    {
        WaveScene* sc = static_cast<WaveScene*>(scene());
        WaveLabel* wl = mValues.at(dataIndex);
        mValues.removeAt(dataIndex);
        sc->deleteWave(dataIndex);
        int n = mValues.size();
        for (int i=dataIndex; i<n; i++)
        {
            float xpos = sc->cursorPos();
            mValues.at(i)->setDataIndex(i);
            updateLabel(i,xpos);
        }
        wl->deleteLater();
        mWaveIndices.clear();
        for (int i=0; i<n; i++)
            mWaveIndices.insert(sc->waveData(i)->name(),i);
    }
*/

    void WaveWidget::addOrReplaceWave(WaveData* wd)
    {
        mWaveView->addOrReplaceWave(wd);
    }

    const WaveData* WaveWidget::waveDataByName(const QString& name) const
    {
        return mWaveView->waveDataByName(name);
    }
/*
    void WaveWidget::editWaveData(int dataIndex)
    {
        WaveScene* sc = static_cast<WaveScene*>(scene());
        const WaveData* editorInput = sc->waveData(dataIndex);
        if (!editorInput) return;
        WaveData wd(*editorInput);
        WaveEditDialog wed(wd,this);
        if (wed.exec() != QDialog::Accepted) return;
        sc->setWaveData(dataIndex,wed.dataFactory());
    }


    void WaveWidget::restoreCursor()
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        int x0 = mapFromScene(0,0).x();
        if (mCursorPixelPos < x0)
            // last cursor pixel pos not in diagram
            sc->setCursorPos(sc->cursorPos(),false);
        else
        {
            QPointF sCurs = mapToScene(mCursorPixelPos,0);
            sc->setCursorPos(sCurs.x(),false);
        }
    }

    void WaveWidget::updateLabel(int dataIndex, float xpos)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (!sc) return;
        WaveLabel* wl = mValues.at(dataIndex);
        QPoint pos = mapFromScene(QPointF(xpos,sc->yPosition(dataIndex)-1));

        int ix = pos.x();

        if (ix < 0 || ix >= width())
            ix = 0;
        else
            mCursorPixelPos = ix;
        pos.setX(ix + 5);
        wl->setValue(sc->waveData(dataIndex)->tValue(xpos));
        wl->move(pos);
    }

    void WaveWidget::handleCursorMoved(float xpos)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (!sc) return;

        if (xpos == mLastCursorPos) return;
        mLastCursorPos = xpos;

        for (int i=0; i<sc->numberWaves(); i++)
            updateLabel(i,xpos);
    }
    */
}

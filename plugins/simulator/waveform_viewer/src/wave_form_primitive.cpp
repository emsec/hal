#include "waveform_viewer/wave_form_primitive.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>
#include <math.h>

namespace hal {
    void WaveFormPrimitiveHline::paint(int y0, QPainter& painter)
    {
        int y = 14 * (1-mValue) + y0;
        painter.drawLine(QLineF(mX0,y,mX1,y));
    }

    void WaveFormPrimitiveTransition::paint(int y0, QPainter& painter)
    {
        painter.drawLine(QLineF(mX0,y0,mX0,y0+14));
    }

    void WaveFormPrimitiveTrigger::paint(int y0, QPainter& painter)
    {
        QBrush saveBrush = painter.brush();
        painter.setBrush(painter.pen().color());
        painter.drawLine(QLineF(mX0,y0+2,mX0,y0+10));
        QPoint pts[5];
        const int px[5] = { 0, 3, 0, -3, 0 } ;
        const int py[5] = { 3, 6, 0,  6, 3 } ;
        for (int i=0; i<5; i++)
        {
            pts[i] = QPoint(px[i]+mX0, py[i]+y0+2);
        }
        painter.drawPolygon(pts,5);
        painter.setBrush(saveBrush);
    }


    void WaveFormPrimitiveUndefined::paint(int y0, QPainter& painter)
    {
        float y = 14 * 0.5 + y0;
        QPen savePen = painter.pen();
        QPen dashPen(QColor(NetlistSimulatorControllerPlugin::sSimulationSettings->color(SimulationSettings::WaveformUndefined)),0);
        dashPen.setStyle(Qt::DotLine);
        painter.setPen(dashPen);
        painter.drawLine(QLineF(mX0,y,mX1,y));
        painter.setPen(savePen);
    }

    WaveFormPrimitiveFilled::WaveFormPrimitiveFilled(float x0, float x1, int val)
        : WaveFormPrimitive(x0,x1)
    {
        memset(mAccumTime, 0, sizeof(mAccumTime));
        if (0 <= val && val <= 1)
            mAccumTime[val] = x1-x0;
    }

    void WaveFormPrimitiveFilled::add(const WaveFormPrimitiveFilled &other)
    {
        for (int i=0; i<2; i++)
            mAccumTime[i] += other.mAccumTime[i];
    }

    void WaveFormPrimitiveFilled::paint(int y0, QPainter& painter)
    {
        QBrush saveBrush = painter.brush();
        QColor fillCol = painter.pen().color();
        double sum = mAccumTime[0] + mAccumTime[1];
        if (sum > 0)
        {
            if (mAccumTime[1] > mAccumTime[0])
            {
                painter.drawLine(QLineF(mX0,0,mX1,0));
                fillCol.setAlpha(floor(55 + 400 * mAccumTime[0] / sum));
            }
            else
            {
                painter.drawLine(QLineF(mX0,14,mX1,14));
                fillCol.setAlpha(floor(55 + 400 * mAccumTime[1] / sum));
            }
        }
        painter.setBrush(QBrush(fillCol));
        painter.drawRect(QRectF(mX0,y0,mX1-mX0,14));
        painter.setBrush(saveBrush);
    }

    WaveFormPrimitiveValue::WaveFormPrimitiveValue(float x0, float x1, int val, int bits, int base)
        : WaveFormPrimitive(x0,x1), mValue(val), mBits(bits), mBase(base)
    {
        QVector<QPointF> pts;
        pts.append(QPointF(x0,7));
        if (mX1-mX0 > 8)
        {
            pts.append(QPointF(x0+4,14));
            pts.append(QPointF(x1-4,14));
        }
        else
        {
            pts.append(QPointF((x0+x1)/2.,14));
        }
        pts.append(QPointF(x1,7));
        if (mX1-mX0 > 8)
        {
            pts.append(QPointF(x1-4,0));
            pts.append(QPointF(x0+4,0));
        }
        else
        {
            pts.append(QPointF((x0+x1)/2.,0));
        }
        mPolygon = QPolygonF(pts);
    }

    void WaveFormPrimitiveValue::paint(int y0, QPainter& painter)
    {
        painter.drawPolygon(mPolygon.translated(0,y0));
        QFont font = painter.font();
        font.setPixelSize(8);
        font.setBold(true);
        QString txt = WaveData::stringValue(mValue,mBits,mBase);
        QFontMetrics fm(font);
        QRectF r = fm.boundingRect(txt);
        r.setWidth(r.width()+4*txt.size());
        float wMax = mX0 < 0 ? mX1 : mX1 - mX0;
        if (r.width()+6 > wMax) return;
        r.setHeight(14);
        float xText = mX0 + (wMax - r.width())/2.;
        if (xText<0) xText = 0;
        r.moveTo(xText, y0-1);
        painter.drawText(r,txt);
    }
}

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QApplication>
#include <QFontMetricsF>

namespace hal
{
    QColor GraphicsNode::sTextColor;
    QFont GraphicsNode::sTextFont[3];
    qreal GraphicsNode::sTextFontHeight[3] = {0, 0, 0};


    GraphicsNode::GraphicsNode(const ItemType type, const u32 id)
        : GraphicsItem(type, id), mMaxTextWidth(0)
    {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

        //setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | ItemIsFocusable);
        //setAcceptHoverEvents(true);
    }

    void GraphicsNode::loadSettings()
    {
        QFont font = QFont("Iosevka");
        font.setPixelSize(graph_widget_constants::sFontSize);

        for (int iline=0; iline<3; iline++)
        {
            sTextFont[iline] = font;
            QFontMetricsF fmf(font);
            sTextFontHeight[iline] = fmf.height();
        }
    }

    void GraphicsNode::setNodeText(const QString* lines, bool init)
    {
        for (int iline=0; iline<3; iline++)
        {
            if (lines[iline].isEmpty()) continue;
            QFontMetricsF fmf(sTextFont[iline]);
            mNodeText[iline] = lines[iline];
            qreal textWidth = fmf.width(mNodeText[iline]);
            if (init)
            {
                if (textWidth > mMaxTextWidth)
                    mMaxTextWidth = textWidth;
            }
            else
            {
                if (textWidth > mMaxTextWidth)
                {
                    QString shorter = mNodeText[iline];
                    while (!mNodeText[iline].isEmpty() && textWidth > mMaxTextWidth)
                    {
                        shorter.chop(1);
                        textWidth = fmf.width(shorter + "…");
                    }
                    mNodeText[iline] = shorter + "…";
                }
                mTextPosition[iline].setX(mWidth / 2 - textWidth / 2);
            }
        }
    }

    void GraphicsNode::setMaxTextWidth(qreal maxTw)
    {
        mMaxTextWidth = maxTw;
        for (int iline=0; iline<3; iline++)
        {
            QString shorter = mNodeText[iline];
            QString txt = mNodeText[iline];
            qreal textWidth = 0;
            do {
                textWidth = QFontMetricsF(sTextFont[iline]).width(txt);
                if (textWidth > mMaxTextWidth)
                {
                    shorter.chop(1);
                    txt = shorter + "…";
                }
            } while (textWidth > mMaxTextWidth);
            mNodeText[iline] = txt;
        }
    }

    void GraphicsNode::initTextPosition(qreal y0, qreal spacing, qreal x0)
    {
        for (int iline=0; iline<3; iline++)
        {
            QFontMetricsF fmf(sTextFont[iline]);
            qreal textWidth = fmf.width(mNodeText[iline]);
            y0 += sTextFontHeight[iline];
            mTextPosition[iline].setX(x0 < 0 ? mWidth / 2 - textWidth / 2 : x0);
            mTextPosition[iline].setY(y0);
            y0 += spacing;
        }
    }


    QRectF GraphicsNode::boundingRect() const
    {
        return QRectF(0, 0, mWidth, mHeight);
    }

    QList<QLineF> GraphicsNode::connectors(u32 netId, bool isInput) const
    {
        Q_UNUSED(netId);
        Q_UNUSED(isInput);
        return QList<QLineF>();
    }

    QPainterPath GraphicsNode::shape() const
    {
        QPainterPath path;
        path.addRect(QRectF(0, 0, mWidth, mHeight));
        return path;
    }

    void GraphicsNode::setVisuals(const GraphicsNode::Visuals& v)
    {
        setVisible(v.mVisible);

        mColor = v.mMainColor;
    }

    qreal GraphicsNode::width() const
    {
        return mWidth;
    }

    qreal GraphicsNode::height() const
    {
        return mHeight;
    }

    QList<u32> GraphicsNode::inputNets() const
    {
        if (mInputByNet.isEmpty()) return QList<u32>();
        int n = mInputByNet.size();
        QVector<u32> retval(n);
        for (auto it=mInputByNet.constBegin(); it!=mInputByNet.constEnd(); ++it)
        {
            Q_ASSERT(it.value() < n);
            retval[it.value()] = it.key();
        }
        return retval.toList();
    }

    QList<u32> GraphicsNode::outputNets() const
    {
        if (mOutputByNet.isEmpty()) return QList<u32>();
        int n = mOutputByNet.size();
        QVector<u32> retval(n);
        for (auto it=mOutputByNet.constBegin(); it!=mOutputByNet.constEnd(); ++it)
        {
            Q_ASSERT(it.value() < n);
            retval[it.value()] = it.key();
        }
        return retval.toList();
    }

    int GraphicsNode::inputByNet(u32 netId) const
    {
        return mInputByNet.value(netId,-1);
    }

    int GraphicsNode::outputByNet(u32 netId) const
    {
        return mOutputByNet.value(netId,-1);
    }

    void GraphicsNode::set_name(const QString &name)
    {
        mNodeText[0] = name;
    }
}

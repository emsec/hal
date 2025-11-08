/***************************************************************
QGVCore
Copyright (c) 2014, Bergont Nicolas, All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
***************************************************************/
#include "QGVEdge.h"
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVEdgePrivate.h>
#include <QDebug>
#include <QPainter>

QGVEdge::QGVEdge(QGVEdgePrivate *edge, QGVScene *scene) :  _scene(scene), _edge(edge), _head_node(nullptr), _tail_node(nullptr)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QGVEdge::~QGVEdge()
{
    _scene->removeItem(this);
		delete _edge;
}

QString QGVEdge::label() const
{
    return getAttribute("label");
}

QRectF QGVEdge::boundingRect() const
{
    return _path.boundingRect() | _head_arrow.boundingRect() | _tail_arrow.boundingRect() | _label_rect;
}

void QGVEdge::setEndpoints(const QHash<void*,QGVNode*>& nodeHash)
{
    auto agHead = _edge->endpoint(true);
    if (agHead) _head_node = nodeHash.value(agHead, nullptr);
    auto agTail = _edge->endpoint(false);
    if (agTail) _tail_node = nodeHash.value(agTail, nullptr);
}

QGVNode* QGVEdge::headNode() const
{
    return _head_node;
}
QGVNode* QGVEdge::tailNode() const
{
    return _tail_node;
}

QPainterPath QGVEdge::shape() const
{
    QPainterPathStroker ps;
    ps.setCapStyle(_pen.capStyle());
    ps.setWidth(_pen.widthF() + 10);
    ps.setJoinStyle(_pen.joinStyle());
    ps.setMiterLimit(_pen.miterLimit());
    return ps.createStroke(_path);
}

void QGVEdge::setLabel(const QString &label)
{
    setAttribute("label", label);
}

void QGVEdge::setHightlight(const QString& hilight)
{
    _highlight_text = hilight;
    update();
}

void QGVEdge::paintText(QPainter* painter)
{
    int pos = _highlight_text.isEmpty() ? -1 : _label.indexOf(_highlight_text);

    if (pos < 0)
    {
        // Not much to do, pen already set in paint() method
        painter->drawText(_label_rect, Qt::AlignLeft, _label);
        return;
    }

    int lastPos = 0;
    int count = 0;
    QFontMetrics fm(painter->font());
    qreal x = _label_rect.x();

    for (;;)
    {
        QPen tpen(_pen);

        QString part;
        if (count % 2 > 0)
        {
            // highlight
            tpen.setColor(QGVStyle::instance()->penColor(true,_pen.color()));
            lastPos = pos + _highlight_text.size();
            part = _label.mid(pos, lastPos-pos);
            pos = _label.indexOf(_highlight_text, lastPos);
        }
        else
        {
            // normal
            tpen.setColor(QGVStyle::instance()->penColor(false,_pen.color()));
            int len = pos < 0 ? _label.size() - lastPos : pos - lastPos;
            if (len > 0) part = _label.mid(lastPos,len);
        }
        if (!part.isEmpty())
        {
            painter->setPen(tpen);
            painter->drawText(x,_label_rect.top(), part);
            x += fm.horizontalAdvance(part);
        }
        if (pos < 0 && count%2 == 0) break;
        ++count;
    }
}

void QGVEdge::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();

    QPen tpen(_pen);
    tpen.setColor(QGVStyle::instance()->penColor(isSelected(),_pen.color()));
    tpen.setWidth(isSelected() ? 2 : 1);
    painter->setPen(tpen);

    painter->drawPath(_path);

    tpen.setWidth(1);
    painter->setPen(tpen);
    /*
    QRectF pp = _path.controlPointRect();
    if(pp.width() < pp.height())
    {
        painter->save();
        painter->translate(_label_rect.topLeft());
        painter->rotate(90);
        painter->drawText(QRectF(QPointF(0, -_label_rect.width()), _label_rect.size()), Qt::AlignCenter, _label);
        painter->restore();
    }
    else
    */

    painter->setBrush(QBrush(tpen.color(), Qt::SolidPattern));
    painter->drawPolygon(_head_arrow);
    painter->drawPolygon(_tail_arrow);
    paintText(painter);
    painter->restore();
}

void QGVEdge::setAttribute(const QString &name, const QString &value)
{
    char empty[] = "";
    agsafeset(_edge->edge(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), empty);
}

QString QGVEdge::getAttribute(const QString &name) const
{
		char* value = agget(_edge->edge(), name.toLocal8Bit().data());
    if(value)
        return value;
    return QString();
}

void QGVEdge::updateLayout()
{
    prepareGeometryChange();

		qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());

		const splines* spl = ED_spl(_edge->edge());
    _path = QGVCore::toPath(spl, gheight);


    //Edge arrows
    if((spl->list != 0) && (spl->list->size%3 == 1))
    {
        if(spl->list->sflag)
        {
            _tail_arrow = toArrow(QLineF(QGVCore::toPoint(spl->list->list[0], gheight), QGVCore::toPoint(spl->list->sp, gheight)));
        }

        if(spl->list->eflag)
        {
            _head_arrow = toArrow(QLineF(QGVCore::toPoint(spl->list->list[spl->list->size-1], gheight), QGVCore::toPoint(spl->list->ep, gheight)));
        }
    }

    _pen.setWidth(1);
    _pen.setColor(QGVCore::toColor(getAttribute("color")));
    _pen.setStyle(QGVCore::toPenStyle(getAttribute("style")));

    //Edge label
    textlabel_t *xlabel = ED_xlabel(_edge->edge());
    if (!xlabel) xlabel = ED_label(_edge->edge());

    if(xlabel)
    {
        _label = xlabel->text;
        _label_rect.setSize(QSize(xlabel->dimen.x, xlabel->dimen.y));
				_label_rect.moveCenter(QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_scene->_graph->graph())));
    }

    setToolTip(getAttribute("tooltip"));
}

QPolygonF QGVEdge::toArrow(const QLineF &line) const
{
    QLineF n = line.normalVector();
    QPointF o(n.dx() / 3.0, n.dy() / 3.0);

    //Only support normal arrow type
    QPolygonF polygon;
    polygon.append(line.p1() + o);
    polygon.append(line.p2());
    polygon.append(line.p1() - o);

    return polygon;
}

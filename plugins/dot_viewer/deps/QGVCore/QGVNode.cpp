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
#include <QGVNode.h>
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVNodePrivate.h>
#include <QDebug>
#include <QPainter>

QGVNode::QGVNode(QGVNodePrivate *node, QGVScene *scene): _scene(scene), _node(node)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QGVNode::~QGVNode()
{
    _scene->removeItem(this);
    delete _node;
}

QString QGVNode::label() const
{
    QString retval = getAttribute("label");
    if (!retval.isEmpty() && retval != "\\N") return retval;
    retval = getAttribute("xlabel");
    if (!retval.isEmpty() && retval != "\\N") return retval;
    const char* name  = agnameof(_node->node());
    if (name) return QString(name);
    return QString();
}

void QGVNode::setLabel(const QString &label)
{
    setAttribute("label", label);
}

QRectF QGVNode::boundingRect() const
{
    return _path.boundingRect();
}

void QGVNode::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();

    painter->setPen(_pen);
    painter->setBrush(QGVStyle::instance()->nodeBrush(isSelected(), _brush));
    painter->drawPath(_path);

    painter->setPen(QGVCore::toColor(getAttribute("labelfontcolor")));

    const QRectF rect = boundingRect().adjusted(2,2,-2,-2); //Margin
    if(_icon.isNull())
    {
        painter->drawText(rect, Qt::AlignCenter , QGVNode::label());
    }
    else
    {
        painter->drawText(rect.adjusted(0,0,0, -rect.height()*2/3), Qt::AlignCenter , QGVNode::label());

        const QRectF img_rect = rect.adjusted(0, rect.height()/3,0, 0);
        QImage img = _icon.scaled(img_rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(img_rect.topLeft() + QPointF((img_rect.width() - img.rect().width())/2, 0), img);
    }

    if (isSelected())
    {
        QColor frameColor = QGVStyle::instance()->selectFrameColor();
        if (frameColor.isValid())
        {
            painter->setPen(QPen(frameColor,2));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(rect);
        }
    }
    painter->restore();
}

void QGVNode::setAttribute(const QString &name, const QString &value)
{
    char empty[] = "";
    agsafeset(_node->node(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), empty);
}

QString QGVNode::getAttribute(const QString &name) const
{
    char* value = agget(_node->node(), name.toLocal8Bit().data());
    if(value)
        return value;
    return QString();
}

void QGVNode::setIcon(const QImage &icon)
{
    _icon = icon;
}

QString QGVNode::getShape() const
{
    return QString(ND_shape(_node->node())->name);
}

void QGVNode::updateLayout()
{
    prepareGeometryChange();
    qreal width = ND_width(_node->node())*DotDefaultDPI;
    qreal height = ND_height(_node->node())*DotDefaultDPI;

    //Node Position (center)
    qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());
    setPos(QGVCore::centerToOrigin(QGVCore::toPoint(ND_coord(_node->node()), gheight), width, height));

    //Node on top
    setZValue(1);

    //Node path
    _path = QGVCore::toPath(getShape(), (polygon_t*)ND_shape_info(_node->node()), width, height);
    _pen.setWidth(1);

    _brush.setStyle(QGVCore::toBrushStyle(getAttribute("style")));
    _brush.setColor(QGVCore::toColor(getAttribute("fillcolor")));
    _pen.setColor(QGVCore::toColor(getAttribute("color")));

    setToolTip(getAttribute("tooltip"));
}

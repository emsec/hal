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
#include "QGVCore.h"
#include <QDebug>

qreal QGVCore::graphHeight(Agraph_t *graph)
{
    //Total height of the graph (allows for the inverse calculation of the coordinates)
    return GD_bb(graph).UR.y;
}

QPointF QGVCore::toPoint(pointf p, qreal gheight)
{
    //The Y coordinate system starts from the bottom in graphViz and from the top for Qt!
    return QPointF(p.x, gheight - p.y);
}

QPointF QGVCore::toPoint(point p, qreal gheight)
{
    //The Y coordinate system starts from the bottom in graphViz and from the top for Qt!
    return QPointF(p.x, gheight - p.y);
}

QPointF QGVCore::centerToOrigin(const QPointF &p, qreal width, qreal height)
{
    //The origin of an object is the center in graphViz and the top left for Qt!
    return QPointF(p.x() - width/2, p.y() - height/2);
}

QPolygonF QGVCore::toPolygon(const polygon_t *poly, qreal width, qreal height)
{
    if (poly->peripheries != 1)
        qWarning("unsupported number of peripheries %d", poly->peripheries);

    const int sides = poly->sides;
    const pointf* vertices = poly->vertices;

    QPolygonF polygon;
    for (int side = 0; side < sides; side++)
        polygon.append(QPointF(vertices[side].x + width/2, vertices[side].y + height/2));
    return polygon;
}
QPainterPath QGVCore::cylinder(const QPolygonF& polygon, qreal width, qreal height)
{
    qreal ymax = -1.E99;
    qreal ymin = 1.E99;

    for (QPointF p : polygon)
    {
        if (qFuzzyCompare(p.x(),0))
        {
            if (p.y() < ymin) ymin = p.y();
            if (p.y() > ymax) ymax = p.y();
        }
    }

    QPainterPath path;

    if (ymin <= 0 || qFuzzyCompare(ymin,1.E99))
    {
        path.addPolygon(polygon);
        return path;
    }
    qreal dx = width/2;
    qreal r = (dx*dx + ymin*ymin) / 2 / ymin;
    qreal ang = asin(dx/r) / M_PI * 180;

    // draw clockwise (neg arc value)
    path.moveTo(0,ymax);
    path.lineTo(0,ymin);
    path.arcTo(dx-r,0,2*r,2*r,90+ang, -2*ang);
    path.lineTo(width,ymin);
    path.lineTo(width,ymax);
    path.arcTo(dx-r,height-2*r,2*r,2*r,270+ang, -2*ang);
    path.closeSubpath();

    // inner line
    path.moveTo(width,ymin);
    path.arcTo(dx-r,2*(ymin-r),2*r,2*r,270+ang, -2*ang);
    path.setFillRule(Qt::WindingFill);
    return path;
}

QPainterPath QGVCore::toPath(const QString& shape, const polygon_t *poly, qreal width, qreal height)
{
    QPainterPath path;
    if ( shape == "rectangle" ||
         shape == "box"       ||
         shape == "hexagon"   ||
         shape == "polygon"   ||
         shape == "diamond"   ||
         shape == "rect"      )
    {
        QPolygonF polygon = toPolygon(poly, width, height);
        polygon.append(polygon[0]);
        path.addPolygon(polygon);
    }
    else if ( shape == "ellipse" ||
              shape == "circle" )
    {
        QPolygonF polygon = toPolygon(poly, width, height);
        path.addEllipse(QRectF(polygon[0], polygon[1]));
    }
    else if ( shape == "cylinder")
    {
        QPolygonF polygon = toPolygon(poly, width, height);
        path = cylinder(polygon, width, height);
    }
    else if ( shape == "triangle" ||
              shape == "invtriangle" )
    {
        QVector<QPointF> trianglePoints;
        for (const QPointF& p : toPolygon(poly, width, height))
            trianglePoints.append(QPointF(p.x(),height - p.y()));
        trianglePoints.append(trianglePoints.at(0));
        path.addPolygon(QPolygonF(trianglePoints));
    }
    else
    {
        qWarning("unsupported shape %s", shape.toStdString().c_str());
    }
    return path;
}

QPainterPath QGVCore::toPath(const splines *spl, qreal gheight)
{
    QPainterPath path;
    if((spl->list != 0) && (spl->list->size%3 == 1))
    {
        bezier bez = spl->list[0];
        //If there is a starting point, draw a line from it to the first curve point
        if(bez.sflag)
        {
            path.moveTo(toPoint(bez.sp, gheight));
            path.lineTo(toPoint(bez.list[0], gheight));
        }
        else
            path.moveTo(toPoint(bez.list[0], gheight));

        //Loop over the curve points
        for(int i=1; i<bez.size; i+=3)
            path.cubicTo(toPoint(bez.list[i], gheight), toPoint(bez.list[i+1], gheight), toPoint(bez.list[i+2], gheight));

        //If there is an ending point, draw a line to it
        if(bez.eflag)
            path.lineTo(toPoint(bez.ep, gheight));
    }
    return path;
}

Qt::BrushStyle QGVCore::toBrushStyle(const QString &style)
{
    if(style == "filled")
        return Qt::SolidPattern;
    return Qt::NoBrush;
}

Qt::PenStyle QGVCore::toPenStyle(const QString &style)
{
    if(style =="dashed")
        return Qt::DashLine;
    else if(style == "dotted")
        return Qt::DotLine;
    return Qt::SolidLine;
}

QColor QGVCore::toColor(const QString &color)
{
    return QColor(color);
}

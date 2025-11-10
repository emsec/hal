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
#ifndef QGVSCENE_H
#define QGVSCENE_H

#include "qgv.h"
#include <QGraphicsScene>
#include <QColor>
#include <QBrush>
#include <QHash>
#include <QString>

class QGVNode;
class QGVEdge;
class QGVSubGraph;
class QGVScene;

class QGVGraphPrivate;
class QGVGvcPrivate;

class QGVStyle
{

private:
    QGVStyle();
    static QGVStyle* inst;

public:
    enum StyleTarget {NodeStyle, EdgeStyle};
    enum StyleType {Dark, Light, Graphviz};

    static QGVStyle* instance();

    void setStyle(StyleTarget target, StyleType type);
    void changeHalStyle(StyleType type);
    StyleType getStyle(StyleTarget target) const;

    QColor penColor(bool selected, const QColor& graphvizColor) const;
    QColor gridColor() const;
    QColor selectFrameColor() const;
    QBrush nodeBrush(bool selected, const QBrush& graphvizBrush) const;
private:
    StyleType mStyleType[2];
};

class QGVInteraction : public QObject
{
    Q_OBJECT

protected:
    static QHash<QString,QGVInteraction*(*)(QGVScene*)>* sConstructorForPlugin;
    bool mDisableHandler;
public:
    QGVInteraction(QGVScene* parent);
    virtual ~QGVInteraction() {;}
    void disableHandler();

    virtual void registerNode(QGVNode* node) = 0;
    virtual void registerEdge(QGVEdge* edge) = 0;

    static void registerConstructorForPlugin(const QString& plugin, QGVInteraction*(*constructor)(QGVScene*));
    static QGVInteraction* construct(const QString& plugin, QGVScene* parent);
    static QStringList getPlugins();
};

/**
 * @brief GraphViz interactive scene
 *
 */
class QGVCORE_EXPORT QGVScene : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit QGVScene(QObject *parent = 0);
    ~QGVScene();

    void setGraphAttribute(const QString &name, const QString &value);
    void setNodeAttribute(const QString &name, const QString &value);
    void setEdgeAttribute(const QString &name, const QString &value);

    QGVNode* addNode(const QString& label);
    QGVEdge* addEdge(QGVNode* source, QGVNode* target, const QString& label=QString());
    QGVSubGraph* addSubGraph(const QString& name, bool cluster=true);

    void deleteNode(QGVNode *node);
    void deleteEdge(QGVEdge *edge);
    void deleteSubGraph(QGVSubGraph *subgraph);

    void setRootNode(QGVNode *node);
    void toggleDrawGrid();

    void loadLayout(const QString &text, QGVInteraction* interact=nullptr);
    void applyLayout();
    void clear();


Q_SIGNALS:
    void nodeContextMenu(QGVNode* node);
    void nodeDoubleClick(QGVNode* node);

    void edgeContextMenu(QGVEdge* edge);
    void edgeDoubleClick(QGVEdge* edge);

    void subGraphContextMenu(QGVSubGraph* graph);
    void subGraphDoubleClick(QGVSubGraph* graph);

    void graphContextMenuEvent();
    
public Q_SLOTS:

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);
    virtual void drawBackground(QPainter * painter, const QRectF & rect);
private:
    friend class QGVNode;
    friend class QGVEdge;
    friend class QGVSubGraph;

    QGVGvcPrivate *_context;
    QGVGraphPrivate *_graph;
    //QFont _font;

    QList<QGVNode*> _nodes;
    QList<QGVEdge*> _edges;
    QList<QGVSubGraph*> _subGraphs;

    bool _drawGrid;

    static int sCount;
};

#endif // QGVSCENE_H

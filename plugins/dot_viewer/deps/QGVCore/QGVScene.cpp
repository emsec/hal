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
#include "QGVScene.h"
#include <QDebug>

#include <QGVNode.h>
#include <QGVEdge.h>
#include <QGVSubGraph.h>

#include <QGVCore.h>
#include <QGVGraphPrivate.h>
#include <QGVGvcPrivate.h>
#include <QGVEdgePrivate.h>
#include <QGVNodePrivate.h>
#include <QGraphicsView>
#include <iostream>
#include "hal_core/utilities/log.h"

int QGVScene::sCount = 0;

extern "C" {
    extern int aaglex_destroy(void);
}

QGVInteraction::QGVInteraction(QGVScene* parent)
    : QObject(parent), mDisableHandler(false)
{;}

QHash<QString,QGVInteraction*(*)(QGVScene*)>* QGVInteraction::sConstructorForPlugin = nullptr;

QGVInteraction* QGVInteraction::construct(const QString &plugin, QGVScene* parent)
{
    if (!sConstructorForPlugin) return nullptr;
    auto it = sConstructorForPlugin->find(plugin);
    if (it == sConstructorForPlugin->end()) return nullptr;
    return (it.value())(parent);
}

void QGVInteraction::registerConstructorForPlugin(const QString& plugin, QGVInteraction*(*constructor)(QGVScene*))
{
    if (!sConstructorForPlugin) sConstructorForPlugin = new QHash<QString,QGVInteraction*(*)(QGVScene*)>();
    sConstructorForPlugin->insert(plugin, constructor);
}

QStringList QGVInteraction::getPlugins()
{
    if (!sConstructorForPlugin) return QStringList();
    return sConstructorForPlugin->keys();
}

void QGVInteraction::disableHandler()
{
    mDisableHandler = true;
}

QGVScene::QGVScene(QObject *parent)
    : QGraphicsScene(parent), _drawGrid(true)
{
    aaglex_destroy();
    _context = new QGVGvcPrivate(gvContext());
    QString name = QString("DotViewer%1").arg(++sCount);
    _graph = new QGVGraphPrivate(agopen(name.toLocal8Bit().data(), Agdirected, NULL));
    //setGraphAttribute("fontname", QFont().family());
}

QGVScene::~QGVScene()
{
    gvFreeLayout(_context->context(), _graph->graph());
    agclose(_graph->graph());
    gvFreeContext(_context->context());
    delete _graph;
    delete _context;
}

void QGVScene::setGraphAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGRAPH, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

void QGVScene::setNodeAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGNODE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

void QGVScene::setEdgeAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGEDGE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

QGVNode *QGVScene::addNode(const QString &label)
{
    Agnode_t *node = agnode(_graph->graph(), NULL, TRUE);
    if(node == NULL)
    {
        qWarning()<<"Invalid node :"<<label;
        return 0;
    }
    QGVNode *item = new QGVNode(new QGVNodePrivate(node, _graph->graph()), this);
    item->setLabel(label);
    addItem(item);
    _nodes.append(item);
    return item;
}

QGVEdge *QGVScene::addEdge(QGVNode *source, QGVNode *target, const QString &label)
{
    Agedge_t* edge = agedge(_graph->graph(), source->_node->node(), target->_node->node(), NULL, TRUE);
    if(edge == NULL)
    {
        qWarning()<<"Invalid egde :"<<label;
        return 0;
    }

    QGVEdge *item = new QGVEdge(new QGVEdgePrivate(edge), this);
    item->setLabel(label);
    addItem(item);
    _edges.append(item);
    return item;
}

QGVSubGraph *QGVScene::addSubGraph(const QString &name, bool cluster)
{
    Agraph_t* sgraph;
    if(cluster)
        sgraph = agsubg(_graph->graph(), ("cluster_" + name).toLocal8Bit().data(), TRUE);
    else
        sgraph = agsubg(_graph->graph(), name.toLocal8Bit().data(), TRUE);

    if(sgraph == NULL)
    {
        qWarning()<<"Invalid subGraph :"<<name;
        return 0;
    }

    QGVSubGraph *item = new QGVSubGraph(new QGVGraphPrivate(sgraph), this);
    addItem(item);
    _subGraphs.append(item);
    return item;
}

void QGVScene::deleteNode(QGVNode* node)
{
    QList<QGVNode *>::iterator it = std::find(_nodes.begin(), _nodes.end(), node);
    if(it == _nodes.end())
    {
        std::cout << "Error, node not part of Scene" << std::endl;
        return;
    }
    std::cout << "delNode ret " << agdelnode(node->_node->graph(), node->_node->node()) << std::endl;;
    _nodes.erase(it);
    delete node;
}

void QGVScene::deleteEdge(QGVEdge* edge)
{
    std::cout << "delEdge ret " << agdeledge(_graph->graph(), edge->_edge->edge()) << std::endl;
    QList<QGVEdge *>::iterator it = std::find(_edges.begin(), _edges.end(), edge);
    if(it == _edges.end())
    {
        std::cout << "Error, QGVEdge not part of Scene" << std::endl;
        return;
    }
    _edges.erase(it);
    delete edge;
}

void QGVScene::deleteSubGraph(QGVSubGraph *subgraph)
{
    std::cout << "Removing sug " << subgraph->_sgraph->graph() << std::endl;
    std::cout << "delSubg ret " << agclose(subgraph->_sgraph->graph()) << std::endl;
    QList<QGVSubGraph *>::iterator it = std::find(_subGraphs.begin(), _subGraphs.end(), subgraph);
    if(it == _subGraphs.end())
    {
        std::cout << "Error, QGVSubGraph not part of Scene" << std::endl;
        return;
    }
    _subGraphs.erase(it);

    delete subgraph;
}

void QGVScene::setRootNode(QGVNode *node)
{
    Q_ASSERT(_nodes.contains(node));
    char root[] = "root";
    agset(_graph->graph(), root, node->label().toLocal8Bit().data());
}

void QGVScene::loadLayout(const QString &text, QGVInteraction *interact)
{
    _graph->setGraph(QGVCore::agmemread2(text.toLocal8Bit().constData()));

    if (!_graph->graph())
    {
        const char* err = aglasterr();
        hal::log_warning("dot_viewer", "Error parsing file input '{}'.", (err ? err : "") );
        return;
    }

    if(gvLayout(_context->context(), _graph->graph(), "dot") != 0)
    {
        const char* err = aglasterr();
        hal::log_warning("dot_viewer", "Layout render error {} '{}'.",  agerrors(), (err ? err : "") );
        return;
    }

    //Debug output
		//gvRenderFilename(_context->context(), _graph->graph(), "png", "debug.png");

    QHash<void*, QGVNode*> nodeHash;
    QList<QGVEdge*> edgeList;

    //Read nodes and edges
    for (Agnode_t* node = agfstnode(_graph->graph()); node != NULL; node = agnxtnode(_graph->graph(), node))
    {
        QGVNode *inode = new QGVNode(new QGVNodePrivate(node, _graph->graph()), this);
        if (interact) interact->registerNode(inode);
        inode->updateLayout();
        addItem(inode);
        nodeHash[node] = inode;
        for (Agedge_t* edge = agfstout(_graph->graph(), node); edge != NULL; edge = agnxtout(_graph->graph(), edge))
        {
            QGVEdge *iedge = new QGVEdge(new QGVEdgePrivate(edge), this);
            if (interact) interact->registerEdge(iedge);
            iedge->updateLayout();
            addItem(iedge);
            edgeList.append(iedge);
        }
    }

    for (QGVEdge* iedge : edgeList)
        iedge->setEndpoints(nodeHash);

    update();
/*
    for (QGraphicsView* view : views()) {
        if (view) {
            view->fitInView(itemsBoundingRect(), Qt::KeepAspectRatio);
        }
    }
*/
}

void QGVScene::applyLayout()
{
    if(gvLayout(_context->context(), _graph->graph(), "dot") != 0)
    {
        /*
         * Si plantage ici :
         *  - Verifier que les dll sont dans le repertoire d'execution
         *  - Verifie que le fichier "configN" est dans le repertoire d'execution !
         */
        qCritical()<<"Layout render error"<<agerrors()<<QString::fromLocal8Bit(aglasterr());
        return;
    }

    //Debug output
		//gvRenderFilename(_context->context(), _graph->graph(), "canon", "debug.dot");
		//gvRenderFilename(_context->context(), _graph->graph(), "png", "debug.png");

    //Update items layout
    foreach(QGVNode* node, _nodes)
        node->updateLayout();

    foreach(QGVEdge* edge, _edges)
        edge->updateLayout();

    foreach(QGVSubGraph* sgraph, _subGraphs)
        sgraph->updateLayout();

    //Graph label
    textlabel_t *xlabel = GD_label(_graph->graph());
    if(xlabel)
    {
        QGraphicsTextItem *item = addText(xlabel->text);
        item->setPos(QGVCore::centerToOrigin(QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_graph->graph())), xlabel->dimen.x, -4));
    }

    gvFreeLayout(_context->context(), _graph->graph());

    update();
}

void QGVScene::clear()
{
    gvFreeLayout(_context->context(), _graph->graph());
    _nodes.clear();
    _edges.clear();
    _subGraphs.clear();
    QGraphicsScene::clear();
}

#include <QGraphicsSceneContextMenuEvent>
void QGVScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsItem *item = itemAt(contextMenuEvent->scenePos(), QTransform());
    if(item)
    {
        item->setSelected(true);
        if(item->type() == QGVNode::Type)
            emit nodeContextMenu(qgraphicsitem_cast<QGVNode*>(item));
        else if(item->type() == QGVEdge::Type)
            emit edgeContextMenu(qgraphicsitem_cast<QGVEdge*>(item));
        else if(item->type() == QGVSubGraph::Type)
            emit subGraphContextMenu(qgraphicsitem_cast<QGVSubGraph*>(item));
        else
            emit graphContextMenuEvent();
    }
    QGraphicsScene::contextMenuEvent(contextMenuEvent);
}

void QGVScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    if(item)
    {
        if(item->type() == QGVNode::Type)
            emit nodeDoubleClick(qgraphicsitem_cast<QGVNode*>(item));
        else if(item->type() == QGVEdge::Type)
            emit edgeDoubleClick(qgraphicsitem_cast<QGVEdge*>(item));
        else if(item->type() == QGVSubGraph::Type)
            emit subGraphDoubleClick(qgraphicsitem_cast<QGVSubGraph*>(item));
    }
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
}

void QGVScene::toggleDrawGrid()
{
    _drawGrid = !_drawGrid;
    update();
}

#include <QVarLengthArray>
#include <QPainter>
void QGVScene::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (_drawGrid)
    {
        const int gridSize = 25;

        qreal x = int(rect.left()) - (int(rect.left()) % gridSize);
        qreal y = int(rect.top()) - (int(rect.top()) % gridSize);

        QVarLengthArray<QLineF, 100> lines;

        while (x < rect.right())
        {
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
            x += gridSize;
        }

        while (y < rect.bottom())
        {
            lines.append(QLineF(rect.left(), y, rect.right(), y));
            y += gridSize;
        }

        painter->setRenderHint(QPainter::Antialiasing, false);

        painter->setPen(QGVStyle::instance()->gridColor());
        painter->drawLines(lines.data(), lines.size());
    }

    painter->setPen(Qt::lightGray);
    //painter->drawRect(sceneRect());
}

///------------------------
QGVStyle* QGVStyle::inst = nullptr;

QGVStyle::QGVStyle() : mStyleType(Dark) {;}

QGVStyle* QGVStyle::instance()
{
    if (!inst) inst = new QGVStyle;
    return inst;
}

void QGVStyle::setStyle(StyleType type)
{
    mStyleType = type;
}

QColor QGVStyle::penColor(bool selected, const QColor& graphvizColor) const
{
    switch (mStyleType) {
        case Dark:
            if (selected) return QColor(Qt::cyan);
            return QColor::fromRgb(200,200,200);
        case Light:
            if (selected) return QColor(Qt::blue);
            return QColor(Qt::black);
        case Graphviz:
            if (selected) return graphvizColor.darker(120);
            return graphvizColor;
    }
    return QColor();
};

QColor QGVStyle::gridColor() const
{
    switch (mStyleType) {
        case Dark:
            return QColor(Qt::black);
        case Light:
            return QColor::fromRgb(220,221,223);
        case Graphviz:
            return QColor(Qt::lightGray);
    }
    return QColor();
}


QBrush QGVStyle::nodeBrush(bool selected, const QBrush& graphvizBrush) const
{
    switch (mStyleType) {
        case Dark:
            if (selected) return QBrush(Qt::white);
            return QBrush(QColor::fromRgb(160,161,164));
        case Light:
            if (selected) return QBrush(QColor::fromRgb(220,221,223));
            return QBrush(Qt::white);
        case Graphviz:
            if (selected) return QBrush(graphvizBrush.color().darker(120));
            return graphvizBrush;
    }
    return QBrush();
}

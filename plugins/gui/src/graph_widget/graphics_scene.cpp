#include "gui/graph_widget/graphics_scene.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include "gui/graph_widget/items/nets/graphics_net.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>

#include <QDebug>

namespace hal
{
    qreal GraphicsScene::sLod = 0;

    const qreal GraphicsScene::sGridFadeStart = 0.4;
    const qreal GraphicsScene::sGridFadeEnd = 1.0;

    bool GraphicsScene::sGridEnabled = true;
    bool GraphicsScene::sGridClustersEnabled = true;
    graph_widget_constants::grid_type GraphicsScene::sGridType = graph_widget_constants::grid_type::Lines;

    QColor GraphicsScene::sGridBaseLineColor = QColor(30, 30, 30);
    QColor GraphicsScene::sGridClusterLineColor = QColor(15, 15, 15);

    QColor GraphicsScene::sGridBaseDotColor = QColor(25, 25, 25);
    QColor GraphicsScene::sGridClusterDotColor = QColor(170, 160, 125);

    void GraphicsScene::setLod(const qreal& lod)
    {
        sLod = lod;

        if (lod >= sGridFadeStart && lod <= sGridFadeEnd)
        {
            const qreal alpha = (lod - sGridFadeStart) / (sGridFadeEnd - sGridFadeStart);

            sGridBaseLineColor.setAlphaF(alpha);
            sGridClusterLineColor.setAlphaF(alpha);

            sGridBaseDotColor.setAlphaF(alpha);
            sGridClusterDotColor.setAlphaF(alpha);
        }
        else
        {
            const int alpha = 255;

            sGridBaseLineColor.setAlpha(alpha);
            sGridClusterLineColor.setAlpha(alpha);

            sGridBaseDotColor.setAlpha(alpha);
            sGridClusterDotColor.setAlpha(alpha);
        }
    }

    void GraphicsScene::setGridEnabled(const bool& value)
    {
        sGridEnabled = value;
    }

    void GraphicsScene::setGridClustersEnabled(const bool& value)
    {
        sGridClustersEnabled = value;
    }

    void GraphicsScene::setGridType(const graph_widget_constants::grid_type& grid_type)
    {
        sGridType = grid_type;
    }

    void GraphicsScene::setGridBaseLineColor(const QColor& color)
    {
        sGridBaseLineColor = color;
    }

    void GraphicsScene::setGridClusterLineColor(const QColor& color)
    {
        sGridClusterLineColor = color;
    }

    void GraphicsScene::setGridBaseDotColor(const QColor& color)
    {
        sGridBaseDotColor = color;
    }

    void GraphicsScene::setGridClusterDotColor(const QColor& color)
    {
        sGridClusterDotColor = color;
    }

    QPointF GraphicsScene::snapToGrid(const QPointF& pos)
    {
        int adjusted_x = qRound(pos.x() / graph_widget_constants::sGridSize) * graph_widget_constants::sGridSize;
        int adjusted_y = qRound(pos.y() / graph_widget_constants::sGridSize) * graph_widget_constants::sGridSize;
        return QPoint(adjusted_x, adjusted_y);
    }

    GraphicsScene::GraphicsScene(QObject* parent) : QGraphicsScene(parent),
        mDragShadowGate(new NodeDragShadow())
    {
        // FIND OUT IF MANUAL CHANGE TO DEPTH IS NECESSARY / INCREASES PERFORMANCE
        //mScene.setBspTreeDepth(10);


        gSelectionRelay->registerSender(this, "GraphView");
        connectAll();

        QGraphicsScene::addItem(mDragShadowGate);

        #ifdef GUI_DEBUG_GRID
        mDebugGridEnable = gSettingsManager->get("debug/grid").toBool();
        #endif
    }

    void GraphicsScene::startDragShadow(const QPointF& posF, const QSizeF& sizeF, const NodeDragShadow::DragCue cue)
    {
        mDragShadowGate->setVisualCue(cue);
        mDragShadowGate->start(posF, sizeF);
    }

    void GraphicsScene::moveDragShadow(const QPointF& posF, const NodeDragShadow::DragCue cue)
    {
        mDragShadowGate->setPos(posF);
        mDragShadowGate->setVisualCue(cue);
    }

    void GraphicsScene::stopDragShadow()
    {
        mDragShadowGate->stop();
    }

    QPointF GraphicsScene::dropTarget()
    {
        return mDragShadowGate->pos();
    }

    void GraphicsScene::addGraphItem(GraphicsItem* item)
    {
        // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

        if (!item)
            return;

        QGraphicsScene::addItem(item);

        switch (item->itemType())
        {
        case ItemType::Gate:
        {
            GraphicsGate* g = static_cast<GraphicsGate*>(item);
            int i = 0;
            while (i < mGateItems.size())
            {
                if (g->id() < mGateItems.at(i).mId)
                    break;

                i++;
            }
            mGateItems.insert(i, GateData{g->id(), g});
            return;
        }
        case ItemType::Net:
        {
            GraphicsNet* n = static_cast<GraphicsNet*>(item);
            int i = 0;
            while (i < mNetItems.size())
            {
                if (n->id() < mNetItems.at(i).mId)
                    break;

                i++;
            }
            mNetItems.insert(i, NetData{n->id(), n});
            return;
        }
        case ItemType::Module:
        {
            GraphicsModule* m = static_cast<GraphicsModule*>(item);
            int i = 0;
            while (i < mModuleItems.size())
            {
                if (m->id() < mModuleItems.at(i).mId)
                    break;

                i++;
            }
            mModuleItems.insert(i, ModuleData{m->id(), m});
            return;
        }
        default:
            return;
        }
    }

    void GraphicsScene::removeGraphItem(GraphicsItem* item)
    {
        // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

        if (!item)
            return;

        QGraphicsScene::removeItem(item);

        switch (item->itemType())
        {
        case ItemType::Gate:
        {
            GraphicsGate* g = static_cast<GraphicsGate*>(item);
            u32 id = g->id();

            int i = 0;
            while (i < mGateItems.size())
            {
                if (mGateItems[i].mId == id)
                {
                    mGateItems.remove(i);
                    delete g;
                    return;
                }

                ++i;
            }

            return;
        }
        case ItemType::Net:
        {
            GraphicsNet* n = static_cast<GraphicsNet*>(item);
            u32 id = n->id();

            int i = 0;
            while (i < mNetItems.size())
            {
                if (mNetItems[i].mId == id)
                {
                    mNetItems.remove(i);
                    delete n;
                    return;
                }

                ++i;
            }

            return;
        }
        case ItemType::Module:
        {
            GraphicsModule* m = static_cast<GraphicsModule*>(item);
            u32 id = m->id();

            int i = 0;
            while (i < mModuleItems.size())
            {
                if (mModuleItems[i].mId == id)
                {
                    mModuleItems.remove(i);
                    delete m;
                    return;
                }

                ++i;
            }

            return;
        }
        default:
            return;
        }
    }

    const GraphicsGate* GraphicsScene::getGateItem(const u32 id) const
    {
        for (const GateData& d : mGateItems)
        {
            if (d.mId > id)
                break;

            if (d.mId == id)
                return d.mItem;
        }

        return nullptr;
    }

    const GraphicsNet* GraphicsScene::getNetItem(const u32 id) const
    {
        for (const NetData& d : mNetItems)
        {
            if (d.mId > id)
                break;

            if (d.mId == id)
                return d.mItem;
        }

        return nullptr;
    }

    const GraphicsModule* GraphicsScene::getModuleItem(const u32 id) const
    {
        for (const ModuleData& d : mModuleItems)
        {
            if (d.mId > id)
                break;

            if (d.mId == id)
                return d.mItem;
        }

        return nullptr;
    }

    void GraphicsScene::connectAll()
    {
        connect(gSettingsRelay, &SettingsRelay::settingChanged, this, &GraphicsScene::handleGlobalSettingChanged);

        connect(this, &GraphicsScene::selectionChanged, this, &GraphicsScene::handleInternSelectionChanged);

        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &GraphicsScene::handleExternSelectionChanged);
        connect(gSelectionRelay, &SelectionRelay::subfocusChanged, this, &GraphicsScene::handleExternSubfocusChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleAssigned, this, &GraphicsScene::handleGroupingAssignModule);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleRemoved, this, &GraphicsScene::handleGroupingAssignModule);
        connect(gNetlistRelay, &NetlistRelay::groupingGateAssigned, this, &GraphicsScene::handleGroupingAssignGate);
        connect(gNetlistRelay, &NetlistRelay::groupingGateRemoved, this, &GraphicsScene::handleGroupingAssignGate);
        connect(gNetlistRelay, &NetlistRelay::groupingNetAssigned, this, &GraphicsScene::handleGroupingAssignNet);
        connect(gNetlistRelay, &NetlistRelay::groupingNetRemoved, this, &GraphicsScene::handleGroupingAssignNet);
        connect(gContentManager->getGroupingManagerWidget()->getModel(),&GroupingTableModel::groupingColorChanged,
                this, &GraphicsScene::handleGroupingColorChanged);
    }

    void GraphicsScene::disconnectAll()
    {
        disconnect(gSettingsRelay, &SettingsRelay::settingChanged, this, &GraphicsScene::handleGlobalSettingChanged);

        disconnect(this, &GraphicsScene::selectionChanged, this, &GraphicsScene::handleInternSelectionChanged);

        disconnect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &GraphicsScene::handleExternSelectionChanged);
        disconnect(gSelectionRelay, &SelectionRelay::subfocusChanged, this, &GraphicsScene::handleExternSubfocusChanged);
        disconnect(gNetlistRelay, &NetlistRelay::groupingModuleAssigned, this, &GraphicsScene::handleGroupingAssignModule);
        disconnect(gNetlistRelay, &NetlistRelay::groupingModuleRemoved, this, &GraphicsScene::handleGroupingAssignModule);
        disconnect(gNetlistRelay, &NetlistRelay::groupingGateAssigned, this, &GraphicsScene::handleGroupingAssignGate);
        disconnect(gNetlistRelay, &NetlistRelay::groupingGateRemoved, this, &GraphicsScene::handleGroupingAssignGate);
        disconnect(gNetlistRelay, &NetlistRelay::groupingNetAssigned, this, &GraphicsScene::handleGroupingAssignNet);
        disconnect(gNetlistRelay, &NetlistRelay::groupingNetRemoved, this, &GraphicsScene::handleGroupingAssignNet);
        disconnect(gContentManager->getGroupingManagerWidget()->getModel(),&GroupingTableModel::groupingColorChanged,
                   this, &GraphicsScene::handleGroupingColorChanged);
    }

    void GraphicsScene::deleteAllItems()
    {
        // this breaks the mDragShadowGate
        // clear();
        // so we do this instead
        // TODO check performance hit
        for (auto item : items())
        {
            if (item != mDragShadowGate)
            {
                removeItem(item);
            }
        }

        mModuleItems.clear();
        mGateItems.clear();
        mNetItems.clear();
    }

    void GraphicsScene::updateVisuals(const GraphShader::Shading &s)
    {
        for (ModuleData& m : mModuleItems)
        {
            m.mItem->setVisuals(s.mOduleVisuals.value(m.mId));
        }

        for (GateData& g : mGateItems)
        {
            g.mItem->setVisuals(s.mGateVisuals.value(g.mId));
        }

        for (NetData& n : mNetItems)
        {
            n.mItem->setVisuals(s.mNetVisuals.value(n.mId));
        }
    }

    void GraphicsScene::moveNetsToBackground()
    {
        for (NetData d : mNetItems)
            d.mItem->setZValue(-1);
    }

    void GraphicsScene::handleInternSelectionChanged()
    {
        gSelectionRelay->clear();

        int gates = 0;
        int nets = 0;
        int modules = 0;

        for (const QGraphicsItem* item : selectedItems())
        {
            switch (static_cast<const GraphicsItem*>(item)->itemType())
            {
            case ItemType::Gate:
            {
                gSelectionRelay->mSelectedGates.insert(static_cast<const GraphicsItem*>(item)->id());
                ++gates;
                break;
            }
            case ItemType::Net:
            {
                gSelectionRelay->mSelectedNets.insert(static_cast<const GraphicsItem*>(item)->id());
                ++nets;
                break;
            }
            case ItemType::Module:
            {
                gSelectionRelay->mSelectedModules.insert(static_cast<const GraphicsItem*>(item)->id());
                ++modules;
                break;
            }
            default:
                break;
            }
        }

        // TEST CODE
        // ADD FOCUS DEDUCTION INTO RELAY ???
        if (gates + nets + modules == 1)
        {
            if (gates)
            {
                gSelectionRelay->mFocusType = SelectionRelay::ItemType::Gate;
                gSelectionRelay->mFocusId = *gSelectionRelay->mSelectedGates.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
            else if (nets)
            {
                gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                gSelectionRelay->mFocusId = *gSelectionRelay->mSelectedNets.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
            else
            {
                gSelectionRelay->mFocusType = SelectionRelay::ItemType::Module;
                gSelectionRelay->mFocusId = *gSelectionRelay->mSelectedModules.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
        }
        else
        {
            gSelectionRelay->mFocusType = SelectionRelay::ItemType::None;
        }
        gSelectionRelay->mSubfocus = SelectionRelay::Subfocus::None;
        // END OF TEST CODE

        //LOG MANUAL SELECTION CHANGED:
        //log_info("gui", "Selection changed through manual interaction with a view to: insert here..");
        gSelectionRelay->relaySelectionChanged(this);

    }

    void GraphicsScene::handleGroupingAssignModule(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsModule* gm = (GraphicsModule*) getModuleItem(id);
        if (gm) gm->update();
    }

    void GraphicsScene::handleGroupingAssignGate(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsGate* gg = (GraphicsGate*) getGateItem(id);
        if (gg) gg->update();
    }

    void GraphicsScene::handleGroupingAssignNet(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsNet* gn = (GraphicsNet*) getNetItem(id);
        if (gn) gn->update();
    }

    void GraphicsScene::handleGroupingColorChanged(Grouping *grp)
    {
        for (const ModuleData& md : mModuleItems)
            if (grp->contains_module_by_id(md.mId))
                md.mItem->update();
        for (const GateData& gd : mGateItems)
            if (grp->contains_gate_by_id(gd.mId))
                gd.mItem->update();
        for (const NetData& nd : mNetItems)
            if (grp->contains_net_by_id(nd.mId))
                nd.mItem->update();
    }

    void GraphicsScene::handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems)
    {
        QSet<u32> highlightSet[SelectionTreeItem::MaxItem];
        for (const SelectionTreeItem* sti : highlightItems)
        {
            if (sti) highlightSet[sti->itemType()].insert(sti->id());
        }

        for (const ModuleData& mdata :  mModuleItems)
            mdata.mItem->setHightlight(highlightSet[SelectionTreeItem::ModuleItem].contains(mdata.mId));
        for (const GateData& gdata :  mGateItems)
            gdata.mItem->setHightlight(highlightSet[SelectionTreeItem::GateItem].contains(gdata.mId));
        for (const NetData& ndata :  mNetItems)
            ndata.mItem->setHightlight(highlightSet[SelectionTreeItem::NetItem].contains(ndata.mId));
    }

    void GraphicsScene::handleExternSelectionChanged(void* sender)
    {
        // CLEAR CURRENT SELECTION EITHER MANUALLY OR USING clearSelection()
        // UNCERTAIN ABOUT THE SENDER PARAMETER

        if (sender == this)
            return;

        bool original_value = blockSignals(true);

        clearSelection();

        if (!gSelectionRelay->mSelectedModules.isEmpty())
        {
            for (auto& element : mModuleItems)
            {
                if (gSelectionRelay->isModuleSelected(element.mId))
                {
                    element.mItem->setSelected(true);
                    element.mItem->update();
                }
            }
        }

        if (!gSelectionRelay->mSelectedGates.isEmpty())
        {
            for (auto& element : mGateItems)
            {
                if (gSelectionRelay->isGateSelected(element.mId))
                {
                    element.mItem->setSelected(true);
                    element.mItem->update();
                }
            }
        }

        if (!gSelectionRelay->mSelectedNets.isEmpty())
        {
            for (auto& element : mNetItems)
            {
                if (gSelectionRelay->isNetSelected(element.mId))
                {
                    element.mItem->setSelected(true);
                    element.mItem->update();
                }
            }
        }

        blockSignals(original_value);
    }

    void GraphicsScene::handleExternSubfocusChanged(void* sender)
    {
        Q_UNUSED(sender)
    }

    void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
    {
        // CONTEXT MENU CLEARING SELECTION WORKAROUND
        if (event->button() == Qt::RightButton)
        {
            event->accept();
            return;
        }

        QGraphicsScene::mousePressEvent(event);
    }

    void GraphicsScene::handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender)

        #ifdef GUI_DEBUG_GRID
        if (key == "debug/grid")
        {
            mDebugGridEnable = value.toBool();
        }
        #endif
    }

    void GraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
    {
        if (!sGridEnabled)
            return;

        if (sLod < sGridFadeStart)
            return;

        QFlags original_flags = painter->renderHints(); // UNNECESSARY ?
        painter->setRenderHint(QPainter::Antialiasing, true);

        QPen pen;
        pen.setWidth(2);

        // OVERDRAW NEEDED BECAUSE QT FAILS AT BASIC geometry
        const int overdraw = 2;

        const int x_from = rect.left() - overdraw;
        const int x_to = rect.right() + overdraw;

        const int y_from = rect.top() - overdraw;
        const int y_to = rect.bottom() + overdraw;

        const int xOffset = x_from % graph_widget_constants::sGridSize;
        const int yOffset = y_from % graph_widget_constants::sGridSize;

        switch (sGridType)
        {
        case graph_widget_constants::grid_type::None:
            return; // nothing to do
        case graph_widget_constants::grid_type::Lines:
        {
            QVarLengthArray<QLine, 512> base_lines;
            QVarLengthArray<QLine, 64> cluster_lines;

            for (int x = x_from - xOffset; x < x_to; x += graph_widget_constants::sGridSize)
            {
                QLine line(x, y_from, x, y_to);
                if (x % (graph_widget_constants::sGridSize * graph_widget_constants::sClusterSize))
                    base_lines.append(line);
                else
                    cluster_lines.append(line);
            }

            for (int y = y_from - yOffset; y < y_to; y += graph_widget_constants::sGridSize)
            {
                QLine line(x_from, y, x_to, y);
                if (y % (graph_widget_constants::sGridSize * graph_widget_constants::sClusterSize))
                    base_lines.append(line);
                else
                    cluster_lines.append(line);
            }

            pen.setColor(sGridBaseLineColor);
            painter->setPen(pen);

            painter->drawLines(base_lines.data(), base_lines.size());

            if (sGridClustersEnabled)
            {
                pen.setColor(sGridClusterLineColor);
                painter->setPen(pen);
            }

            painter->drawLines(cluster_lines.data(), cluster_lines.size());
            break;
        }

        case graph_widget_constants::grid_type::Dots:
        {
            QVector<QPoint> base_points;
            QVector<QPoint> cluster_points;

            for (int x = x_from - xOffset; x < x_to; x += graph_widget_constants::sGridSize)
                for (int y = y_from - yOffset; y < y_to; y += graph_widget_constants::sGridSize)
                {
                    if ((x % (graph_widget_constants::sGridSize * graph_widget_constants::sClusterSize)) && (y % (graph_widget_constants::sGridSize * graph_widget_constants::sClusterSize)))
                        base_points.append(QPoint(x,y));
                    else
                        cluster_points.append(QPoint(x,y));
                }

            pen.setColor(sGridBaseDotColor);
            painter->setPen(pen);

            painter->drawPoints(base_points.data(), base_points.size());

            if (sGridClustersEnabled)
            {
                pen.setColor(sGridClusterDotColor);
                painter->setPen(pen);
            }

            painter->drawPoints(cluster_points.data(), cluster_points.size());
            break;
        }
        }

        #ifdef GUI_DEBUG_GRID
        if (mDebugGridEnable)
            debugDrawLayouterGrid(painter, x_from, x_to, y_from, y_to);
        #endif

        painter->setRenderHints(original_flags); // UNNECESSARY ?
    }

    #ifdef GUI_DEBUG_GRID
    void GraphicsScene::debugSetLayouterGrid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width)
    {
        mDebugXLines = debug_x_lines;
        mDebugYLines = debug_y_lines;
        mDebugDefaultHeight = debug_default_height;
        mDebugDefaultWidth = debug_default_width;
    }

    void GraphicsScene::debugDrawLayouterGrid(QPainter* painter, const int x_from, const int x_to, const int y_from, const int y_to)
    {
        painter->setPen(QPen(Qt::magenta));

        for (qreal x : mDebugXLines)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
        }

        for (qreal y : mDebugYLines)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
        }

        painter->setPen(QPen(Qt::green));

        qreal x = mDebugXLines.last() + mDebugDefaultWidth;

        while (x <= x_to)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
            x += mDebugDefaultWidth;
        }

        x = mDebugXLines.first() - mDebugDefaultWidth;

        while (x >= x_from)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
            x -= mDebugDefaultWidth;
        }

        qreal y = mDebugYLines.last() + mDebugDefaultHeight;

        while (y <= y_to)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
            y += mDebugDefaultHeight;
        }

        y = mDebugYLines.first() - mDebugDefaultHeight;

        while (y >= y_from)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
            y -= mDebugDefaultHeight;
        }
    }
    #endif
}

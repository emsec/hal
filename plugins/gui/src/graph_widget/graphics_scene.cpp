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
#include "gui/settings/settings_items/settings_item_checkbox.h"

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
    GraphicsScene::GridType GraphicsScene::sGridType = GraphicsScene::GridType::Dots;

    void GraphicsScene::setLod(const qreal& lod)
    {
        sLod = lod;

        if (lod >= sGridFadeStart && lod <= sGridFadeEnd)
        {
            const qreal alpha = (lod - sGridFadeStart) / (sGridFadeEnd - sGridFadeStart);

            GraphicsQssAdapter::instance()->setGridAlphaF(alpha);
        }
        else
        {
            const int alpha = 255;

            GraphicsQssAdapter::instance()->setGridAlpha(alpha);
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

    void GraphicsScene::setGridType(const GraphicsScene::GridType& gridType)
    {
        sGridType = gridType;
    }

    QPointF GraphicsScene::snapToGrid(const QPointF& pos)
    {
        int adjusted_x = qRound(pos.x() / graph_widget_constants::sGridSize) * graph_widget_constants::sGridSize;
        int adjusted_y = qRound(pos.y() / graph_widget_constants::sGridSize) * graph_widget_constants::sGridSize;
        return QPoint(adjusted_x, adjusted_y);
    }

    GraphicsScene::GraphicsScene(QObject* parent) : QGraphicsScene(parent),
        mDragShadowGate(new NodeDragShadow()), mDebugGridEnable(false),
        mSelectionStatus(NotPressed)
    {
        // FIND OUT IF MANUAL CHANGE TO DEPTH IS NECESSARY / INCREASES PERFORMANCE
        //mScene.setBspTreeDepth(10);


        gSelectionRelay->registerSender(this, "GraphView");
        connectAll();

        QGraphicsScene::addItem(mDragShadowGate);
        connect(gGraphContextManager->sSettingNetGroupingToPins,&SettingsItem::valueChanged,this,&GraphicsScene::updateAllItems);
    }

    GraphicsScene::~GraphicsScene()
    {
        disconnect(this, &QGraphicsScene::selectionChanged, this, &GraphicsScene::handleInternSelectionChanged);
        for (QGraphicsItem* gi : items())
        {
            removeItem(gi);
            delete gi;
        }
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
                if (g->id() < mGateItems.at(i)->id())
                    break;

                i++;
            }
            mGateItems.insert(i, g);
            return;
        }
        case ItemType::Net:
        {
            GraphicsNet* n = static_cast<GraphicsNet*>(item);
            int i = 0;
            while (i < mNetItems.size())
            {
                if (n->id() < mNetItems.at(i)->id())
                    break;

                i++;
            }
            mNetItems.insert(i, n);
            return;
        }
        case ItemType::Module:
        {
            GraphicsModule* m = static_cast<GraphicsModule*>(item);
            int i = 0;
            while (i < mModuleItems.size())
            {
                if (m->id() < mModuleItems.at(i)->id())
                    break;

                i++;
            }
            mModuleItems.insert(i, m);
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
                if (mGateItems.at(i)->id() == id)
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
                if (mNetItems.at(i)->id() == id)
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
                if (mModuleItems.at(i)->id() == id)
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
        for (const GraphicsGate* gg : mGateItems)
        {
            if (gg->id() > id)
                break;

            if (gg->id() == id)
                return gg;
        }

        return nullptr;
    }

    const GraphicsNet* GraphicsScene::getNetItem(const u32 id) const
    {
        for (const GraphicsNet* gn : mNetItems)
        {
            if (gn->id() > id)
                break;

            if (gn->id() == id)
                return gn;
        }

        return nullptr;
    }

    const GraphicsModule* GraphicsScene::getModuleItem(const u32 id) const
    {
        for (const GraphicsModule* gm : mModuleItems)
        {
            if (gm->id() > id)
                break;

            if (gm->id() == id)
                return gm;
        }

        return nullptr;
    }

    void GraphicsScene::connectAll()
    {
        connect(this, &QGraphicsScene::selectionChanged, this, &GraphicsScene::handleInternSelectionChanged);

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
        disconnect(this, &QGraphicsScene::selectionChanged, this, &GraphicsScene::handleInternSelectionChanged);

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
        GraphicsQssAdapter::instance()->repolish();
    }

    void GraphicsScene::updateVisuals(const GraphShader::Shading &s)
    {
        for (GraphicsModule* gm : mModuleItems)
        {
            gm->setVisuals(s.mOduleVisuals.value(gm->id()));
        }

        for (GraphicsGate* gg : mGateItems)
        {
            gg->setVisuals(s.mGateVisuals.value(gg->id()));
        }

        for (GraphicsNet* gn : mNetItems)
        {
            gn->setVisuals(s.mNetVisuals.value(gn->id()));
        }
    }

    void GraphicsScene::moveNetsToBackground()
    {
        for (GraphicsNet* gn : mNetItems)
            gn->setZValue(-1);
    }

    void GraphicsScene::setMousePressed(bool isPressed)
    {
        if (isPressed)
            mSelectionStatus = BeginPressed;
        else
        {
            // not pressed ...
            if (mSelectionStatus == SelectionChanged)
            {
                mSelectionStatus = EndPressed;
                handleInternSelectionChanged();
            }
            mSelectionStatus = NotPressed;
        }
    }

    void GraphicsScene::handleInternSelectionChanged()
    {
        switch (mSelectionStatus)
        {
        case SelectionChanged:
            return;
        case BeginPressed:
            mSelectionStatus = SelectionChanged;
            gSelectionRelay->clear();
            gSelectionRelay->relaySelectionChanged(this);
            return;
        default:
            // no mouse pressed (single click) or mouse released
            break;
        }

        gSelectionRelay->clear();

        QSet<u32> mods;
        QSet<u32> gats;
        QSet<u32> nets;

        for (const QGraphicsItem* item : selectedItems())
        {
            switch (static_cast<const GraphicsItem*>(item)->itemType())
            {
            case ItemType::Gate:
            {
                gats.insert(static_cast<const GraphicsItem*>(item)->id());
                break;
            }
            case ItemType::Net:
            {
                nets.insert(static_cast<const GraphicsItem*>(item)->id());
                break;
            }
            case ItemType::Module:
            {
                mods.insert(static_cast<const GraphicsItem*>(item)->id());
                break;
            }
            default:
                break;
            }
        }

        // TEST CODE
        // ADD FOCUS DEDUCTION INTO RELAY ???
        if (gats.size() + nets.size() + mods.size() == 1)
        {
            if (!gats.isEmpty())
            {
                gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,*gats.begin());
            }
            else if (!nets.isEmpty())
            {
                gSelectionRelay->setFocus(SelectionRelay::ItemType::Net,*nets.begin());
            }
            else
            {
                gSelectionRelay->setFocus(SelectionRelay::ItemType::Module,*mods.begin());
            }
        }
        else
        {
            gSelectionRelay->setFocus(SelectionRelay::ItemType::None,0);
        }
        // END OF TEST CODE

        //LOG MANUAL SELECTION CHANGED:
        //log_info("gui", "Selection changed through manual interaction with a view to: insert here..");

        gSelectionRelay->setSelectedModules(mods);
        gSelectionRelay->setSelectedGates(gats);
        gSelectionRelay->setSelectedNets(nets);
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
        Q_UNUSED(grp);

        updateAllItems();
    }

    void GraphicsScene::updateAllItems()
    {
        for (GraphicsModule* gm : mModuleItems)
            gm->update();
        for (GraphicsGate* gg : mGateItems)
            gg->update();
        for (GraphicsNet* gn : mNetItems)
            gn->update();
    }

    void GraphicsScene::handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems)
    {
        QSet<u32> highlightSet[SelectionTreeItem::MaxItem];
        for (const SelectionTreeItem* sti : highlightItems)
        {
            if (sti) highlightSet[sti->itemType()].insert(sti->id());
        }

        for (GraphicsModule* gm :  mModuleItems)
            gm->setHightlight(highlightSet[SelectionTreeItem::ModuleItem].contains(gm->id()));
        for (GraphicsGate* gg :  mGateItems)
            gg->setHightlight(highlightSet[SelectionTreeItem::GateItem].contains(gg->id()));
        for (GraphicsNet* gn :  mNetItems)
            gn->setHightlight(highlightSet[SelectionTreeItem::NetItem].contains(gn->id()));
    }

    void GraphicsScene::handleExternSelectionChanged(void* sender)
    {
        // CLEAR CURRENT SELECTION EITHER MANUALLY OR USING clearSelection()
        // UNCERTAIN ABOUT THE SENDER PARAMETER

        if (sender == this)
            return;

        bool original_value = blockSignals(true);

        clearSelection();

        if (gSelectionRelay->numberSelectedModules())
        {
            for (GraphicsModule* gm : mModuleItems)
            {
                if (gSelectionRelay->isModuleSelected(gm->id()))
                {
                    gm->setSelected(true);
                    gm->update();
                }
            }
        }

        if (gSelectionRelay->numberSelectedGates())
        {
            for (GraphicsGate* gg : mGateItems)
            {
                if (gSelectionRelay->isGateSelected(gg->id()))
                {
                    gg->setSelected(true);
                    gg->update();
                }
            }
        }

        if (gSelectionRelay->numberSelectedNets())
        {
            for (GraphicsNet* gn : mNetItems)
            {
                if (gSelectionRelay->isNetSelected(gn->id()))
                {
                    gn->setSelected(true);
                    gn->update();
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
        case GraphicsScene::GridType::None:
            break;//return; // nothing to do //indirectly disabled the debug grid if activated
        case GraphicsScene::GridType::Lines:
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

            pen.setColor(GraphicsQssAdapter::instance()->gridBaseLineColor());
            painter->setPen(pen);

            painter->drawLines(base_lines.data(), base_lines.size());

            if (sGridClustersEnabled)
            {
                pen.setColor(GraphicsQssAdapter::instance()->gridClusterLineColor());
                painter->setPen(pen);
            }

            painter->drawLines(cluster_lines.data(), cluster_lines.size());
            break;
        }

        case GraphicsScene::GridType::Dots:
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

            pen.setColor(GraphicsQssAdapter::instance()->gridBaseDotColor());
            painter->setPen(pen);

            painter->drawPoints(base_points.data(), base_points.size());

            if (sGridClustersEnabled)
            {
                pen.setColor(GraphicsQssAdapter::instance()->gridClusterDotColor());
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
        if (mDebugXLines.isEmpty() || mDebugYLines.isEmpty()) return;
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

    void GraphicsScene::setDebugGridEnabled(bool enabled)
    {
        mDebugGridEnable = enabled;
    }

    bool GraphicsScene::debugGridEnabled()
    {
        return mDebugGridEnable;
    }
    #endif
}

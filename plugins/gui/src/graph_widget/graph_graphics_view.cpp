#include "gui/graph_widget/graph_graphics_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nets/separated_graphics_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QDrag>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QWidgetAction>
#include <algorithm>
#include <qmath.h>


namespace hal
{
    const QString GraphGraphicsView::sAssignToGrouping("Assign to grouping ");

    GraphGraphicsView::GraphGraphicsView(GraphWidget* parent)
        : QGraphicsView(parent), mGraphWidget(parent), mMinimapEnabled(false), mGridEnabled(true), mGridClustersEnabled(true), mGridType(graph_widget_constants::grid_type::Lines),
          mZoomModifier(Qt::NoModifier), mZoomFactorBase(1.0015)
    {
        connect(gSelectionRelay, &SelectionRelay::subfocusChanged, this, &GraphGraphicsView::conditionalUpdate);
        connect(this, &GraphGraphicsView::customContextMenuRequested, this, &GraphGraphicsView::showContextMenu);
        connect(gSettingsRelay, &SettingsRelay::settingChanged, this, &GraphGraphicsView::handleGlobalSettingChanged);

        initializeSettings();

        setContextMenuPolicy(Qt::CustomContextMenu);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

        setAcceptDrops(true);
        setMouseTracking(true);
    }

    void GraphGraphicsView::initializeSettings()
    {
        unsigned int drag_modifier_setting = gSettingsManager->get("graph_view/drag_mode_modifier").toUInt();
        mDragModifier                    = Qt::KeyboardModifier(drag_modifier_setting);
        unsigned int move_modifier_setting = gSettingsManager->get("graph_view/move_modifier").toUInt();
        mMoveModifier                    = Qt::KeyboardModifier(move_modifier_setting);

        // might think about Q_ENUM to avoid separate enum and config file tokens
        const char* gridTypeNames[] = {"lines", "dots", "none", 0};

        QString sGridType = gSettingsManager->get("graph_view/grid_type").toString();
        for (int i = 0; gridTypeNames[i]; i++)
            if (sGridType == gridTypeNames[i])
            {
                mGridType = static_cast<graph_widget_constants::grid_type>(i);
                break;
            }

#ifdef GUI_DEBUG_GRID
        mDebugGridposEnable = gSettingsManager->get("debug/grid").toBool();
#endif
    }

    void GraphGraphicsView::conditionalUpdate()
    {
        if (QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()) >= graph_widget_constants::sGateMinLod)
            update();
    }

    void GraphGraphicsView::handleChangeColorAction()
    {
        QColor color = QColorDialog::getColor();

        if (!color.isValid())
            return;
    }

    void GraphGraphicsView::handleIsolationViewAction()
    {
        u32 cnt = 0;
        while (true)
        {
            ++cnt;
            QString name = "Isolated View " + QString::number(cnt);
            if (!gGraphContextManager->contextWithNameExists(name))
            {
                auto context = gGraphContextManager->createNewContext(name);
                context->add(gSelectionRelay->mSelectedModules, gSelectionRelay->mSelectedGates);
                return;
            }
        }
    }

    void GraphGraphicsView::handleMoveAction(QAction* action)
    {
        const u32 mod_id = action->data().toInt();
        Module* m        = gNetlist->get_module_by_id(mod_id);
        for (const auto& id : gSelectionRelay->mSelectedGates)
        {
            m->assign_gate(gNetlist->get_gate_by_id(id));
        }
        for (const auto& id : gSelectionRelay->mSelectedModules)
        {
            gNetlist->get_module_by_id(id)->set_parent_module(m);
        }

        auto gates   = gSelectionRelay->mSelectedGates;
        auto modules = gSelectionRelay->mSelectedModules;
        gSelectionRelay->clear();
        gSelectionRelay->relaySelectionChanged(this);
    }

    void GraphGraphicsView::handleMoveNewAction()
    {
        std::unordered_set<Gate*> gate_objs;
        std::unordered_set<Module*> module_objs;
        for (const auto& id : gSelectionRelay->mSelectedGates)
        {
            gate_objs.insert(gNetlist->get_gate_by_id(id));
        }
        for (const auto& id : gSelectionRelay->mSelectedModules)
        {
            module_objs.insert(gNetlist->get_module_by_id(id));
        }
        Module* parent      = gui_utility::firstCommonAncestor(module_objs, gate_objs);
        QString parent_name = QString::fromStdString(parent->get_name());
        bool ok;
        QString name = QInputDialog::getText(nullptr, "", "New module will be created under \"" + parent_name + "\"\nModule Name:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty())
            return;
        Module* m = gNetlist->create_module(gNetlist->get_unique_module_id(), name.toStdString(), parent);

        for (const auto& id : gSelectionRelay->mSelectedGates)
        {
            m->assign_gate(gNetlist->get_gate_by_id(id));
        }
        for (const auto& id : gSelectionRelay->mSelectedModules)
        {
            gNetlist->get_module_by_id(id)->set_parent_module(m);
        }

        auto gates   = gSelectionRelay->mSelectedGates;
        auto modules = gSelectionRelay->mSelectedModules;
        gSelectionRelay->clear();
        gSelectionRelay->relaySelectionChanged(this);
    }

    void GraphGraphicsView::handleRenameAction()
    {
        if (mItem->itemType() == ItemType::Gate)
        {
            Gate* g            = gNetlist->get_gate_by_id(mItem->id());
            const QString name = QString::fromStdString(g->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change gate name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                g->set_name(new_name.toStdString());
            }
        }
        else if (mItem->itemType() == ItemType::Module)
        {
            Module* m          = gNetlist->get_module_by_id(mItem->id());
            const QString name = QString::fromStdString(m->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change module name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                m->set_name(new_name.toStdString());
            }
        }
        else if (mItem->itemType() == ItemType::Net)
        {
            Net* n             = gNetlist->get_net_by_id(mItem->id());
            const QString name = QString::fromStdString(n->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change net name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                n->set_name(new_name.toStdString());
            }
        }
    }

    void GraphGraphicsView::handleChangeTypeAction()
    {
        if (mItem->itemType() == ItemType::Module)
        {
            Module* m          = gNetlist->get_module_by_id(mItem->id());
            const QString type = QString::fromStdString(m->get_type());
            bool confirm;
            const QString new_type = QInputDialog::getText(this, "Change module type", "New type:", QLineEdit::Normal, type, &confirm);
            if (confirm)
            {
                m->set_type(new_type.toStdString());
            }
        }
    }

    void GraphGraphicsView::adjustMinScale()
    {
        if (!scene())
            return;
        mMinScale = std::min(viewport()->width() / scene()->width(), viewport()->height() / scene()->height());
    }

    void GraphGraphicsView::paintEvent(QPaintEvent* event)
    {
        qreal lod = QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform());

        // USE CONSISTENT METHOD NAMES
        GraphicsScene::setLod(lod);
        GraphicsScene::setGridEnabled(mGridEnabled);
        GraphicsScene::setGridClustersEnabled(mGridClustersEnabled);
        GraphicsScene::setGridType(mGridType);

        GraphicsItem::setLod(lod);
        NodeDragShadow::setLod(lod);

        StandardGraphicsModule::updateAlpha();
        StandardGraphicsGate::updateAlpha();
        StandardGraphicsNet::updateAlpha();
        SeparatedGraphicsNet::updateAlpha();

        QGraphicsView::paintEvent(event);
    }

    void GraphGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        GraphicsItem* item = static_cast<GraphicsItem*>(itemAt(event->pos()));

        if (!item)
            return;

        if (item->itemType() == ItemType::Module)
            Q_EMIT moduleDoubleClicked(item->id());
    }

    void GraphGraphicsView::drawForeground(QPainter* painter, const QRectF& rect)
    {
        Q_UNUSED(rect)

#ifdef GUI_DEBUG_GRID
        if (mDebugGridposEnable)
            debugDrawLayouterGridpos(painter);
#endif

        if (!mMinimapEnabled)
            return;

        QRectF map(viewport()->width() - 210, viewport()->height() - 130, 200, 120);
        painter->resetTransform();
        painter->fillRect(map, QColor(0, 0, 0, 170));
    }

#ifdef GUI_DEBUG_GRID
    void GraphGraphicsView::debugDrawLayouterGridpos(QPainter* painter)
    {
        painter->resetTransform();
        painter->setPen(QPen(Qt::magenta));
        QString pos_str = QString("(%1, %2)").arg(m_debug_gridpos.x()).arg(m_debug_gridpos.y());
        painter->drawText(QPoint(25, 25), pos_str);
    }
#endif

    void GraphGraphicsView::mousePressEvent(QMouseEvent* event)
    {
        if (event->modifiers() == mMoveModifier)
        {
            if (event->button() == Qt::LeftButton)
                mMovePosition = event->pos();
        }
        else if (event->button() == Qt::LeftButton)
        {
            GraphicsItem* item = static_cast<GraphicsItem*>(itemAt(event->pos()));
            if (item && itemDraggable(item))
            {
                mDragItem               = static_cast<GraphicsGate*>(item);
                mDragMousedownPosition = event->pos();
                mDragStartGridpos      = closestLayouterPos(mapToScene(mDragMousedownPosition))[0];
            }
            else
            {
                mDragItem = nullptr;
            }

            // we still need the normal mouse logic for single clicks
            QGraphicsView::mousePressEvent(event);
        }
        else
            QGraphicsView::mousePressEvent(event);
    }

    void GraphGraphicsView::mouseMoveEvent(QMouseEvent* event)
    {
        if (!scene())
            return;

        QPointF delta = mTargetViewportPos - event->pos();

        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            mTargetViewportPos = event->pos();
            mTargetScenePos    = mapToScene(event->pos());
        }

        if (event->buttons().testFlag(Qt::LeftButton))
        {
            if (event->modifiers() == mMoveModifier)
            {
                QScrollBar* hBar  = horizontalScrollBar();
                QScrollBar* vBar  = verticalScrollBar();
                QPoint delta_move = event->pos() - mMovePosition;
                mMovePosition   = event->pos();
                hBar->setValue(hBar->value() + (isRightToLeft() ? delta_move.x() : -delta_move.x()));
                vBar->setValue(vBar->value() - delta_move.y());
            }
            else
            {
                if (mDragItem && (event->pos() - mDragMousedownPosition).manhattanLength() >= QApplication::startDragDistance())
                {
                    QDrag* drag         = new QDrag(this);
                    QMimeData* mimeData = new QMimeData;

                    // TODO set MIME type and icon
                    mimeData->setText("dragTest");
                    drag->setMimeData(mimeData);
                    // drag->setPixmap(iconPixmap);

                    // enable DragMoveEvents until mouse released
                    drag->exec(Qt::MoveAction);
                }
            }
        }
#ifdef GUI_DEBUG_GRID
        debugShowLayouterGridpos(event->pos());
#endif

        QGraphicsView::mouseMoveEvent(event);
    }

    void GraphGraphicsView::dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            event->acceptProposedAction();
            QSizeF size(mDragItem->width(), mDragItem->height());
            QPointF mouse = event->posF();
            QPointF snap  = closestLayouterPos(mapToScene(mouse.x(), mouse.y()))[1];
            if (gSelectionRelay->mSelectedGates.size() > 1)
            {
                // if we are in multi-select mode, reduce the selection to the
                // item we are dragging
                gSelectionRelay->clear();
                gSelectionRelay->mSelectedGates.insert(mDragItem->id());
                gSelectionRelay->mFocusType = SelectionRelay::ItemType::Gate;
                gSelectionRelay->mFocusId   = mDragItem->id();
                gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
            mDropAllowed = false;
            static_cast<GraphicsScene*>(scene())->startDragShadow(snap, size, NodeDragShadow::DragCue::Rejected);
        }
        else
        {
            //causes a bug where dockbar-buttons just disappear instead of snapping back when dragged into the view
//            QGraphicsView::dragEnterEvent(event);
        }
    }

    void GraphGraphicsView::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)
        static_cast<GraphicsScene*>(scene())->stopDragShadow();
    }

    void GraphGraphicsView::dragMoveEvent(QDragMoveEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            bool swapModifier    = event->keyboardModifiers() == mDragModifier;
            QVector<QPoint> snap = closestLayouterPos(mapToScene(event->pos()));

            if (snap[0] == mDragCurrentGridpos && swapModifier == mDragCurrentModifier)
            {
                return;
            }
            mDragCurrentGridpos  = snap[0];
            mDragCurrentModifier = swapModifier;

            auto context            = mGraphWidget->getContext();
            GraphLayouter* layouter = context->debugGetLayouter();
            assert(layouter->done());    // ensure grid stable
            QMap<QPoint, Node>::const_iterator node_iter = layouter->positionToNodeMap().find(snap[0]);

            NodeDragShadow::DragCue cue = NodeDragShadow::DragCue::Rejected;
            // disallow dropping an item on itself
            if (snap[0] != mDragStartGridpos)
            {
                if (swapModifier)
                {
                    if (node_iter != layouter->positionToNodeMap().end())
                    {
                        // allow move only on empty cells
                        cue = NodeDragShadow::DragCue::Swappable;
                    }
                }
                else
                {
                    if (node_iter == layouter->positionToNodeMap().end())
                    {
                        // allow move only on empty cells
                        cue = NodeDragShadow::DragCue::Movable;
                    }
                }
            }
            mDropAllowed = (cue != NodeDragShadow::DragCue::Rejected);

            static_cast<GraphicsScene*>(scene())->moveDragShadow(snap[1], cue);
        }
    }

    void GraphGraphicsView::dropEvent(QDropEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            event->acceptProposedAction();
            GraphicsScene* s = static_cast<GraphicsScene*>(scene());
            s->stopDragShadow();
            if (mDropAllowed)
            {
                auto context            = mGraphWidget->getContext();
                GraphLayouter* layouter = context->debugGetLayouter();
                assert(layouter->done());    // ensure grid stable

                // convert scene coordinates into layouter grid coordinates
                QPointF targetPos        = s->dropTarget();
                QPoint targetLayouterPos = closestLayouterPos(targetPos)[0];
                QPoint sourceLayouterPos = layouter->gridPointByItem(mDragItem);

                if (targetLayouterPos == sourceLayouterPos)
                {
                    qDebug() << "Attempted to drop gate onto itself, this should never happen!";
                    return;
                }
                // assert(targetLayouterPos != sourceLayouterPos);

                bool modifierPressed = event->keyboardModifiers() == mDragModifier;
                if (modifierPressed)
                {
                    // swap mode; swap gates

                    Node nodeFrom = layouter->positionToNodeMap().value(sourceLayouterPos);
                    Node nodeTo   = layouter->positionToNodeMap().value(targetLayouterPos);
                    assert(!nodeFrom.isNull());    // assert that value was found
                    assert(!nodeTo.isNull());
                    layouter->swapNodePositions(nodeFrom, nodeTo);
                }
                else
                {
                    // move mode; move gate to the selected location
                    QMap<QPoint,Node> nodeMap = layouter->positionToNodeMap();
                    auto nodeToMoveIt = nodeMap.find(sourceLayouterPos);
                    Q_ASSERT(nodeToMoveIt != nodeMap.end());
                    layouter->setNodePosition(nodeToMoveIt.value(), targetLayouterPos);
                }
                // re-layout the nets
                context->scheduleSceneUpdate();
            }
        }
        else
        {
            QGraphicsView::dropEvent(event);
        }
    }

    void GraphGraphicsView::wheelEvent(QWheelEvent* event)
    {
        if (QApplication::keyboardModifiers() == mZoomModifier)
        {
            if (event->orientation() == Qt::Vertical)
            {
                qreal angle  = event->angleDelta().y();
                qreal factor = qPow(mZoomFactorBase, angle);
                gentleZoom(factor);
            }
        }
    }

    void GraphGraphicsView::keyPressEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
            case Qt::Key_Space: {
                //qDebug() << "Space pressed";
            }
            break;
        }

        event->ignore();
    }

    void GraphGraphicsView::keyReleaseEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
            case Qt::Key_Space: {
                //qDebug() << "Space released";
            }
            break;
        }

        event->ignore();
    }

    void GraphGraphicsView::resizeEvent(QResizeEvent* event)
    {
        QGraphicsView::resizeEvent(event);
        adjustMinScale();
    }

    void GraphGraphicsView::showContextMenu(const QPoint& pos)
    {
        GraphicsScene* s = static_cast<GraphicsScene*>(scene());

        if (!s)
            return;

        QMenu context_menu(this);
        QAction* action;

        QGraphicsItem* item = itemAt(pos);
        bool isGate         = false;
        bool isModule       = false;
        bool isMultiSelect  = false;
        bool isNet          = false;

        if (item)
        {
            mItem   = static_cast<GraphicsItem*>(item);
            isGate   = mItem->itemType() == ItemType::Gate;
            isModule = mItem->itemType() == ItemType::Module;
            isNet    = mItem->itemType() == ItemType::Net;

            if (isGate)
            {
                if (gSelectionRelay->mSelectedGates.find(mItem->id()) == gSelectionRelay->mSelectedGates.end())
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->mSelectedGates.insert(mItem->id());
                    gSelectionRelay->mFocusType = SelectionRelay::ItemType::Gate;
                    gSelectionRelay->mFocusId   = mItem->id();
                    gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;
                    gSelectionRelay->relaySelectionChanged(this);
                }

                context_menu.addAction("This gate:")->setEnabled(false);

                action = context_menu.addAction("  Change gate name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleRenameAction);

                action = context_menu.addAction("  Fold parent module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleFoldSingleAction);
            }
            else if (isModule)
            {
                if (gSelectionRelay->mSelectedModules.find(mItem->id()) == gSelectionRelay->mSelectedModules.end())
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->mSelectedModules.insert(mItem->id());
                    gSelectionRelay->mFocusType = SelectionRelay::ItemType::Module;
                    gSelectionRelay->mFocusId   = mItem->id();
                    gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;
                    gSelectionRelay->relaySelectionChanged(this);
                }

                context_menu.addAction("This module:")->setEnabled(false);

                action = context_menu.addAction("  Change module name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleRenameAction);

                action = context_menu.addAction("  Change module type");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleChangeTypeAction);

                action = context_menu.addAction("  Unfold module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleUnfoldSingleAction);
            }
            else if (isNet)
            {
                if (gSelectionRelay->mSelectedNets.find(mItem->id()) == gSelectionRelay->mSelectedNets.end())
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->mSelectedNets.insert(mItem->id());
                    gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                    gSelectionRelay->mFocusId   = mItem->id();
                    gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;
                    gSelectionRelay->relaySelectionChanged(this);
                }

                context_menu.addAction("This net:")->setEnabled(false);

                action = context_menu.addAction("  Change net name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleRenameAction);
            }

            if (gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size() > 1)
            {
                context_menu.addSeparator();
                context_menu.addAction("Entire selection:")->setEnabled(false);
            }

            if (gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size() + gSelectionRelay->mSelectedNets.size() > 1)
                isMultiSelect = true;

            if (isGate || isModule)
            {
                action = context_menu.addAction("  Isolate In New View");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleIsolationViewAction);

                action = context_menu.addAction("  Add successors to view");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleSelectOutputs);

                action = context_menu.addAction("  Add predecessors to view");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleSelectInputs);

                Gate* g   = isGate ? gNetlist->get_gate_by_id(mItem->id()) : nullptr;
                Module* m = isModule ? gNetlist->get_module_by_id(mItem->id()) : nullptr;

                // only allow move actions on anything that is not the top module
                if (!(isModule && m == gNetlist->get_top_module()))
                {
                    QMenu* module_submenu = context_menu.addMenu("  Move to module …");

                    action = module_submenu->addAction("New module …");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleMoveNewAction);
                    module_submenu->addSeparator();

                    QActionGroup* module_actions = new QActionGroup(module_submenu);
                    for (auto& module : gNetlist->get_modules())
                    {
                        // don't allow a gate to be moved into its current module
                        // && don't allow a module to be moved into its current module
                        // && don't allow a module to be moved into itself
                        // (either check automatically passes if g respective m is nullptr, so we
                        // don't have to create two loops)
                        if (!module->contains_gate(g) && !module->contains_module(m) && module != m && (!m || !m->contains_module(module)))
                        {
                            QString mod_name = QString::fromStdString(module->get_name());
                            const u32 mod_id = module->get_id();
                            action           = module_submenu->addAction(mod_name);
                            module_actions->addAction(action);
                            action->setData(mod_id);
                        }
                    }
                    QObject::connect(module_actions, SIGNAL(triggered(QAction*)), this, SLOT(handleMoveAction(QAction*)));
                }

                Grouping* assignedGrouping = nullptr;
                if (isGate)
                {
                    if (g)
                        assignedGrouping = g->get_grouping();
                }
                if (isModule)
                {
                    if (m)
                        assignedGrouping = m->get_grouping();
                }
                QMenu* groupingSubmenu;
                if (isMultiSelect)
                    groupingSubmenu = context_menu.addMenu("  Assign all to grouping …");
                else if (assignedGrouping)
                    groupingSubmenu = context_menu.addMenu("  Change grouping assignment …");
                else
                    groupingSubmenu = context_menu.addMenu("  Assign to grouping …");

                QString assignedGroupingName;
                if (assignedGrouping && !isMultiSelect)
                {
                    action = groupingSubmenu->addAction("Delete current assignment");
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingUnassign);
                    assignedGroupingName = QString::fromStdString(assignedGrouping->get_name());
                }
                action = groupingSubmenu->addAction("New grouping …");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingAssignNew);
                for (Grouping* grouping : gNetlist->get_groupings())
                {
                    QString groupingName = QString::fromStdString(grouping->get_name());
                    if (groupingName == assignedGroupingName && !isMultiSelect)
                        continue;
                    action = groupingSubmenu->addAction(sAssignToGrouping + groupingName);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingAssingExisting);
                }
            }

            if (gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size() > 1)
            {
                if (!gSelectionRelay->mSelectedGates.empty())
                {
                    context_menu.addSeparator();
                    context_menu.addAction("All selected gates:")->setEnabled(false);

                    action = context_menu.addAction("  Fold all parent modules");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleFoldAllAction);
                }
                if (!gSelectionRelay->mSelectedModules.empty())
                {
                    context_menu.addSeparator();
                    context_menu.addAction("All selected modules:")->setEnabled(false);

                    action = context_menu.addAction("  Unfold all");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleUnfoldAllAction);
                }
            }
        }

        // if (!item || isNet)
        // {
        // QAction* antialiasing_action = context_menu.addAction("Antialiasing");
        // QAction* cosmetic_action     = context_menu.addAction("Cosmetic Nets");
        // QMenu* grid_menu = context_menu.addMenu("Grid");
        // QMenu* type_menu = grid_menu->addMenu("Type");
        // QMenu* cluster_menu          = grid_menu->addMenu("Clustering");
        // QAction* lines_action        = type_menu->addAction("Lines");
        // QAction* dots_action         = type_menu->addAction("Dots");
        // QAction* none_action         = type_menu->addAction("None");
        // connect(action, &QAction::triggered, this, SLOT);
        // }

        context_menu.exec(mapToGlobal(pos));
        update();
    }

    void GraphGraphicsView::updateMatrix(const int delta)
    {
        qreal scale = qPow(2.0, delta / 100.0);

        QMatrix matrix;
        matrix.scale(scale, scale);
        setMatrix(matrix);
    }

    void GraphGraphicsView::toggleAntialiasing()
    {
        setRenderHint(QPainter::Antialiasing, !(renderHints() & QPainter::Antialiasing));
    }

    bool GraphGraphicsView::itemDraggable(GraphicsItem* item)
    {
        ItemType type = item->itemType();
        return type == ItemType::Gate || type == ItemType::Module;
    }

    void GraphGraphicsView::gentleZoom(const qreal factor)
    {
        scale(factor, factor);
        centerOn(mTargetScenePos);
        QPointF delta_viewport_pos = mTargetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF viewport_center    = mapFromScene(mTargetScenePos) - delta_viewport_pos;
        centerOn(mapToScene(viewport_center.toPoint()));
    }

    void GraphGraphicsView::viewportCenterZoom(const qreal factor)
    {
        QPointF target_pos = mapToScene(viewport()->rect().center());
        scale(factor, factor);
        centerOn(target_pos.toPoint());
    }

    void GraphGraphicsView::handleSelectOutputs()
    {
        auto context           = mGraphWidget->getContext();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : gSelectionRelay->mSelectedGates)
            {
                auto gate = gNetlist->get_gate_by_id(sel_id);
                for (auto net : gate->get_fan_out_nets())
                {
                    for (const auto& suc : net->get_destinations())
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = gNetlist->get_module_by_id(id);
                            if (m->contains_gate(suc->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(suc->get_gate()->get_id());
                        }
                    }
                }
            }
            for (auto sel_id : gSelectionRelay->mSelectedModules)
            {
                auto module = gNetlist->get_module_by_id(sel_id);
                for (auto net : module->get_output_nets())
                {
                    for (const auto& suc : net->get_destinations())
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = gNetlist->get_module_by_id(id);
                            if (m->contains_gate(suc->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(suc->get_gate()->get_id());
                        }
                    }
                }
            }

            context->add({}, gates);
        }
    }
    void GraphGraphicsView::handleSelectInputs()
    {
        auto context           = mGraphWidget->getContext();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : gSelectionRelay->mSelectedGates)
            {
                auto gate = gNetlist->get_gate_by_id(sel_id);
                for (auto net : gate->get_fan_in_nets())
                {
                    if (!net->get_sources().empty() && net->get_sources().at(0)->get_gate() != nullptr)
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = gNetlist->get_module_by_id(id);
                            if (m->contains_gate(net->get_sources().at(0)->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(net->get_sources().at(0)->get_gate()->get_id());
                        }
                    }
                }
            }
            for (auto sel_id : gSelectionRelay->mSelectedModules)
            {
                auto module = gNetlist->get_module_by_id(sel_id);
                for (auto net : module->get_input_nets())
                {
                    if (!net->get_sources().empty() && net->get_sources().at(0)->get_gate() != nullptr)
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = gNetlist->get_module_by_id(id);
                            if (m->contains_gate(net->get_sources().at(0)->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(net->get_sources().at(0)->get_gate()->get_id());
                        }
                    }
                }
            }
            context->add({}, gates);
        }
    }

    void GraphGraphicsView::handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value)
    {
        UNUSED(sender);
        if (key == "graph_view/drag_mode_modifier")
        {
            unsigned int modifier = value.toUInt();
            mDragModifier       = Qt::KeyboardModifier(modifier);
        }
        else if (key == "graph_view/move_modifier")
        {
            unsigned int modifier = value.toUInt();
            mMoveModifier       = Qt::KeyboardModifier(modifier);
        }
#ifdef GUI_DEBUG_GRID
        else if (key == "debug/grid")
        {
            mDebugGridposEnable = value.toBool();
        }
#endif
    }

    void GraphGraphicsView::handleFoldSingleAction()
    {
        auto context = mGraphWidget->getContext();
        context->foldModuleOfGate(mItem->id());
    }

    void GraphGraphicsView::handleUnfoldSingleAction()
    {
        auto context = mGraphWidget->getContext();
        auto m       = gNetlist->get_module_by_id(mItem->id());
        if (m->get_gates().empty() && m->get_submodules().empty())
        {
            QMessageBox msg;
            msg.setText("This module is empty.\nYou can't unfold it.");
            msg.setWindowTitle("Error");
            msg.exec();
            return;
            // We would otherwise unfold the empty module into nothing, so the user
            // would have nowhere to click to get their module back
        }
        context->unfoldModule(mItem->id());
    }

    void GraphGraphicsView::handleFoldAllAction()
    {
        auto context = mGraphWidget->getContext();

        context->beginChange();
        for (u32 id : gSelectionRelay->mSelectedGates)
        {
            context->foldModuleOfGate(id);
        }
        context->endChange();
    }

    void GraphGraphicsView::handleUnfoldAllAction()
    {
        auto context = mGraphWidget->getContext();

        context->beginChange();
        for (u32 id : gSelectionRelay->mSelectedModules)
        {
            context->unfoldModule(id);
        }
        context->endChange();
    }

    void GraphGraphicsView::handleGroupingUnassign()
    {
        Grouping* assignedGrouping = nullptr;
        if (mItem->itemType() == ItemType::Gate)
        {
            Gate* g = gNetlist->get_gate_by_id(mItem->id());
            if (g)
                assignedGrouping = g->get_grouping();
            if (!assignedGrouping)
                return;
            assignedGrouping->remove_gate(g);
        }
        if (mItem->itemType() == ItemType::Module)
        {
            Module* m = gNetlist->get_module_by_id(mItem->id());
            if (m)
                assignedGrouping = m->get_grouping();
            if (!assignedGrouping)
                return;
            assignedGrouping->remove_module(m);
        }
    }

    void GraphGraphicsView::groupingAssignInternal(Grouping* grp)
    {
        if (gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size() + gSelectionRelay->mSelectedNets.size() > 1)
        {
            gContentManager->getSelectionDetailsWidget()->selectionToGroupingInternal(grp);
            return;
        }

        if (mItem->itemType() == ItemType::Gate)
        {
            Gate* g = gNetlist->get_gate_by_id(mItem->id());
            if (g)
                grp->assign_gate(g);
        }
        if (mItem->itemType() == ItemType::Module)
        {
            Module* m = gNetlist->get_module_by_id(mItem->id());
            if (m)
                grp->assign_module(m);
        }
        gSelectionRelay->clear();
        gSelectionRelay->relaySelectionChanged(nullptr);
    }

    void GraphGraphicsView::handleGroupingAssignNew()
    {
        handleGroupingUnassign();
        Grouping* grp = gContentManager->getGroupingManagerWidget()->getModel()->addDefaultEntry();
        if (grp)
            groupingAssignInternal(grp);
    }

    void GraphGraphicsView::handleGroupingAssingExisting()
    {
        handleGroupingUnassign();
        const QAction* action = static_cast<const QAction*>(QObject::sender());
        QString grpName       = action->text();
        if (grpName.startsWith(sAssignToGrouping))
            grpName.remove(0, sAssignToGrouping.size());
        Grouping* grp = gContentManager->getGroupingManagerWidget()->getModel()->groupingByName(grpName);
        if (grp)
            groupingAssignInternal(grp);
    }

#ifdef GUI_DEBUG_GRID
    void GraphGraphicsView::debugShowLayouterGridpos(const QPoint& mouse_pos)
    {
        auto context = mGraphWidget->getContext();
        if (!context)
            return;

        GraphLayouter* layouter = context->debugGetLayouter();
        if (!(layouter->done()))
            return;

        QPointF scene_mouse_pos = mapToScene(mouse_pos);
        QPoint layouter_pos     = closestLayouterPos(scene_mouse_pos)[0];
        m_debug_gridpos         = layouter_pos;
    }
#endif

    QVector<QPoint> GraphGraphicsView::closestLayouterPos(const QPointF& scene_pos) const
    {
        auto context = mGraphWidget->getContext();
        assert(context);

        GraphLayouter* layouter = context->debugGetLayouter();
        assert(layouter->done());    // ensure grid stable

        int default_width      = layouter->defaultGridWidth();
        int default_height     = layouter->defaultGridHeight();
        int min_x              = layouter->minXIndex();
        int min_y              = layouter->minYIndex();
        QVector<qreal> x_vals  = layouter->xValues();
        QVector<qreal> y_vals  = layouter->yValues();
        LayouterPoint x_point = closestLayouterPoint(scene_pos.x(), default_width, min_x, x_vals);
        LayouterPoint y_point = closestLayouterPoint(scene_pos.y(), default_height, min_y, y_vals);
        return QVector({QPoint(x_point.mIndex, y_point.mIndex), QPoint(x_point.mPos, y_point.mPos)});
    }

    GraphGraphicsView::LayouterPoint GraphGraphicsView::closestLayouterPoint(qreal scene_pos, int default_spacing, int min_index, QVector<qreal> sections) const
    {
        int index = min_index;
        qreal pos = 0;
        if (sections.first() > scene_pos)
        {
            int distance   = sections.first() - scene_pos;
            int nSections  = distance / default_spacing;    // this rounds down
            qreal posThis  = sections.first() - nSections * default_spacing;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posPrev  = posThis - default_spacing;
            qreal distPrev = qAbs(scene_pos - posPrev);
            if (distPrev < distThis)
            {
                index -= nSections + 1;
                pos = posPrev;
            }
            else
            {
                index -= nSections;
                pos = posThis;
            }
        }
        else if (sections.last() < scene_pos)
        {
            int distance   = scene_pos - sections.last();
            int nSections  = distance / default_spacing;    // this rounds down
            qreal posThis  = sections.last() + nSections * default_spacing;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posNext  = posThis + default_spacing;
            qreal distNext = qAbs(scene_pos - posNext);
            if (distNext < distThis)
            {
                index += nSections + sections.size();
                pos = posNext;
            }
            else
            {
                index += nSections + sections.size() - 1;
                pos = posThis;
            }
        }
        else
        {
            // binary search for first value in sections larger than or equal to scene_pos
            const qreal* needle = std::lower_bound(sections.constBegin(), sections.constEnd(), scene_pos);
            int i               = needle - sections.begin();    // position of needle in the vector
            index += i;
            // check if we're closer to this or the next position
            qreal posThis  = *needle;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posPrev  = (i > 0) ? sections[i - 1] : (sections.first() - default_spacing);
            qreal distPrev = qAbs(scene_pos - posPrev);
            if (distPrev < distThis)
            {
                index--;
                pos = posPrev;
            }
            else
            {
                pos = posThis;
            }
        }
        return LayouterPoint{index, pos};
    }
}    // namespace hal

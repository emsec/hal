#include "gui/graph_widget/graph_graphics_view.h"

#include "gui/comment_system/comment_speech_bubble.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/drag_controller.h"
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
#include "gui/content_manager/content_manager.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/grouping_dialog/grouping_dialog.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/gui_utils/common_successor_predecessor.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_fold_module.h"
#include "gui/user_action/action_move_node.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_set_selection_focus.h"
#include "gui/user_action/action_unfold_module.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/module_dialog/gate_dialog.h"
#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_utils.h"
#include "gui/module_context_menu/module_context_menu.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDrag>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QWidgetAction>
#include <QDebug>
#include <algorithm>
#include <qmath.h>


namespace hal
{
    const QString GraphGraphicsView::sAssignToGrouping("Assign to grouping ");

    GraphGraphicsView::GraphGraphicsView(GraphWidget* parent)
        : QGraphicsView(parent), mGraphWidget(parent),
          mMinimapEnabled(false), mGridEnabled(true), mGridClustersEnabled(true),
          mGridType(GraphicsScene::GridType::Dots),
          mDragController(new DragController(mGraphWidget,this)),
          mDragModifier(Qt::KeyboardModifier::AltModifier),
          mPanModifier(Qt::KeyboardModifier::ShiftModifier),
          mZoomModifier(Qt::NoModifier),
          mZoomFactorBase(1.0015)
    {
        connect(gSelectionRelay, &SelectionRelay::subfocusChanged, this, &GraphGraphicsView::conditionalUpdate);
        connect(this, &GraphGraphicsView::customContextMenuRequested, this, &GraphGraphicsView::showContextMenu);
 
        setContextMenuPolicy(Qt::CustomContextMenu);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

        setAcceptDrops(true);
        setMouseTracking(true);
    }

    void GraphGraphicsView::conditionalUpdate()
    {
        if (QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()) >= graph_widget_constants::sGateMinLod)
            update();
    }

    void GraphGraphicsView::handleRemoveFromView()
    {
        GraphContext* ctx = gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!ctx) return;
        ActionRemoveItemsFromObject* act =
                new ActionRemoveItemsFromObject(gSelectionRelay->selectedModules(),
                                                gSelectionRelay->selectedGates());
        act->setObject(UserActionObject(ctx->id(),UserActionObjectType::ContextView));

        //  delete context/view if nothing left to show
        QSet<u32> remainingMods = ctx->modules() - gSelectionRelay->selectedModules();
        QSet<u32> remainingGats = ctx->gates()   - gSelectionRelay->selectedGates();
        if (remainingMods.isEmpty() && remainingGats.isEmpty())
        {
            UserActionCompound* compound = new UserActionCompound;
            compound->setUseCreatedObject();
            act->setObjectLock(true);
            compound->addAction(act);
            compound->addAction(new ActionDeleteObject);
            compound->exec();
        }
        else
            act->exec();
    }

    void GraphGraphicsView::handleIsolationViewAction()
    {
        auto selected_modules = gSelectionRelay->selectedModules();
        auto selected_gates = gSelectionRelay->selectedGates();

        // When there is only one module selected, check if there is a context connected to that module
        // If yes, select the context
        // If no, create a new context and connect the module to the context
        if (selected_modules.size() == 1 && selected_gates.empty())
        {
            u32 module_id = *selected_modules.begin();
            auto module_context = gGraphContextManager->getContextByExclusiveModuleId(module_id);
            if (module_context)
            {
                gContentManager->getContextManagerWidget()->selectViewContext(module_context);
                gContentManager->getContextManagerWidget()->handleOpenContextClicked();
            }
            else
            {
                UserActionCompound* act = new UserActionCompound;
                act->setUseCreatedObject();
                QString name = QString::fromStdString(gNetlist->get_module_by_id(module_id)->get_name()) + " (ID: " + QString::number(module_id) + ")";
                act->addAction(new ActionCreateObject(UserActionObjectType::ContextView, name));
                act->addAction(new ActionAddItemsToObject(selected_modules, selected_gates));
                act->exec();
                GraphContext* context = gGraphContextManager->getContextById(act->object().id());
                context->setDirty(false);
                context->setExclusiveModuleId(module_id);
            }
            return;
        }

        QString name = gGraphContextManager->nextViewName("Isolated View");
        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::ContextView, name));
        act->addAction(new ActionAddItemsToObject(selected_modules, selected_gates));
        act->exec();
        GraphContext* context = gGraphContextManager->getContextById(act->object().id());
        context->setDirty(false);
    }

    void GraphGraphicsView::adjustMinScale()
    {
        if (!scene())
            return;
        mMinScale = std::min(viewport()->width() / scene()->width(), viewport()->height() / scene()->height());
    }

    void GraphGraphicsView::handleAddCommentAction()
    {
        auto action = dynamic_cast<QAction*>(sender());
        if(!action) return;

        auto node = action->data().value<Node>();
        CommentDialog commentDialog("New Comment");
        if(commentDialog.exec() == QDialog::Accepted)
            gCommentManager->addComment(new CommentEntry(node, commentDialog.getText(), commentDialog.getHeader()));
        commentDialog.close();
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

        QGraphicsItem* item = itemAt(event->pos());
        if (!item) return;

        CommentSpeechBubble* csb = dynamic_cast<CommentSpeechBubble*>(item);
        if (csb)
        {
            csb->mouseDoubleClickEvent(nullptr);
            return;
        }

        GraphicsItem* git = dynamic_cast<GraphicsItem*>(itemAt(event->pos()));

        if (!git || git->itemType() != ItemType::Module)
            return;

        Q_EMIT moduleDoubleClicked(git->id());
    }

    void GraphGraphicsView::drawForeground(QPainter* painter, const QRectF& rect)
    {
        Q_UNUSED(rect)

#ifdef GUI_DEBUG_GRID
        if(mGraphWidget->getContext()->scene()->debugGridEnabled())
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
        // it the clicked item is a speechbubble, simply return so that it does
        // not change / clear the current selection, double-click however works fine
        if(dynamic_cast<CommentSpeechBubble*>(itemAt(event->pos())))
            return;

        if ((event->modifiers() == mPanModifier && event->button() == Qt::LeftButton) ||
                (event->button() == Qt::MiddleButton && gGraphContextManager->sSettingPanOnMiddleButton->value().toBool()))
        {
            mMovePosition = event->pos();
        }
        else if (event->button() == Qt::LeftButton)
        {
            GraphicsNode* item = dynamic_cast<GraphicsNode*>(itemAt(event->pos()));
            if (item && itemDraggable(item))
            {
                mDragController->set(item, event->pos());
            }
            else
            {
                mDragController->clear();
            }

            // we still need the normal mouse logic for single clicks
            mousePressEventNotItemDrag(event);
        }
        else
            mousePressEventNotItemDrag(event);
    }

    void GraphGraphicsView::mousePressEventNotItemDrag(QMouseEvent *event)
    {
        QGraphicsView::mousePressEvent(event);
        GraphicsScene* sc = dynamic_cast<GraphicsScene*>(scene());
        if (sc) sc->setMousePressed(true);
    }


    void GraphGraphicsView::mouseReleaseEvent(QMouseEvent *event)
    {
        GraphicsScene* sc = dynamic_cast<GraphicsScene*>(scene());
        if (sc) sc->setMousePressed(false);
        QGraphicsView::mouseReleaseEvent(event);
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

        if ((event->buttons().testFlag(Qt::LeftButton) && event->modifiers() == mPanModifier) ||
                (event->buttons().testFlag(Qt::MiddleButton) && gGraphContextManager->sSettingPanOnMiddleButton->value().toBool()))
        {
            QScrollBar* hBar  = horizontalScrollBar();
            QScrollBar* vBar  = verticalScrollBar();
            QPoint delta_move = event->pos() - mMovePosition;
            mMovePosition   = event->pos();
            hBar->setValue(hBar->value() + (isRightToLeft() ? delta_move.x() : -delta_move.x()));
            vBar->setValue(vBar->value() - delta_move.y());
        }
        else if (event->buttons().testFlag(Qt::LeftButton))
        {
            if (mDragController->hasDragged(event->pos()))
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
#ifdef GUI_DEBUG_GRID
        debugShowLayouterGridpos(event->pos());
#endif

        QGraphicsView::mouseMoveEvent(event);
    }

    void GraphGraphicsView::dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            mDragController->enterDrag(event->keyboardModifiers() == mDragModifier);
            event->acceptProposedAction();
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
        mDragController->clear();
    }

    void GraphGraphicsView::dragPan(float dpx, float dpy)
    {
        if (dpx != 0)
        {
            QScrollBar* hBar  = horizontalScrollBar();
            int hValue = hBar->value() + 10*dpx;
            if (hValue < hBar->minimum()) hBar->setMinimum(hValue);
            if (hValue > hBar->maximum()) hBar->setMaximum(hValue);
            hBar->setValue(hValue);
        }
        if (dpy != 0)
        {
            QScrollBar* vBar  = verticalScrollBar();
            int vValue = vBar->value() + 10*dpy;
            if (vValue < vBar->minimum()) vBar->setMinimum(vValue);
            if (vValue > vBar->maximum()) vBar->setMaximum(vValue);
            vBar->setValue(vValue);
        }
    }

    void GraphGraphicsView::dragMoveEvent(QDragMoveEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            QPair<QPoint,QPointF> snap = closestLayouterPos(mapToScene(event->pos()));

            mDragController->move(event->pos(),event->keyboardModifiers() == mDragModifier,snap.first);

            QPoint p = event->pos() - viewport()->geometry().topLeft();
            float rx = 100. * p.x() / viewport()->geometry().width();
            float ry = 100. * p.y() / viewport()->geometry().height();
//            qDebug() << "move it" << event->pos() << viewport()->geometry() << rx << ry;
            float dpx = 0;
            float dpy = 0;
            if (rx < 10) dpx = -10+rx;
            if (rx > 90) dpx =  rx-90;
            if (ry < 10) dpy = -10+ry;
            if (ry > 90) dpy =  ry-90;
            if (dpx !=0 || dpy != 0)
                dragPan(dpx, dpy);
        }
    }

    void GraphGraphicsView::dropEvent(QDropEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            event->acceptProposedAction();
            if (mDragController->isDropAllowed())
            {
                GridPlacement* plc = mDragController->finalGridPlacement();
                GraphContext* context = mGraphWidget->getContext();
                GraphLayouter* layouter = context->getLayouter();
                assert(layouter->done());    // ensure grid stable
                ActionMoveNode* act = new ActionMoveNode(context->id(),plc);
                if (act->exec())
                    context->setDirty(true);
                delete plc;
            }
        }
        else
        {
            QGraphicsView::dropEvent(event);
        }
        mDragController->clear();
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
        bool isNet          = false;

        // otherwise crashes, speechbubbles do not need a context menu (for now)
        // add ItemType::SpeechBubble so that it can be handled better?
        if(dynamic_cast<CommentSpeechBubble*>(item)) return;

        bool isMultiGates = gSelectionRelay->selectedGates().size() > 1 &&
                gSelectionRelay->selectedModules().isEmpty();
        if (item)
        {
            mItem   = static_cast<GraphicsItem*>(item);
            isGate   = mItem->itemType() == ItemType::Gate;
            isModule = mItem->itemType() == ItemType::Module;
            isNet    = mItem->itemType() == ItemType::Net;

            if (isGate)
            {
                if (!gSelectionRelay->containsGate(mItem->id()))
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->addGate(mItem->id());
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,mItem->id());
                    gSelectionRelay->relaySelectionChanged(this);
                }
            }
            else if (isModule)
            {
                if (!gSelectionRelay->containsModule(mItem->id()))
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->addModule(mItem->id());
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Module,mItem->id());
                    gSelectionRelay->relaySelectionChanged(this);
                }
            }
            else if (isNet)
            {
                if (!gSelectionRelay->containsNet((mItem->id())))
                {
                    gSelectionRelay->clear();
                    gSelectionRelay->addNet(mItem->id());
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Net,mItem->id());
                    gSelectionRelay->relaySelectionChanged(this);
                }
            }

            if (isGate || isModule)
            {
                QMenu* preSucMenu = context_menu.addMenu("Successor/Predecessor …");
                recursionLevelMenu(preSucMenu->addMenu("Add successors to view …"),           true, &GraphGraphicsView::handleAddSuccessorToView);
                if (isMultiGates)
                    recursionLevelMenu(preSucMenu->addMenu("Add common successors to view …"),true, &GraphGraphicsView::handleAddCommonSuccessorToView);
                if (isGate)
                {
                    action = preSucMenu->addAction("Add path to successor gate to view …");
                    action->setData(SuccessorGate);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleShortestPathToView);
                    action = preSucMenu->addAction("Add path to successor module to view …");
                    action->setData(SuccessorModule);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleShortestPathToView);
                }
                recursionLevelMenu(preSucMenu->addMenu("Highlight successors …"),             true, &GraphGraphicsView::handleHighlightSuccessor, true);
                recursionLevelMenu(preSucMenu->addMenu("Highlight successors by distance …"), true, &GraphGraphicsView::handleSuccessorDistance);
                if (isGate)
                {
                    action = preSucMenu->addAction("Highlight path to successor gate …");
                    action->setData(SuccessorGate);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleQueryShortestPathGate);
                    action = preSucMenu->addAction("Highlight path to successor module …");
                    action->setData(SuccessorModule);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleQueryShortestPathModule);
                }

                preSucMenu->addSeparator();
                recursionLevelMenu(preSucMenu->addMenu("Add predecessors to view …"),           false, &GraphGraphicsView::handleAddPredecessorToView);
                if (isMultiGates)
                    recursionLevelMenu(preSucMenu->addMenu("Add common predecessors to view …"),false, &GraphGraphicsView::handleAddCommonPredecessorToView);
                if (isGate)
                {
                    action = preSucMenu->addAction("Add path to predecessor gate to view …");
                    action->setData(PredecessorGate);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleShortestPathToView);
                    action = preSucMenu->addAction("Add path to predecessor module to view …");
                    action->setData(PredecessorModule);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleShortestPathToView);
                }
                recursionLevelMenu(preSucMenu->addMenu("Highlight predecessors …"),             false, &GraphGraphicsView::handleHighlightPredecessor, true);
                recursionLevelMenu(preSucMenu->addMenu("Highlight predecessors by distance …"), false, &GraphGraphicsView::handlePredecessorDistance);
                if (isGate)
                {
                    action = preSucMenu->addAction("Highlight path to predecessor gate …");
                    action->setData(PredecessorGate);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleQueryShortestPathGate);
                    action = preSucMenu->addAction("Highlight path to predecessor module …");
                    action->setData(PredecessorModule);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleQueryShortestPathModule);
                }
            }

            action = context_menu.addAction("Remove selected items from view");
            connect(action, &QAction::triggered, this, &GraphGraphicsView::handleRemoveFromView);

            if (!gContentManager->getGraphTabWidget()->isSelectMode())
            {
                action = context_menu.addAction("Cancel pick-item mode");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleCancelPickMode);
            }

            if(isModule)
                ModuleContextMenu::addModuleSubmenu(&context_menu, mItem->id());
            else if(isGate)
                ModuleContextMenu::addGateSubmenu(&context_menu, mItem->id());
            else if(isNet)
                ModuleContextMenu::addNetSubmenu(&context_menu, mItem->id());

            // Appended to single selected item menu
            if(isGate || isModule)
            {
                action = context_menu.addAction("  Fold parent module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleFoldParentSingle);
            }
            if(isModule)
            {
                action = context_menu.addAction("  Unfold module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handleUnfoldSingleAction);
            }

            if (gSelectionRelay->numberSelectedItems() > 1)
            {
                ModuleContextMenu::addMultipleElementsSubmenu(&context_menu, gSelectionRelay->selectedModules(), gSelectionRelay->selectedGates(), gSelectionRelay->selectedNets());

                // Appended to multiple selected items menu
                action = context_menu.addAction("  Fold all parent modules");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleFoldParentAll);
                action = context_menu.addAction("  Isolate all in new view");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleIsolationViewAction);
                action = context_menu.addAction("  Unfold all selected modules");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleUnfoldAllAction);

                action = context_menu.addAction("  Add comment");
                QVariant data;
                data.setValue(Node(mItem->id(), isGate ? Node::NodeType::Gate : Node::NodeType::Module));
                action->setData(data);
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleAddCommentAction);
            }
        }
        else
        {
            context_menu.addAction("This view:")->setEnabled(false);

            action = context_menu.addAction("  Add module to view");
            connect(action, &QAction::triggered, this, &GraphGraphicsView::handleAddModuleToView);

            int selectable_modules_count = 0;
            QSet<u32> not_selectable_modules = getNotSelectableModules();

            for (Module* m : gNetlist->get_modules())
                if (!not_selectable_modules.contains(m->get_id()))
                    selectable_modules_count++;

            if (selectable_modules_count == 0)
                action->setDisabled(true);

            action = context_menu.addAction("  Add gate to view");
            connect(action, &QAction::triggered, this, &GraphGraphicsView::handleAddGateToView);
            if (getSelectableGates().empty())
                 action->setDisabled(true);
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

        GuiPluginManager::addPluginSubmenus(&context_menu, gNetlist, 
                                            gSelectionRelay->selectedModulesVector(),
                                            gSelectionRelay->selectedGatesVector(),
                                            gSelectionRelay->selectedNetsVector());
        context_menu.exec(mapToGlobal(pos));
        update();
    }

    void GraphGraphicsView::handlePluginContextContributionTriggered()
    {
        QAction* act = static_cast<QAction*>(sender());
        Q_ASSERT(act);
        ContextMenuContribution* cmc = static_cast<ContextMenuContribution*>(act->data().value<void*>());
        Q_ASSERT(cmc);
        Q_ASSERT(cmc->mContributer);
        cmc->mContributer->execute_function(cmc->mTagname,gNetlist,
                                            gSelectionRelay->selectedModulesVector(),
                                            gSelectionRelay->selectedGatesVector(),
                                            gSelectionRelay->selectedNetsVector());
        if (gPythonContext->pythonThread())
            gPythonContext->pythonThread()->unlock();
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

    void GraphGraphicsView::handleCancelPickMode()
    {
        gContentManager->getGraphTabWidget()->emitTerminatePicker();
    }

    namespace ShortestPath
    {
        const Net* net(const Gate* g0, const Gate* g1)
        {
            for (const Net* n0 : g0->get_fan_out_nets())
                for (const Net* n1 : g1->get_fan_in_nets())
                    if (n0 == n1)
                        return n0;
            return nullptr;
        }

        const Module* module(const Gate* g, const NodeBoxes& boxes)
        {
            const Module* parent = g->get_module();
            while (parent)
            {
                Node nd(parent->get_id(),Node::Module);
                if (boxes.boxForNode(nd)) return parent;
                parent = parent->get_parent_module();
            }
            return nullptr;
        }

    }


    void GraphGraphicsView::handleAddModuleToView()
    {
        GraphContext* context = mGraphWidget->getContext();

//        QSet<u32> not_selectable_modules;
//        QSet<u32> modules_in_context = context->modules();
//        QSet<u32> gates_in_context = context->gates();

//        for (Module* module : gNetlist->get_modules())
//        {
//            bool module_in_context = false;
//            for (Module* submodule: module->get_submodules(nullptr, true))
//            {
//                if (modules_in_context.contains(submodule->get_id()))
//                {
//                    module_in_context = true;
//                    break;
//                }
//            }
//            for (Gate* subgate : module->get_gates(nullptr, true))
//            {
//                if (gates_in_context.contains(subgate->get_id()))
//                {
//                    module_in_context = true;
//                    break;
//                }
//            }
//            if (module_in_context)
//            {
//                not_selectable_modules.insert(module->get_id());
//            }
//        }

//        not_selectable_modules += modules_in_context;

//        QSet<u32> direct_par_modules;
//        for (u32 id : modules_in_context)
//        {
//            Module* cur_module = gNetlist->get_module_by_id(id);
//            for (Module* module : cur_module->get_submodules(nullptr, true))
//            {
//                not_selectable_modules.insert(module->get_id());
//            }

//            if (!cur_module->is_top_module())
//            {
//                direct_par_modules.insert(cur_module->get_parent_module()->get_id());
//            }
//        }

//        if (!gates_in_context.empty())
//        {
//            for (u32 id : gates_in_context)
//            {
//                direct_par_modules.insert(gNetlist->get_gate_by_id(id)->get_module()->get_id());
//            }
//        }

//        for (u32 id : direct_par_modules)
//        {
//            not_selectable_modules.insert(id);

//            Module* tmp_module = gNetlist->get_module_by_id(id);
//            while (!tmp_module->is_top_module())
//            {
//                Module* par_module = tmp_module->get_parent_module();
//                tmp_module = par_module;
//                not_selectable_modules.insert(par_module->get_id());
//            }
//        }

        ModuleDialog module_dialog(getNotSelectableModules(),"Add module to view", true, nullptr, this);
        if (module_dialog.exec() == QDialog::Accepted)
        {
            QSet<u32> module_to_add;
            module_to_add.insert(module_dialog.selectedId());
            ActionAddItemsToObject* act = new ActionAddItemsToObject(module_to_add, {});
            act->setObject(UserActionObject(context->id(), UserActionObjectType::ContextView));
            act->exec();
        }
    }


    QSet<u32> GraphGraphicsView::getNotSelectableModules()
    {
        GraphContext* context = mGraphWidget->getContext();

        QSet<u32> not_selectable_modules;
        QSet<u32> modules_in_context = context->modules();
        QSet<u32> gates_in_context = context->gates();

        for (Module* module : gNetlist->get_modules())
        {
            bool module_in_context = false;
            for (Module* submodule: module->get_submodules(nullptr, true))
            {
                if (modules_in_context.contains(submodule->get_id()))
                {
                    module_in_context = true;
                    break;
                }
            }
            for (Gate* subgate : module->get_gates(nullptr, true))
            {
                if (gates_in_context.contains(subgate->get_id()))
                {
                    module_in_context = true;
                    break;
                }
            }
            if (module_in_context)
            {
                not_selectable_modules.insert(module->get_id());
            }
        }

        not_selectable_modules += modules_in_context;

        QSet<u32> direct_par_modules;
        for (u32 id : modules_in_context)
        {
            Module* cur_module = gNetlist->get_module_by_id(id);
            for (Module* module : cur_module->get_submodules(nullptr, true))
            {
                not_selectable_modules.insert(module->get_id());
            }

            if (!cur_module->is_top_module())
            {
                direct_par_modules.insert(cur_module->get_parent_module()->get_id());
            }
        }

        if (!gates_in_context.empty())
        {
            for (u32 id : gates_in_context)
            {
                direct_par_modules.insert(gNetlist->get_gate_by_id(id)->get_module()->get_id());
            }
        }

        for (u32 id : direct_par_modules)
        {
            not_selectable_modules.insert(id);

            Module* tmp_module = gNetlist->get_module_by_id(id);
            while (!tmp_module->is_top_module())
            {
                Module* par_module = tmp_module->get_parent_module();
                tmp_module = par_module;
                not_selectable_modules.insert(par_module->get_id());
            }
        }

        return not_selectable_modules;
    }

    QSet<u32> GraphGraphicsView::getSelectableGates()
    {
        GraphContext* context = mGraphWidget->getContext();

        QSet<u32> not_selectable_gates = context->gates();
        QSet<u32> modules_in_context = context->modules();

        for (u32 module_id : modules_in_context)
        {
            for (Gate* gate : gNetlist->get_module_by_id(module_id)->get_gates(nullptr, true))
            {
                not_selectable_gates.insert(gate->get_id());
            }
        }

        QSet<u32> selectable_gates;
        for (Gate* gate : gNetlist->get_gates())
        {
            if (!not_selectable_gates.contains(gate->get_id()))
            {
                selectable_gates.insert(gate->get_id());
            }
        }

        return selectable_gates;
    }

    void GraphGraphicsView::handleAddGateToView()
    {
        QSet<u32> selectable_gates = getSelectableGates();

        GraphContext* context = mGraphWidget->getContext();

        GateDialog gate_dialog(selectable_gates, "Add gate to view", nullptr, this);
        if (gate_dialog.exec() == QDialog::Accepted)
        {
            QSet<u32> gate_to_add;
            gate_to_add.insert(gate_dialog.selectedId());
            ActionAddItemsToObject* act = new ActionAddItemsToObject({}, gate_to_add);
            act->setObject(UserActionObject(context->id(), UserActionObjectType::ContextView));
            act->exec();
        }
    }


    void GraphGraphicsView::handleAddSuccessorToView()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        addSuccessorToView(level, true);
    }

    void GraphGraphicsView::handleAddPredecessorToView()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        addSuccessorToView(level, false);
    }

    void GraphGraphicsView::handleAddCommonSuccessorToView()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        addCommonSuccessorToView(level, true);
    }

    void GraphGraphicsView::handleAddCommonPredecessorToView()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        addCommonSuccessorToView(level, false);
    }

    void GraphGraphicsView::addSuccessorToView(int maxLevel, bool succ)
    {

        QSet<u32> gatsNew;
        QSet<const Gate*> gatsHandled;

        QList<const Gate*> startList;
        Node startNode;

        switch (mItem->itemType())
        {
        case ItemType::Gate:
            startNode = Node(mItem->id(),Node::Gate);
            startList.append(gNetlist->get_gate_by_id(mItem->id()));
            break;
        case ItemType::Module:
            startNode = Node(mItem->id(),Node::Module);
            for (const Gate* g : gNetlist->get_module_by_id(mItem->id())->get_gates(nullptr,true))
                startList.append(g);
            break;
        default:
            return;
        }

        Q_ASSERT(startList.size());

        for (const Gate* g : startList)
            gatsHandled.insert(g);

        const NodeBoxes& boxes = mGraphWidget->getContext()->getLayouter()->boxes();
        const NodeBox* box = boxes.boxForNode(startNode);
        Q_ASSERT(box);
        int xOrigin = box->x();
        int yOrigin = box->y();
        int xDir = succ ? 1 : -1;

        PlacementHint plc(PlacementHint::Standard);

        for (int loop = 0; !maxLevel || loop<maxLevel; loop++)
        {
            int y = 0;
            QList<const Gate*> foundList;

            for (const Gate* gOrigin : startList)
            {
                for (const Gate* g : netlist_utils::get_next_gates(gOrigin, succ, 1))
                {
                    if (gatsHandled.contains(g)) continue;
                    gatsHandled.insert(g);
                    if (boxes.boxForGate(g)) continue; // by in view
                    foundList.append(g);
                }
            }

            if (foundList.isEmpty()) break;
            for (const Gate* g: foundList)
            {
                gatsNew.insert(g->get_id());
                QPoint point(xOrigin + (loop+1) * xDir, yOrigin + y);
                y = y > 0 ? -y : -y+1;
                Node nd(g->get_id(),Node::Gate);
                if (!boxes.boxForPoint(point)) // not occupied yet
                {
                    if (plc.mode() == PlacementHint::Standard)
                        plc = PlacementHint(PlacementHint::GridPosition);
                    plc.addGridPosition(nd,point);
                }
            }
            startList = foundList;
        }

        ActionAddItemsToObject* act = new ActionAddItemsToObject({}, gatsNew);
        act->setObject(UserActionObject(mGraphWidget->getContext()->id(),UserActionObjectType::ContextView));
        act->setPlacementHint(plc);
        act->exec();
    }


    void GraphGraphicsView::addCommonSuccessorToView(int maxLevel, bool succ)
    {
        CommonSuccessorPredecessor csp(gSelectionRelay->selectedGatesList(),succ,maxLevel);
        const NodeBoxes& boxes = mGraphWidget->getContext()->getLayouter()->boxes();

        QSet<u32> gatsNew;
        for (const Gate* g : csp.result())
        {
            if (boxes.boxForGate(g)) continue;
            gatsNew.insert(g->get_id());
        }
        ActionAddItemsToObject* act = new ActionAddItemsToObject({}, gatsNew);
        act->setObject(UserActionObject(mGraphWidget->getContext()->id(),UserActionObjectType::ContextView));
        act->exec();
    }

    void GraphGraphicsView::handleHighlightSuccessor()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        gContentManager->getGroupingManagerWidget()->newGroupingSuccOrPred(level,true,mItem);
    }

    void GraphGraphicsView::handleHighlightPredecessor()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        gContentManager->getGroupingManagerWidget()->newGroupingSuccOrPred(level,false,mItem);
    }

    void GraphGraphicsView::handleSuccessorDistance()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        gContentManager->getGroupingManagerWidget()->newGroupingByDistance(level,true,mItem);
    }

    void GraphGraphicsView::handlePredecessorDistance()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        int level = send->data().toInt();
        gContentManager->getGroupingManagerWidget()->newGroupingByDistance(level,false,mItem);
    }

    void GraphGraphicsView::handleShortestPathToView()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);

        int param = send->data().toInt();
        bool succ = (param == SuccessorModule || param == SuccessorGate);
        bool isMod = (param == SuccessorModule || param == PredecessorModule);
        std::vector<Gate*> spath;
        Gate* startGate = gNetlist->get_gate_by_id(mItem->id());
        Q_ASSERT(startGate);

        if (isMod)
        {
            QSet<u32> excludeModules;
            const Module* parMod = startGate->get_module();
            while (parMod)
            {
                excludeModules.insert(parMod->get_id());
                parMod = parMod->get_parent_module();
            }

            ModuleDialog md(excludeModules, "Shortest path to module", true, nullptr, this);

            if (md.exec() != QDialog::Accepted) return;

            Module* endModule = gNetlist->get_module_by_id(md.selectedId());
            Q_ASSERT(endModule);

            spath = netlist_utils::get_shortest_path(startGate,endModule,succ);
        }
        else
        {
            QSet<u32> selectableGates;

            for (Gate* g : netlist_utils::get_next_gates(startGate,succ))
            {
                selectableGates.insert(g->get_id());
            }

//        GraphGraphicsViewNeighborSelector* ggvns = new GraphGraphicsViewNeighborSelector(mItem->id(), succ, this);
            GateDialog gd(selectableGates, QString("Shortest path %1 gate").arg(succ?"to":"from"), nullptr, this);

            if (gd.exec() != QDialog::Accepted) return;

            Gate* endGate = gNetlist->get_gate_by_id(gd.selectedId());
            Q_ASSERT(endGate);

            if (succ)
                spath = netlist_utils::get_shortest_path(startGate,endGate);
            else
            {
                spath = netlist_utils::get_shortest_path(endGate,startGate);
            }
        }
        if (spath.empty()) return;
        if (!succ) std::reverse(spath.begin(), spath.end());

        const NodeBoxes& boxes = mGraphWidget->getContext()->getLayouter()->boxes();
        const NodeBox* originBox = boxes.boxForGate(startGate);
        Q_ASSERT(originBox);
        QPoint point(originBox->x(),originBox->y());
        QPoint deltaX(succ ? 1 : -1, 0);
        PlacementHint plc(PlacementHint::Standard);

        QSet<u32> gats;

        for (Gate* g : spath)
        {
            if (boxes.boxForGate(g)) continue; // already in view
            do {
                point += deltaX;
            } while (boxes.boxForPoint(point)); // find grid point not occupied
            gats.insert(g->get_id());
            Node nd(g->get_id(),Node::Gate);
            if (plc.mode() == PlacementHint::Standard)
                plc = PlacementHint(PlacementHint::GridPosition);
            plc.addGridPosition(nd,point);
        }

        ActionAddItemsToObject* act = new ActionAddItemsToObject({},gats);
        act->setObject(UserActionObject(mGraphWidget->getContext()->id(),UserActionObjectType::ContextView));
        act->setPlacementHint(plc);
        act->exec();
    }

    void GraphGraphicsView::handleQueryShortestPathModule()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        bool succ = send->data().toInt() == SuccessorGate;

        QSet<u32> excludeModules;
        Gate* g = gNetlist->get_gate_by_id(mItem->id());
        Q_ASSERT(g);
        Module* par = g->get_module();
        while(par)
        {
            excludeModules.insert(par->get_id());
            par = par->get_parent_module();
        }
        ModuleDialog md(excludeModules, "Shortest path to module", true, nullptr, this);

        if (md.exec() != QDialog::Accepted) return;

        handleShortestPathToGrouping(mItem->id(),Node(md.selectedId(),Node::Module),succ);
    }

    void GraphGraphicsView::handleQueryShortestPathGate()
    {
        QAction* send = static_cast<QAction*>(sender());
        Q_ASSERT(send);
        bool succ = send->data().toInt() == SuccessorGate;

        QSet<u32> selectableGates;
        for (Gate* g : netlist_utils::get_next_gates(gNetlist->get_gate_by_id(mItem->id()),succ))
            selectableGates.insert(g->get_id());

        GraphGraphicsViewNeighborSelector* ggvns = new GraphGraphicsViewNeighborSelector(mItem->id(), succ, this);
        GateDialog gd(selectableGates,QString("Shortest path %1 gate").arg(succ?"to":"from"),ggvns,this);

        if (gd.exec() != QDialog::Accepted) return;

        u32 targetId = gd.selectedId();
        if (!targetId) return;

        if (succ)
            handleShortestPathToGrouping(mItem->id(),Node(targetId,Node::Gate),true);
        else
            handleShortestPathToGrouping(targetId,Node(mItem->id(),Node::Gate),true);
    }

    void GraphGraphicsView::handleShortestPathToGrouping(u32 idFrom, Node nodeTo, bool forwardDirection)
    {
        std::vector<Gate*> spath;

        Gate* g0 = gNetlist->get_gate_by_id(idFrom);
        Q_ASSERT(g0);
        Q_ASSERT(!nodeTo.isNull());

        QString target;

        if(nodeTo.isGate())
        {
            Gate* g1 = gNetlist->get_gate_by_id(nodeTo.id());
            Q_ASSERT(g1);
            spath = forwardDirection ?
                        netlist_utils::get_shortest_path(g0,g1) :
                        netlist_utils::get_shortest_path(g1,g0);
            target = QString("gate '%1'[%2]").arg(QString::fromStdString(g1->get_name()), g1->get_id());
        }
        else
        {
            Module* m = gNetlist->get_module_by_id(nodeTo.id());
            Q_ASSERT(m);
            spath = netlist_utils::get_shortest_path(g0,m,forwardDirection);
            target = QString("module '%1'[%2]").arg(QString::fromStdString(m->get_name()), m->get_id());
        }


        QSet<u32> mods;
        QSet<u32> gats;
        QSet<u32> nets;

        Gate* previousGate = nullptr;
        for (Gate* g : spath)
        {
            const Module* pm = ShortestPath::module(g,mGraphWidget->getContext()->getLayouter()->boxes());
            if (pm) mods.insert(pm->get_id());
            gats.insert(g->get_id());
            if (previousGate)
            {
                const Net* n = ShortestPath::net(previousGate,g);
                if (n) nets.insert(n->get_id());
            }
            previousGate = g;
        }

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Grouping,
                                              QString("Path from '%1'[%2] to %3")
                                              .arg(QString::fromStdString(g0->get_name())).arg(g0->get_id())
                                              .arg(target)));
        act->addAction(new ActionAddItemsToObject(mods,gats,nets));
        act->addAction(new ActionSetSelectionFocus());
        act->exec();
    }

    void GraphGraphicsView::handleSelectOutputs()
    {
        auto context           = mGraphWidget->getContext();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : gSelectionRelay->selectedGatesList())
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
            for (auto sel_id : gSelectionRelay->selectedModulesList())
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

            gates = context->getLayouter()->boxes().filterNotInView(gates);
            ActionAddItemsToObject* act = new ActionAddItemsToObject({},gates);
            act->setObject(UserActionObject(context->id(),UserActionObjectType::ContextView));
            if (gSelectionRelay->numberSelectedNodes()==1)
            {
                Node origin = (gSelectionRelay->numberSelectedModules()==1)
                   ? Node(gSelectionRelay->selectedModulesList().at(0),Node::Module)
                   : Node(gSelectionRelay->selectedGatesList().at(0),Node::Gate);
                act->setPlacementHint(PlacementHint(PlacementHint::PreferRight,origin));
            }
            act->exec();
        }
    }
    void GraphGraphicsView::handleSelectInputs()
    {
        auto context           = mGraphWidget->getContext();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : gSelectionRelay->selectedGatesList())
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
            for (auto sel_id : gSelectionRelay->selectedModulesList())
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

            gates = context->getLayouter()->boxes().filterNotInView(gates);
            ActionAddItemsToObject* act = new ActionAddItemsToObject({},gates);
            act->setObject(UserActionObject(context->id(),UserActionObjectType::ContextView));
            if (gSelectionRelay->numberSelectedNodes()==1)
            {
                Node origin = (gSelectionRelay->numberSelectedModules()==1)
                   ? Node(gSelectionRelay->selectedModulesList().at(0),Node::Module)
                   : Node(gSelectionRelay->selectedGatesList().at(0),Node::Gate);
                act->setPlacementHint(PlacementHint(PlacementHint::PreferLeft,origin));
            }
            act->exec();
        }
    }

    void GraphGraphicsView::selectedNodeToItem()
    {
        if (gSelectionRelay->numberSelectedItems() != 1) return;
        NodeBox* box = nullptr;
        if (gSelectionRelay->numberSelectedGates() == 1)
        {
            u32 gatId = gSelectionRelay->selectedGatesList().at(0);
            box = mGraphWidget->getContext()->getLayouter()->boxes().boxForNode(Node(gatId,Node::Gate));
        }
        else if (gSelectionRelay->numberSelectedModules() == 1)
        {
            u32 modId = gSelectionRelay->selectedModulesList().at(0);
            box = mGraphWidget->getContext()->getLayouter()->boxes().boxForNode(Node(modId,Node::Module));
        }
        if (!box) return;
        mItem = box->item();
    }

    void GraphGraphicsView::handleFoldModuleShortcut()
    {
        selectedNodeToItem();
        if (!mItem) return;
        handleFoldParentSingle();
    }

    void GraphGraphicsView::handleUnfoldModuleShortcut()
    {
        selectedNodeToItem();
        if (!mItem || mItem->itemType() != ItemType::Module) return;
        handleUnfoldSingleAction();
    }

    void GraphGraphicsView::handleFoldParentSingle()
    {
        const Module* parentModule = nullptr;
        Node childNode;
        u32 id = mItem->id();
        GraphContext* context = mGraphWidget->getContext();
        switch (mItem->itemType())
        {
        case ItemType::Module:
            parentModule = gNetlist->get_module_by_id(id)->get_parent_module();
            childNode = Node(id, Node::Module);
            break;
        case ItemType::Gate:
            parentModule = gNetlist->get_gate_by_id(id)->get_module();
            childNode = Node(id, Node::Gate);
            break;
        default:
            return;
        }

        if (!parentModule || childNode.type()==Node::None) return;
        NodeBox* box = context->getLayouter()->boxes().boxForNode(childNode);
        if (!box) return;

        PlacementHint plc(PlacementHint::GridPosition);
        plc.addGridPosition(Node(parentModule->get_id(),Node::Module),box->gridPosition());
        ActionFoldModule* act = new ActionFoldModule(parentModule->get_id());
        act->setContextId(context->id());
        act->setPlacementHint(plc);
        act->exec();
    }

    void GraphGraphicsView::handleUnfoldSingleAction()
    {
        GraphContext* context = mGraphWidget->getContext();
        Module* m       = gNetlist->get_module_by_id(mItem->id());
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
        ActionUnfoldModule* act = new ActionUnfoldModule(mItem->id());
        act->setContextId(context->id());
        act->exec();
    }

    void GraphGraphicsView::handleFoldParentAll()
    {
        GraphContext* context = mGraphWidget->getContext();

        QSet<const Module*> modSet;

        for (Node& nd : gSelectionRelay->selectedNodesList())
        {
            if (!context->getLayouter()->boxes().boxForNode(nd)) continue; // not in view

            switch (nd.type())
            {
            case Node::Gate:
                modSet.insert(gNetlist->get_gate_by_id(nd.id())->get_module());
                break;
            case Node::Module:
                modSet.insert(gNetlist->get_module_by_id(nd.id())->get_parent_module());
            default:
                continue;
            }
        }

        QMultiMap<int,const Module*> modDepth;
        for (const Module* m : modSet)
            if (m) modDepth.insertMulti(m->get_submodule_depth(),m);

        QMapIterator<int,const Module*> it(modDepth);
        it.toBack();
        while (it.hasPrevious())
        {
            it.previous();
            const Module* m = it.value();
            ActionFoldModule* act = new ActionFoldModule(m->get_id());
            act->setContextId(context->id());
            act->exec();
        }
    }

    void GraphGraphicsView::handleUnfoldAllAction()
    {
        auto context = mGraphWidget->getContext();

        context->beginChange();
        UserActionCompound* act = new UserActionCompound;
        for (u32 id : gSelectionRelay->selectedModulesList())
        {
            ActionUnfoldModule* ufo = new ActionUnfoldModule(id);
            ufo->setContextId(context->id());
            act->addAction(ufo);
        }
        act->exec();
        context->endChange();
    }

#ifdef GUI_DEBUG_GRID
    void GraphGraphicsView::debugShowLayouterGridpos(const QPoint& mouse_pos)
    {
        auto context = mGraphWidget->getContext();
        if (!context)
            return;

        const GraphLayouter* layouter = context->getLayouter();
        if (!(layouter->done()))
            return;

        QPointF scene_mouse_pos = mapToScene(mouse_pos);
        QPoint layouter_pos     = closestLayouterPos(scene_mouse_pos).first;
        m_debug_gridpos         = layouter_pos;
    }
#endif

    QPair<QPoint,QPointF> GraphGraphicsView::closestLayouterPos(const QPointF& scene_pos) const
    {
        auto context = mGraphWidget->getContext();
        assert(context);

        const GraphLayouter* layouter = context->getLayouter();
        assert(layouter->done());    // ensure grid stable

        int default_width      = layouter->defaultGridWidth();
        int default_height     = layouter->defaultGridHeight();
        int min_x              = layouter->minXIndex();
        int min_y              = layouter->minYIndex();
        QVector<qreal> x_vals  = layouter->xValues();
        QVector<qreal> y_vals  = layouter->yValues();
        LayouterPoint x_point = closestLayouterPoint(scene_pos.x(), default_width, min_x, x_vals);
        LayouterPoint y_point = closestLayouterPoint(scene_pos.y(), default_height, min_y, y_vals);
        return qMakePair(QPoint(x_point.mIndex, y_point.mIndex), QPointF(x_point.mPos, y_point.mPos));
    }

    GraphGraphicsView::LayouterPoint GraphGraphicsView::closestLayouterPoint(qreal scene_pos, int default_spacing, int min_index, QVector<qreal> sections) const
    {
        int index = min_index;
        qreal pos = 0;
        if (sections.first() > scene_pos)
        {
            int distance   = sections.first() - scene_pos;
            int nSections  = distance / default_spacing;    // this rounds down
            index -= (nSections + 1);
            pos = sections.first() + index * default_spacing;
        }
        else if (sections.last() <= scene_pos)
        {
            int distance   = scene_pos - sections.last();
            int nSections  = distance / default_spacing;    // this rounds down
            index += (sections.size() + nSections -1);
            pos = sections.last() + nSections * default_spacing;
        }
        else
        {
            // search for first value in sections larger than or equal to scene_pos
            auto it = sections.constBegin();
            auto jt = it+1;
            while (jt != sections.constEnd())
            {
                if (scene_pos < *jt) break; // found value in inteval [it..[jt
                ++it;
                ++jt;
                ++index;
            }
            pos = *it;
        }
        return LayouterPoint{index, pos};
    }


    GraphicsScene::GridType GraphGraphicsView::gridType()
    {
        return mGridType;
    }

    void GraphGraphicsView::setGridType(GraphicsScene::GridType gridType)
    {
        mGridType = gridType;
    }

    Qt::KeyboardModifier GraphGraphicsView::dragModifier()
    {
        return mDragModifier;
    }

    void GraphGraphicsView::setDragModifier(Qt::KeyboardModifier dragModifier)
    {
        mDragModifier = dragModifier;
    }

    Qt::KeyboardModifier GraphGraphicsView::panModifier()
    {
        return mPanModifier;
    }

    void GraphGraphicsView::setPanModifier(Qt::KeyboardModifier panModifier)
    {
        mPanModifier = panModifier;
    }

    void GraphGraphicsViewNeighborSelector::handleGatesPicked(const QSet<u32>& gats)
    {
        if (!gats.empty())
        {
            u32 pickedGate = *gats.constBegin();
            GraphGraphicsView* ggv = dynamic_cast<GraphGraphicsView*>(parent());
            if (ggv)
            {
                if (mPickSuccessor)
                    ggv->handleShortestPathToGrouping(mOrigin,Node(pickedGate,Node::Gate),true);
                else
                    ggv->handleShortestPathToGrouping(pickedGate,Node(mOrigin,Node::Gate),true);
            }
        }
        this->deleteLater();
    }
}    // namespace hal

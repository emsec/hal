#include "gui/graph_widget/graph_widget.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_layout_spinner_widget.h"
#include "gui/graph_widget/graph_navigation_widget.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/content_manager/content_manager.h"
#include "gui/overlay/widget_overlay.h"
#include "gui/toolbar/toolbar.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

namespace hal
{
    GraphWidget::GraphWidget(GraphContext* context, QWidget* parent)
        : ContentWidget("Graph", parent), mView(new GraphGraphicsView(this)), mContext(context), mOverlay(new WidgetOverlay(this)),
          mNavigationWidgetV3(new GraphNavigationWidget(false)),
          mSpinnerWidget(new GraphLayoutSpinnerWidget(this)), mCurrentExpansion(0)
    {
        connect(mNavigationWidgetV3, &GraphNavigationWidget::navigationRequested, this, &GraphWidget::handleNavigationJumpRequested);
        connect(mNavigationWidgetV3, &GraphNavigationWidget::closeRequested, mOverlay, &WidgetOverlay::hide);
        connect(mNavigationWidgetV3, &GraphNavigationWidget::closeRequested, this, &GraphWidget::resetFocus);

        connect(mOverlay, &WidgetOverlay::clicked, mOverlay, &WidgetOverlay::hide);

        connect(mView, &GraphGraphicsView::moduleDoubleClicked, this, &GraphWidget::handleModuleDoubleClicked);

        mOverlay->hide();
        mOverlay->setWidget(mNavigationWidgetV3);
        mSpinnerWidget->hide();
        mContentLayout->addWidget(mView);

        mView->setFrameStyle(QFrame::NoFrame);
        mView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        mView->setRenderHint(QPainter::Antialiasing, false);
        mView->setDragMode(QGraphicsView::RubberBandDrag);

        mContext->subscribe(this);

        if (!mContext->sceneUpdateInProgress())
        {
            mView->setScene(mContext->scene());
            mView->centerOn(0, 0);
        }
    }

    GraphContext* GraphWidget::getContext() const
    {
        return mContext;
    }

    void GraphWidget::handleSceneAvailable()
    {
        mView->setScene(mContext->scene());

        connect(mOverlay, &WidgetOverlay::clicked, mOverlay, &WidgetOverlay::hide);

        mOverlay->hide();
        mSpinnerWidget->hide();
        mOverlay->setWidget(mNavigationWidgetV3);

        if (hasFocus())
            mView->setFocus();
    }

    void GraphWidget::handleSceneUnavailable()
    {
        mView->setScene(nullptr);

        disconnect(mOverlay, &WidgetOverlay::clicked, mOverlay, &WidgetOverlay::hide);

        mOverlay->setWidget(mSpinnerWidget);

        if (mOverlay->isHidden())
            mOverlay->show();
    }

    void GraphWidget::handleContextAboutToBeDeleted()
    {
        mView->setScene(nullptr);
        mContext = nullptr;
    }

    void GraphWidget::handleStatusUpdate(const int percent)
    {
        Q_UNUSED(percent)
    }

    void GraphWidget::handleStatusUpdate(const QString& message)
    {
        Q_UNUSED(message)
    }

    void GraphWidget::keyPressEvent(QKeyEvent* event)
    {
        if (!mContext)
            return;

        if (mContext->sceneUpdateInProgress())
            return;

        switch (event->key())
        {
            case Qt::Key_Left:
            {
                handleNavigationLeftRequest();
                break;
            }
            case Qt::Key_Right:
            {
                handleNavigationRightRequest();
                break;
            }
            case Qt::Key_Up:
            {
                handleNavigationUpRequest();
                break;
            }
            case Qt::Key_Down:
            {
                handleNavigationDownRequest();
                break;
            }
            case Qt::Key_Z:
            {
                if (event->modifiers() & Qt::ControlModifier)    // modifiers are set as bitmasks
                {
                }
                break;
            }
            case Qt::Key_Escape: {
                gSelectionRelay->clearAndUpdate();
                break;
            }
            default:
                break;
        }
    }

    void GraphWidget::substituteByVisibleModules(const QSet<u32>& gates,
                                                    const QSet<u32>& modules,
                                                    QSet<u32>& target_gates,
                                                    QSet<u32>& target_modules,
                                                    QSet<u32>& remove_gates,
                                                    QSet<u32>& remove_modules) const
    {
        // EXPAND SELECTION AND CONTEXT UP THE HIERARCHY TREE

        for (auto& mid : modules)
        {
            auto m           = gNetlist->get_module_by_id(mid);
            QSet<u32> common = gui_utility::parentModules(m) & mContext->modules();
            if (common.empty())
            {
                // we can select the module
                target_modules.insert(mid);
            }
            else
            {
                // we must select the respective parent module instead
                // (this "common" set only has one element)
                assert(common.size() == 1);
                target_modules += common;
            }
        }

        for (auto& gid : gates)
        {
            auto g           = gNetlist->get_gate_by_id(gid);
            QSet<u32> common = gui_utility::parentModules(g) & mContext->modules();
            if (common.empty())
            {
                target_gates.insert(gid);
            }
            else
            {
                // At this stage, "common" could contain multiple elements because
                // we might have inserted a parent module where its child  module is
                // already mVisible. This is cleaned up later.
                target_modules += common;
            }
        }

        // PRUNE SELECTION AND CONTEXT DOWN THE HIERARCHY TREE

        // discard (and if required schedule for removal) all modules whose
        // parent modules we'll be showing
        QSet<u32> new_module_set = mContext->modules() + target_modules;
        for (auto& mid : mContext->modules())
        {
            auto m = gNetlist->get_module_by_id(mid);
            if (gui_utility::parentModules(m).intersects(new_module_set))
            {
                remove_modules.insert(mid);
            }
        }
        auto it = target_modules.constBegin();
        while (it != target_modules.constEnd())
        {
            auto m = gNetlist->get_module_by_id(*it);
            if (gui_utility::parentModules(m).intersects(new_module_set))
            {
                it = target_modules.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // discard (and if required schedule for removal) all gates whose
        // parent modules we'll be showing
        new_module_set = (mContext->modules() - remove_modules) + target_modules;
        for (auto& gid : mContext->gates())
        {
            auto g = gNetlist->get_gate_by_id(gid);
            if (gui_utility::parentModules(g).intersects(new_module_set))
            {
                remove_gates.insert(gid);
            }
        }
        it = target_gates.constBegin();
        while (it != target_gates.constEnd())
        {
            auto g = gNetlist->get_gate_by_id(*it);
            if (gui_utility::parentModules(g).intersects(new_module_set))
            {
                it = target_gates.erase(it);
            }
            else
            {
                ++it;
            }
        }
        // qDebug() << "-----------";
        // qDebug() << "requested gates" << gates;
        // qDebug() << "requested modules" << modules;
        // qDebug() << "target gates" << target_gates;
        // qDebug() << "target modules" << target_modules;
        // qDebug() << "remove gates" << remove_gates;
        // qDebug() << "remove modules" << remove_modules;
    }

    void GraphWidget::setModifiedIfModule()
    {
        // if our name matches that of a module, add the "modified" label and
        // optionally a number if a "modified"-labeled context already exists
        for (const auto m : gNetlist->get_modules())
        {
            if (m->get_name() == mContext->name().toStdString())
            {
                u32 cnt = 0;
                while (true)
                {
                    ++cnt;
                    QString new_name = mContext->name() + " modified";
                    if (cnt > 1)
                    {
                        new_name += " (" + QString::number(cnt) + ")";
                    }
                    bool found = false;
                    for (const auto& ctx : gGraphContextManager->getContexts())
                    {
                        if (ctx->name() == new_name)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        gGraphContextManager->renameGraphContext(mContext, new_name);
                        break;
                    }
                }
                break;
            }
        }
    }

    void GraphWidget::handleNavigationJumpRequested(const Node &origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules)
    {
        bool bail_animation = false;

        setFocus();

        // ASSERT INPUTS ARE VALID
        auto n = gNetlist->get_net_by_id(via_net);
        if (!n || (to_gates.empty() && to_modules.empty()))
        {
            // prevent stuck navigation widget
            mOverlay->hide();
            mView->setFocus();
            return;
        }

        // Substitute all gates by their modules if we're showing them.
        // This avoids ripping gates out of their already mVisible modules.
        QSet<u32> final_modules, remove_modules;
        QSet<u32> final_gates, remove_gates;
        substituteByVisibleModules(to_gates, to_modules, final_gates, final_modules, remove_gates, remove_modules);

        // find out which gates and modules we still need to add to the context
        // (this makes the cone view work)
        QSet<u32> nonvisible_gates   = final_gates - mContext->gates();
        QSet<u32> nonvisible_modules = final_modules - mContext->modules();

        // if we don't have all gates and modules, we need to add them
        if (!nonvisible_gates.empty() || !nonvisible_modules.empty())
        {
            // display the "modified" label if we're showing a module context
            setModifiedIfModule();

            // hint the layouter at the direction we're navigating in
            // (so the cone view nicely extends to the right or left)
            // either they're all inputs or all outputs, so just check the first one

            std::vector<Net*> in_nets;
            if (to_gates.empty())
            {
                in_nets = gNetlist->get_module_by_id(*to_modules.constBegin())->get_input_nets();
            }
            else
            {
                in_nets = utils::to_vector(gNetlist->get_gate_by_id(*to_gates.begin())->get_fan_in_nets());
            }
            bool netIsInput               = std::find(in_nets.begin(), in_nets.end(), n) != in_nets.cend();
            PlacementHint::PlacementModeType placementMode = netIsInput
                    ? PlacementHint::PreferRight
                    : PlacementHint::PreferLeft;

            // add all new gates and modules
            mContext->beginChange();
            mContext->remove(remove_modules, remove_gates);
            mContext->add(nonvisible_modules, nonvisible_gates, PlacementHint{placementMode, origin});
            mContext->endChange();

            // FIXME find out how to do this properly
            // If we have added any gates, the scene may have resized. In that case, the animation can be erratic,
            // so we set a mFlag here that we can't run the animation. See end of this method for more details.
            bail_animation = true;
        }
        else
        {
            // if we don't need to add anything, we're done here

            mOverlay->hide();
            //if (hasFocus())
            mView->setFocus();
        }

        // SELECT IN RELAY
        gSelectionRelay->clear();
        gSelectionRelay->mSelectedGates   = final_gates;
        gSelectionRelay->mSelectedModules = final_modules;

        // TODO implement subselections on modules, then add a case for when the
        // selection is only one module (instead of one gate)

        if (final_gates.size() == 1 && final_modules.empty())
        {
            // subfocus only possible when just one gate selected
            u32 gid = *final_gates.begin();
            auto g  = gNetlist->get_gate_by_id(gid);

            gSelectionRelay->mFocusType = SelectionRelay::ItemType::Gate;
            gSelectionRelay->mFocusId   = gid;
            gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;

            u32 cnt = 0;
            // TODO simplify (we do actually know if we're navigating left or right)
            for (const auto& pin : g->get_input_pins())
            {
                if (g->get_fan_in_net(pin) == n)    // input net
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Left;
                    gSelectionRelay->mSubfocusIndex = cnt;
                    break;
                }
                cnt++;
            }
            if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::None)
            {
                cnt = 0;
                for (const auto& pin : g->get_output_pins())
                {
                    if (g->get_fan_out_net(pin) == n)    // input net
                    {
                        gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Right;
                        gSelectionRelay->mSubfocusIndex = cnt;
                        break;
                    }
                    cnt++;
                }
            }
        }
        else if (final_modules.size() == 1 && final_gates.empty())
        {
            // subfocus only possible when just one module selected
            u32 mid = *final_modules.begin();
            auto m  = gNetlist->get_module_by_id(mid);

            gSelectionRelay->mFocusType = SelectionRelay::ItemType::Module;
            gSelectionRelay->mFocusId   = mid;
            gSelectionRelay->mSubfocus   = SelectionRelay::Subfocus::None;

            u32 cnt = 0;
            // FIXME this is super hacky because currently we have no way of
            // properly indexing port names on modules (since no order is guaranteed
            // on the port names (different to pin names in gates), but our GUI
            // wants integer indexes)
            // (what we use here is the fact that GraphicsModule builds its port
            // list by traversing m->get_input_nets(), so we just use that order and
            // hope nobody touches that implementation)

            // TODO simplify (we do actually know if we're navigating left or right)
            for (const auto& inet : m->get_input_nets())
            {
                if (inet == n)    // input net
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Left;
                    gSelectionRelay->mSubfocusIndex = cnt;
                    break;
                }
                cnt++;
            }
            if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::None)
            {
                cnt = 0;
                for (const auto& inet : m->get_output_nets())
                {
                    if (inet == n)    // input net
                    {
                        gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Right;
                        gSelectionRelay->mSubfocusIndex = cnt;
                        break;
                    }
                    cnt++;
                }
            }
        }

        gSelectionRelay->relaySelectionChanged(nullptr);

        // FIXME If the scene has been resized during this method, the animation triggered by
        // ensure_gates_visible is broken. Thus, if that is the case, we bail out here and not
        // trigger the animation.
        if (bail_animation)
            return;

        // JUMP TO THE GATES AND MODULES
        ensureItemsVisible(final_gates, final_modules);
    }

    void GraphWidget::handleModuleDoubleClicked(const u32 id)
    {
        // CONNECT DIRECTLY TO HANDLE ???
        // MAYBE ADDITIONAL CODE NECESSARY HERE...
        handleEnterModuleRequested(id);
    }

    // ADD SOUND OR ERROR MESSAGE TO FAILED NAVIGATION ATTEMPTS
    void GraphWidget::handleNavigationLeftRequest()
    {
        const SelectionRelay::Subfocus navigateLeft = SelectionRelay::Subfocus::Left;
        switch (gSelectionRelay->mFocusType)
        {
        case SelectionRelay::ItemType::None: {
                return;
            }
        case SelectionRelay::ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);

                if (!g)
                    return;

                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Left)
                {
                    std::string pin_type = g->get_input_pins()[gSelectionRelay->mSubfocusIndex];
                    Net* n               = g->get_fan_in_net(pin_type);

                    if (!n)
                        return;

                    if (n->get_num_of_sources() == 0)
                    {
                        gSelectionRelay->clear();
                        gSelectionRelay->mSelectedNets.insert(n->get_id());
                        gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                        gSelectionRelay->mFocusId   = n->get_id();
                        gSelectionRelay->relaySelectionChanged(nullptr);
                    }
                    else if (n->get_num_of_sources() == 1)
                    {
                        handleNavigationJumpRequested(Node(g->get_id(),Node::Gate), n->get_id(), {n->get_sources().at(0)->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        mNavigationWidgetV3->setup(navigateLeft);
                        mNavigationWidgetV3->setFocus();
                        mOverlay->show();
                    }
                }
                else if (g->get_input_pins().size())
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Left;
                    gSelectionRelay->mSubfocusIndex = 0;

                    gSelectionRelay->relaySubfocusChanged(nullptr);
                }

                return;
            }
        case SelectionRelay::ItemType::Net: {
                Net* n = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);

                if (!n)
                    return;

                if (n->get_num_of_sources() == 0)
                    return;

                if (n->get_num_of_sources() == 1)
                {
                    handleNavigationJumpRequested(Node(), n->get_id(), {n->get_sources()[0]->get_gate()->get_id()}, {});
                }
                else
                {
                    mNavigationWidgetV3->setup(navigateLeft);
                    mNavigationWidgetV3->setFocus();
                    mOverlay->show();
                }

                return;
            }
        case SelectionRelay::ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(gSelectionRelay->mFocusId);

                if (!m)
                    return;

                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Left)
                {
                    // FIXME this is super hacky because currently we have no way of
                    // properly indexing port names on modules (since no order is guaranteed
                    // on the port names (different to pin names in gates), but our GUI
                    // wants integer indexes)
                    // (what we use here is the fact that GraphicsModule builds its port
                    // list by traversing m->get_input_nets(), so we just use that order and
                    // hope nobody touches that implementation)
                    auto nets = m->get_input_nets();
                    auto it   = nets.begin();
                    if (gSelectionRelay->mSubfocusIndex > 0)
                        std::advance(it, gSelectionRelay->mSubfocusIndex);
                    auto n = *it;
                    if (n->get_num_of_sources() == 0)
                    {
                        gSelectionRelay->clear();
                        gSelectionRelay->mSelectedNets.insert(n->get_id());
                        gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                        gSelectionRelay->mFocusId   = n->get_id();
                        gSelectionRelay->relaySelectionChanged(nullptr);
                    }
                    else if (n->get_num_of_sources() == 1)
                    {
                        handleNavigationJumpRequested(Node(m->get_id(),Node::Module), n->get_id(), {n->get_sources()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        mNavigationWidgetV3->setup(navigateLeft);
                        mNavigationWidgetV3->setFocus();
                        mOverlay->show();
                    }
                }
                else if (m->get_input_nets().size())
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Left;
                    gSelectionRelay->mSubfocusIndex = 0;

                    gSelectionRelay->relaySubfocusChanged(nullptr);
                }

                return;
            }
        }
    }

    void GraphWidget::handleNavigationRightRequest()
    {
        const SelectionRelay::Subfocus navigateRight = SelectionRelay::Subfocus::Right;
        switch (gSelectionRelay->mFocusType)
        {
        case SelectionRelay::ItemType::None:
            {
                return;
            }
        case SelectionRelay::ItemType::Gate:
            {
                Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);

                if (!g)
                    return;

                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Right)
                {
                    auto n = g->get_fan_out_net(g->get_output_pins()[gSelectionRelay->mSubfocusIndex]);
                    if (n->get_num_of_destinations() == 0)
                    {
                        gSelectionRelay->clear();
                        gSelectionRelay->mSelectedNets.insert(n->get_id());
                        gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                        gSelectionRelay->mFocusId   = n->get_id();
                        gSelectionRelay->relaySelectionChanged(nullptr);
                    }
                    else if (n->get_num_of_destinations() == 1)
                    {
                        handleNavigationJumpRequested(Node(g->get_id(),Node::Gate), n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        mNavigationWidgetV3->setup(navigateRight);
                        mNavigationWidgetV3->setFocus();
                        mOverlay->show();
                    }
                }
                else if (g->get_output_pins().size())
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Right;
                    gSelectionRelay->mSubfocusIndex = 0;

                    gSelectionRelay->relaySubfocusChanged(nullptr);
                }

                return;
            }
        case SelectionRelay::ItemType::Net:
            {
                Net* n = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);

                if (!n)
                    return;

                if (n->get_num_of_destinations() == 0)
                    return;

                if (n->get_num_of_destinations() == 1)
                {
                    handleNavigationJumpRequested(Node(), n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                }
                else
                {
                    mNavigationWidgetV3->setup(navigateRight);
                    mNavigationWidgetV3->setFocus();
                    mOverlay->show();
                }

                return;
            }
        case SelectionRelay::ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(gSelectionRelay->mFocusId);

                if (!m)
                    return;

                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Right)
                {
                    // FIXME this is super hacky because currently we have no way of
                    // properly indexing port names on modules (since no order is guaranteed
                    // on the port names (different to pin names in gates), but our GUI
                    // wants integer indexes)
                    // (what we use here is the fact that GraphicsModule builds its port
                    // list by traversing m->get_input_nets(), so we just use that order and
                    // hope nobody touches that implementation)
                    auto nets = m->get_output_nets();
                    auto it   = nets.begin();
                    if (gSelectionRelay->mSubfocusIndex > 0)
                        std::advance(it, gSelectionRelay->mSubfocusIndex);
                    auto n = *it;
                    if (n->get_num_of_destinations() == 0)
                    {
                        gSelectionRelay->clear();
                        gSelectionRelay->mSelectedNets.insert(n->get_id());
                        gSelectionRelay->mFocusType = SelectionRelay::ItemType::Net;
                        gSelectionRelay->mFocusId   = n->get_id();
                        gSelectionRelay->relaySelectionChanged(nullptr);
                    }
                    else if (n->get_num_of_destinations() == 1)
                    {
                        handleNavigationJumpRequested(Node(m->get_id(),Node::Module), n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        mNavigationWidgetV3->setup(navigateRight);
                        mNavigationWidgetV3->setFocus();
                        mOverlay->show();
                    }
                }
                else if (m->get_output_nets().size())
                {
                    gSelectionRelay->mSubfocus       = SelectionRelay::Subfocus::Right;
                    gSelectionRelay->mSubfocusIndex = 0;

                    gSelectionRelay->relaySubfocusChanged(nullptr);
                }
            }
        }
    }

    void GraphWidget::handleNavigationUpRequest()
    {
        // FIXME this is ugly
        if ((gSelectionRelay->mFocusType == SelectionRelay::ItemType::Gate && mContext->gates().contains(gSelectionRelay->mFocusId))
            || (gSelectionRelay->mFocusType == SelectionRelay::ItemType::Module && mContext->modules().contains(gSelectionRelay->mFocusId)))
            gSelectionRelay->navigateUp();
    }

    void GraphWidget::handleNavigationDownRequest()
    {
        // FIXME this is ugly
        if ((gSelectionRelay->mFocusType == SelectionRelay::ItemType::Gate && mContext->gates().contains(gSelectionRelay->mFocusId))
            || (gSelectionRelay->mFocusType == SelectionRelay::ItemType::Module && mContext->modules().contains(gSelectionRelay->mFocusId)))
            gSelectionRelay->navigateDown();
    }

    void GraphWidget::handleEnterModuleRequested(const u32 id)
    {
        auto m = gNetlist->get_module_by_id(id);
        if (m->get_gates().empty() && m->get_submodules().empty())
        {
            QMessageBox msg;
            msg.setText("This module is empty.\nYou can't enter it.");
            msg.setWindowTitle("Error");
            msg.exec();
            return;
            // We would otherwise allow creation of a context with no gates, which
            // is bad because that context won't react to any updates since empty
            // contexts can't infer their corresponding module from their contents
        }

        if (mContext->gates().isEmpty() && mContext->modules() == QSet<u32>({id}))
        {
            mContext->unfoldModule(id);
            return;
        }

        QSet<u32> gate_ids;
        QSet<u32> module_ids;
        for (const auto& g : m->get_gates())
        {
            gate_ids.insert(g->get_id());
        }
        for (auto sm : m->get_submodules())
        {
            module_ids.insert(sm->get_id());
        }

        for (const auto& ctx : gGraphContextManager->getContexts())
        {
            if ((ctx->gates().isEmpty() && ctx->modules() == QSet<u32>({id})) || (ctx->modules() == module_ids && ctx->gates() == gate_ids))
            {
                gContentManager->getGraphTabWidget()->showContext(ctx);
                return;
            }
        }

        auto ctx = gGraphContextManager->createNewContext(QString::fromStdString(m->get_name()));
        ctx->add(module_ids, gate_ids);
    }

    void GraphWidget::ensureItemsVisible(const QSet<u32>& gates, const QSet<u32>& modules)
    {
        if (mContext->sceneUpdateInProgress())
            return;

        int min_x = INT_MAX;
        int min_y = INT_MAX;
        int max_x = INT_MIN;
        int max_y = INT_MIN;

        for (auto id : gates)
        {
            auto rect = mContext->scene()->getGateItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        // TODO clean up redundancy
        for (auto id : modules)
        {
            auto rect = mContext->scene()->getModuleItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        auto targetRect = QRectF(min_x, min_y, max_x - min_x, max_y - min_y).marginsAdded(QMarginsF(20, 20, 20, 20));

        // FIXME This breaks as soon as the layouter call that preceded the call to this function
        // changed the scene size. If that happens, mapToScene thinks that the view is looking at (0,0)
        // and the animation jumps to (0,0) before moving to the correct target.
        auto currentRect = mView->mapToScene(mView->viewport()->geometry()).boundingRect();    // this has incorrect coordinates

        auto centerFix = targetRect.center();
        targetRect.setWidth(std::max(targetRect.width(), currentRect.width()));
        targetRect.setHeight(std::max(targetRect.height(), currentRect.height()));
        targetRect.moveCenter(centerFix);

        //qDebug() << currentRect;

        auto anim = new QVariantAnimation();
        anim->setDuration(1000);
        anim->setStartValue(currentRect);
        anim->setEndValue(targetRect);
        // FIXME fitInView miscalculates the scale required to actually fit the rect into the viewport,
        // so that every time fitInView is called this will cause the scene to scale down by a very tiny amount.
        connect(anim, &QVariantAnimation::valueChanged, [=](const QVariant& value) { mView->fitInView(value.toRectF(), Qt::KeepAspectRatio); });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void GraphWidget::ensureSelectionVisible()
    {
        if (mContext->sceneUpdateInProgress())
            return;

        if (mContext->gates().contains(gSelectionRelay->mSelectedGates) == false || mContext->nets().contains(gSelectionRelay->mSelectedNets) == false
            || mContext->modules().contains(gSelectionRelay->mSelectedModules) == false)
            return;

        int min_x = INT_MAX;
        int min_y = INT_MAX;
        int max_x = INT_MIN;
        int max_y = INT_MIN;

        for (auto id : gSelectionRelay->mSelectedGates)
        {
            auto rect = mContext->scene()->getGateItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        for (auto id : gSelectionRelay->mSelectedNets)
        {
            auto rect = mContext->scene()->getNetItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        for (auto id : gSelectionRelay->mSelectedModules)
        {
            auto rect = mContext->scene()->getModuleItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        auto targetRect = QRectF(min_x, min_y, max_x - min_x, max_y - min_y).marginsAdded(QMarginsF(20, 20, 20, 20));

        // FIXME This breaks as soon as the layouter call that preceded the call to this function
        // changed the scene size. If that happens, mapToScene thinks that the view is looking at (0,0)
        // and the animation jumps to (0,0) before moving to the correct target.
        auto currentRect = mView->mapToScene(mView->viewport()->geometry()).boundingRect();    // this has incorrect coordinates

        auto centerFix = targetRect.center();
        targetRect.setWidth(std::max(targetRect.width(), currentRect.width()));
        targetRect.setHeight(std::max(targetRect.height(), currentRect.height()));
        targetRect.moveCenter(centerFix);

        //qDebug() << currentRect;

        auto anim = new QVariantAnimation();
        anim->setDuration(1000);
        anim->setStartValue(currentRect);
        anim->setEndValue(targetRect);
        // FIXME fitInView miscalculates the scale required to actually fit the rect into the viewport,
        // so that every time fitInView is called this will cause the scene to scale down by a very tiny amount.
        connect(anim, &QVariantAnimation::valueChanged, [=](const QVariant& value) { mView->fitInView(value.toRectF(), Qt::KeepAspectRatio); });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void GraphWidget::resetFocus()
    {
        mView->setFocus();
    }

    GraphGraphicsView* GraphWidget::view()
    {
        return mView;
    }
}    // namespace hal

#include "gui/graph_widget/graph_navigation_widget_v2.h"

#include "hal_core/utilities/log.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTreeWidgetItem>

#include <assert.h>
#include <QDebug>

namespace hal
{
    GraphNavigationWidgetV2::GraphNavigationWidgetV2(QWidget* parent) : QTreeWidget(parent)
    {
        setSelectionMode(QAbstractItemView::MultiSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        header()->setStretchLastSection(false);
        setColumnCount(5);
        setHeaderLabels({"Name", "ID", "Type", "Pin", "Parent Module"});
        setAllColumnsShowFocus(true);

        connect(this, &GraphNavigationWidgetV2::itemSelectionChanged, this, &GraphNavigationWidgetV2::handleSelectionChanged);
        connect(this, &GraphNavigationWidgetV2::itemDoubleClicked, this, &GraphNavigationWidgetV2::handleItemDoubleClicked);

        // FIXME for some reason, arrow key navigation does not work on MacOS
    }

    void GraphNavigationWidgetV2::setup(bool direction)
    {
        clear();

        switch (gSelectionRelay->mFocusType)
        {
        case SelectionRelay::ItemType::None:
            {
                return;
            }
        case SelectionRelay::ItemType::Gate:
            {
                Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);

                assert(g);

                std::string pin        = (direction ? g->get_output_pins() : g->get_input_pins())[gSelectionRelay->mSubfocusIndex];
                Net* n = (direction ? g->get_fan_out_net(pin) : g->get_fan_in_net(pin));

                assert(n);

                mOrigin = Node(g->get_id(), Node::Gate);
                mViaNet = n;

                fillTable(direction);

                return;
            }
        case SelectionRelay::ItemType::Net:
            {
                Net* n = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);

                assert(n);
                assert(direction ? n->get_num_of_destinations() : n->get_num_of_sources());

                mOrigin = Node();
                mViaNet = n;

                fillTable(direction);

                return;
            }
        case SelectionRelay::ItemType::Module:
            {
                Module* m = gNetlist->get_module_by_id(gSelectionRelay->mFocusId);

                assert(m);

                // FIXME this is super hacky because currently we have no way of
                // properly indexing port names on modules (since no order is guaranteed
                // on the port names (different to pin names in gates), but our GUI
                // wants integer indexes)
                // (what we use here is the fact that GraphicsModule builds its port
                // list by traversing m->get_input_nets(), so we just use that order and
                // hope nobody touches that implementation)

                std::vector<Net*> nets = m->get_output_nets();
                auto it = nets.begin();
                if (gSelectionRelay->mSubfocusIndex > 0)
                    std::advance(it, gSelectionRelay->mSubfocusIndex);
                auto n = *it;

                assert(n);

                mOrigin = Node(m->get_id(),Node::Module);
                mViaNet = n;

                fillTable(direction);
                return;

            }
        }
    }

    void GraphNavigationWidgetV2::setup(Node origin, Net* via_net, bool direction)
    {
        clear();
        fillTable(direction);
        mViaNet = via_net;
        mOrigin = origin;
    }

    void GraphNavigationWidgetV2::keyPressEvent(QKeyEvent* event)
    {
        // qDebug() << "KeyDebug:" << "dn:" << (event->key() == Qt::Key_Down) << "/ up:" << (event->key() == Qt::Key_Up);
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Right)
        {
            commitSelection();
            mViaNet = nullptr;
            return;
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            Q_EMIT closeRequested();
            Q_EMIT resetFocus();
            mViaNet = nullptr;
            return;
        }

        return QTreeWidget::keyPressEvent(event);
    }

    void GraphNavigationWidgetV2::fillTable(bool direction)
    {
        int row = 0;
        QMap<u32, QTreeWidgetItem*> created_parents;

        // iterate over all sources, respective destinations, of the via net
        for (Endpoint* e : (direction ? mViaNet->get_destinations() : mViaNet->get_sources()))
        {
            Gate* g = e->get_gate();
            if (!g)
            {
                // skip non-gate endpoints
                continue;
            }

            // find the highest module in the hierarchy that we should offer for this
            // gate (if we have a module A that contains both the origin and the target
            // gate, then we don't want to offer navigating to that module or any
            // modules further up the hierarchy)
            Module* common_ancestor = nullptr; //fixes uninit warning
            if (mOrigin.id() == 0)
            {
                // we're navigating from a net
                if (mViaNet->is_global_input_net() || mViaNet->is_global_output_net())
                {
                    // in this special case, the net is actually _outside_ the
                    // top module, so we can't do the common ancestor approach
                    common_ancestor = nullptr;
                }
                else
                {
                    // in this case we don't really know how to limit our view,
                    // so we look for the common ancestor of all sources and sinks of
                    // this net and use that
                    auto net_sources = mViaNet->get_sources();
                    auto net_destinations = mViaNet->get_destinations();
                    std::unordered_set<Gate*> net_gates;
                    for (auto ep : net_sources)
                        net_gates.insert(ep->get_gate());
                    for (auto ep : net_destinations)
                        net_gates.insert(ep->get_gate());
                    common_ancestor = gui_utility::firstCommonAncestor({}, net_gates);
                }
            }
            else if (mOrigin.type() == Node::Gate)
            {
                Gate* origin = gNetlist->get_gate_by_id(mOrigin.id());
                assert(origin);
                common_ancestor = gui_utility::firstCommonAncestor({}, {origin, g});
            }
            else if (mOrigin.type() == Node::Module)
            {
                Module* origin = gNetlist->get_module_by_id(mOrigin.id());
                assert(origin);
                common_ancestor = gui_utility::firstCommonAncestor({origin}, {g});
            }
            else
            {
                // malformed node type
                assert(false);
            }

            // make item for the gate
            QTreeWidgetItem* item = new QTreeWidgetItem({
                QString::fromStdString(g->get_name()),
                QString::number(g->get_id()),
                QString::fromStdString(g->get_type()->get_name()),
                QString::fromStdString(e->get_pin()),
                QString::fromStdString(g->get_module()->get_name())
            });
            // TODO is there a better way?
            item->setData(1, Qt::ItemDataRole::UserRole, g->get_id());

            // recurse up the hierarchy until one prior to the common ancestor and
            // create entries for the parent gates, reusing ones that have already
            // been created for other gates
            // (if we're navigating from a global in/out net, then common_ancestor
            // is nullptr, so we stop instead when we run out of parents)
            Module* parent = g->get_module();
            bool reused_item = false;
            while(parent != common_ancestor) {
                // qDebug() << QString::fromStdString(parent->get_name());
                QTreeWidgetItem* parent_item;
                auto it = created_parents.find(parent->get_id());
                if (it != created_parents.end())
                {
                    // reuse the existing item so we can append more child items to it
                    // instead of duplicating the entry for the parent
                    parent_item = *it;
                    reused_item = true;
                }
                else
                {
                    QString portname = QString::fromStdString(direction ?
                        parent->get_input_port_name(mViaNet) :
                        parent->get_output_port_name(mViaNet)
                    );
                    QString type = QString::fromStdString(parent->get_type());
                    if (type.isEmpty())
                    {
                        type = "<empty-type>";
                    }
                    type+= " Module";
                    auto parents_parent = parent->get_parent_module();
                    QString parents_parent_name = QString::fromStdString(parents_parent ? parents_parent->get_name() : "(none)");
                    // lazy-init the item for this module if it's not in the map yet
                    parent_item = new QTreeWidgetItem({
                        QString::fromStdString(parent->get_name()),
                        QString::number(parent->get_id()),
                        type,
                        portname,
                        parents_parent_name
                    });
                    // TODO is there a better way?
                    parent_item->setData(1, Qt::ItemDataRole::UserRole, parent->get_id());

                    created_parents.insert(parent->get_id(), parent_item);
                    reused_item = false;
                }
                parent_item->addChild(item);
                parent = parent->get_parent_module();
                item = parent_item;
                // we'll always hit the common ancestor before we run out of parents,
                // except if we're looking at a global input/output net
                assert(parent || (common_ancestor == nullptr));
            }
            if (!reused_item)
            {
                // insert the top level item into the view if we haven't already done so
                // in an earlier iteration
                insertTopLevelItem(row++, item);
            }
        }
        expandAll();
        resizeToFit();
    }

    void GraphNavigationWidgetV2::resizeToFit()
    {
        // Qt apparently needs these 2 pixels extra, otherwise you get scollbars

        int width = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 2;
        for (int i = 0; i < columnCount(); i++)
        {
            resizeColumnToContents(i);
            width += columnWidth(i);
        }

        int height = header()->height() + 2;
        height += sumRowHeights(this->invisibleRootItem());

        int MAXIMUM_ALLOWED_HEIGHT = std::min(500, static_cast<QWidget*>(parent())->height());
        int MAXIMUM_ALLOWED_WIDTH = std::min(700, static_cast<QWidget*>(parent())->width());
        setFixedWidth((width > MAXIMUM_ALLOWED_WIDTH) ? MAXIMUM_ALLOWED_WIDTH : width);
        setFixedHeight((height > MAXIMUM_ALLOWED_HEIGHT) ? MAXIMUM_ALLOWED_HEIGHT : height);
    }

    int GraphNavigationWidgetV2::sumRowHeights(const QTreeWidgetItem *itm, bool top)
    {
        int row_heights = 0;
        if (!top)
             row_heights = visualItemRect(itm).height();
        for(int i = 0; i < itm->childCount(); i++)
            row_heights += sumRowHeights(itm->child(i), false);
        return row_heights;
    }

    void GraphNavigationWidgetV2::handleItemDoubleClicked(QTreeWidgetItem* item)
    {
        Q_UNUSED(item)
        commitSelection();
    }

    void GraphNavigationWidgetV2::commitSelection()
    {
        if (selectedItems().isEmpty())
        {
            // if we don't have a selection, fake a selection on the item that is
            // currently focused (this allows navigating to a single gate with just
            // the arrow keys)
            setItemSelected(currentItem(), true);
        }

        QSet<u32> target_gates;
        QSet<u32> target_modules;

        for (auto i : selectedItems())
        {
            u32 id = i->data(1, Qt::ItemDataRole::UserRole).toUInt();
            // TODO this is evil
            bool isModule = i->childCount() > 0;
            if (isModule)
            {
                target_modules.insert(id);
            }
            else
            {
                target_gates.insert(id);
            }
        }

        Q_EMIT navigationRequested(mOrigin, mViaNet->get_id(), target_gates, target_modules);
    }

    void GraphNavigationWidgetV2::handleSelectionChanged()
    {
        auto new_selection = selectedItems().toSet();
        auto deselected = mPreviousSelection - new_selection;
        auto selected = new_selection - mPreviousSelection;
        for (auto& itm : deselected)
        {
            int count = itm->childCount();
            for (int i = 0; i < count; i++)
            {
                // TODO style this on "sunny"
                itm->child(i)->setDisabled(false);
            }
        }
        for (auto& itm : selected)
        {
            int count = itm->childCount();
            for (int i = 0; i < count; i++)
            {
                itm->child(i)->setDisabled(true);
                itm->child(i)->setSelected(false);
            }
        }
        mPreviousSelection = new_selection;
    }
}

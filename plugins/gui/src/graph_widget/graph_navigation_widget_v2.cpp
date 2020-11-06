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

        connect(this, &GraphNavigationWidgetV2::itemSelectionChanged, this, &GraphNavigationWidgetV2::handle_selection_changed);
        connect(this, &GraphNavigationWidgetV2::itemDoubleClicked, this, &GraphNavigationWidgetV2::handle_item_double_clicked);

        // FIXME for some reason, arrow key navigation does not work on MacOS
    }

    void GraphNavigationWidgetV2::setup(bool direction)
    {
        clear();

        switch (g_selection_relay->m_focus_type)
        {
            case SelectionRelay::item_type::none:
            {
                return;
            }
            case SelectionRelay::item_type::gate:
            {
                Gate* g = g_netlist->get_gate_by_id(g_selection_relay->m_focus_id);

                assert(g);

                std::string pin        = (direction ? g->get_output_pins() : g->get_input_pins())[g_selection_relay->m_subfocus_index];
                Net* n = (direction ? g->get_fan_out_net(pin) : g->get_fan_in_net(pin));

                assert(n);

                m_origin = Node(g->get_id(), Node::Gate);
                m_via_net = n;

                fill_table(direction);

                return;
            }
            case SelectionRelay::item_type::net:
            {
                Net* n = g_netlist->get_net_by_id(g_selection_relay->m_focus_id);

                assert(n);
                assert(direction ? n->get_num_of_destinations() : n->get_num_of_sources());

                m_origin = Node();
                m_via_net = n;

                fill_table(direction);

                return;
            }
            case SelectionRelay::item_type::module:
            {
                Module* m = g_netlist->get_module_by_id(g_selection_relay->m_focus_id);

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
                if (g_selection_relay->m_subfocus_index > 0)
                    std::advance(it, g_selection_relay->m_subfocus_index);
                auto n = *it;

                assert(n);

                m_origin = Node(m->get_id(),Node::Module);
                m_via_net = n;

                fill_table(direction);
                return;

            }
        }
    }

    void GraphNavigationWidgetV2::setup(Node origin, Net* via_net, bool direction)
    {
        clear();
        fill_table(direction);
        m_via_net = via_net;
        m_origin = origin;
    }

    void GraphNavigationWidgetV2::keyPressEvent(QKeyEvent* event)
    {
        // qDebug() << "KeyDebug:" << "dn:" << (event->key() == Qt::Key_Down) << "/ up:" << (event->key() == Qt::Key_Up);
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Right)
        {
            commit_selection();
            m_via_net = nullptr;
            return;
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            Q_EMIT close_requested();
            Q_EMIT reset_focus();
            m_via_net = nullptr;
            return;
        }

        return QTreeWidget::keyPressEvent(event);
    }

    void GraphNavigationWidgetV2::fill_table(bool direction)
    {
        int row = 0;
        QMap<u32, QTreeWidgetItem*> created_parents;

        // iterate over all sources, respective destinations, of the via net
        for (Endpoint* e : (direction ? m_via_net->get_destinations() : m_via_net->get_sources()))
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
            if (m_origin.id() == 0)
            {
                // we're navigating from a net
                if (m_via_net->is_global_input_net() || m_via_net->is_global_output_net())
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
                    auto net_sources = m_via_net->get_sources();
                    auto net_destinations = m_via_net->get_destinations();
                    std::unordered_set<Gate*> net_gates;
                    for (auto ep : net_sources)
                        net_gates.insert(ep->get_gate());
                    for (auto ep : net_destinations)
                        net_gates.insert(ep->get_gate());
                    common_ancestor = gui_utility::first_common_ancestor({}, net_gates);
                }
            }
            else if (m_origin.type() == Node::Gate)
            {
                Gate* origin = g_netlist->get_gate_by_id(m_origin.id());
                assert(origin);
                common_ancestor = gui_utility::first_common_ancestor({}, {origin, g});
            }
            else if (m_origin.type() == Node::Module)
            {
                Module* origin = g_netlist->get_module_by_id(m_origin.id());
                assert(origin);
                common_ancestor = gui_utility::first_common_ancestor({origin}, {g});
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
                        parent->get_input_port_name(m_via_net) :
                        parent->get_output_port_name(m_via_net)
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
        resize_to_fit();
    }

    void GraphNavigationWidgetV2::resize_to_fit()
    {
        // Qt apparently needs these 2 pixels extra, otherwise you get scollbars

        int width = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 2;
        for (int i = 0; i < columnCount(); i++)
        {
            resizeColumnToContents(i);
            width += columnWidth(i);
        }

        int height = header()->height() + 2;
        height += sum_row_heights(this->invisibleRootItem());

        int MAXIMUM_ALLOWED_HEIGHT = std::min(500, static_cast<QWidget*>(parent())->height());
        int MAXIMUM_ALLOWED_WIDTH = std::min(700, static_cast<QWidget*>(parent())->width());
        setFixedWidth((width > MAXIMUM_ALLOWED_WIDTH) ? MAXIMUM_ALLOWED_WIDTH : width);
        setFixedHeight((height > MAXIMUM_ALLOWED_HEIGHT) ? MAXIMUM_ALLOWED_HEIGHT : height);
    }

    int GraphNavigationWidgetV2::sum_row_heights(const QTreeWidgetItem *itm, bool top)
    {
        int row_heights = 0;
        if (!top)
             row_heights = visualItemRect(itm).height();
        for(int i = 0; i < itm->childCount(); i++)
            row_heights += sum_row_heights(itm->child(i), false);
        return row_heights;
    }

    void GraphNavigationWidgetV2::handle_item_double_clicked(QTreeWidgetItem* item)
    {
        Q_UNUSED(item)
        commit_selection();
    }

    void GraphNavigationWidgetV2::commit_selection()
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

        Q_EMIT navigation_requested(m_origin, m_via_net->get_id(), target_gates, target_modules);
    }

    void GraphNavigationWidgetV2::handle_selection_changed()
    {
        auto new_selection = selectedItems().toSet();
        auto deselected = m_previous_selection - new_selection;
        auto selected = new_selection - m_previous_selection;
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
        m_previous_selection = new_selection;
    }
}

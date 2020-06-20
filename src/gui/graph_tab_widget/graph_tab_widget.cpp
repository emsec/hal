#include "graph_tab_widget/graph_tab_widget.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"

#include "gui_globals.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>

namespace hal
{
    GraphTabWidget::GraphTabWidget(QWidget* parent) : ContentWidget("Graph-Views", parent), m_tab_widget(new QTabWidget()), m_layout(new QVBoxLayout()), m_zoom_factor(1.2)
    {
        m_content_layout->addWidget(m_tab_widget);
        m_tab_widget->setTabsClosable(true);
        m_tab_widget->setMovable(true);

        connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &GraphTabWidget::handle_tab_close_requested);
        connect(m_tab_widget, &QTabWidget::currentChanged, this, &GraphTabWidget::handle_tab_changed);
        connect(&g_graph_context_manager, &GraphContextManager::context_created, this, &GraphTabWidget::handle_context_created);
        connect(&g_graph_context_manager, &GraphContextManager::context_renamed, this, &GraphTabWidget::handle_context_renamed);
        connect(&g_graph_context_manager, &GraphContextManager::deleting_context, this, &GraphTabWidget::handle_context_removed);
    }

    QList<QShortcut *> GraphTabWidget::create_shortcuts()
    {
        QShortcut* zoom_in_sc = g_keybind_manager.make_shortcut(this, "keybinds/graph_view_zoom_in");
        connect(zoom_in_sc, &QShortcut::activated, this, &GraphTabWidget::zoom_in_shortcut);

        QShortcut* zoom_out_sc = g_keybind_manager.make_shortcut(this, "keybinds/graph_view_zoom_out");
        connect(zoom_out_sc, &QShortcut::activated, this, &GraphTabWidget::zoom_out_shortcut);

        QList<QShortcut*> list;
        list.append(zoom_in_sc);
        list.append(zoom_out_sc);

        return list;
    }

    int GraphTabWidget::addTab(QWidget* tab, QString name)
    {
        int tab_index = m_tab_widget->addTab(tab, name);
        return tab_index;
    }

    void GraphTabWidget::handle_tab_changed(int index)
    {
        auto w = dynamic_cast<GraphWidget*>(m_tab_widget->widget(index));
        if (w)
        {
            auto ctx = w->get_context();
            g_content_manager->get_context_manager_widget()->select_view_context(ctx);

            disconnect(g_gui_api, &GuiApi::navigation_requested, m_current_widget, &GraphWidget::ensure_selection_visible);
            connect(g_gui_api, &GuiApi::navigation_requested, w, &GraphWidget::ensure_selection_visible);
            m_current_widget = w;
        }
    }

    void GraphTabWidget::handle_tab_close_requested(int index)
    {
        auto w = dynamic_cast<GraphWidget*>(m_tab_widget->widget(index));
        disconnect(g_gui_api, &GuiApi::navigation_requested, w, &GraphWidget::ensure_selection_visible);
        m_tab_widget->removeTab(index);

        //right way to do it??
        //GraphWidget* graph_wid = dynamic_cast<GraphWidget*>(m_tab_widget->widget(index));
        //GraphContext* dyn_con = g_graph_context_manager.get_context_by_name(m_tab_widget->tabText(index));
        //dyn_con->unsubscribe(graph_wid);
    }

    void GraphTabWidget::show_context(GraphContext* context)
    {
        auto index = get_context_tab_index(context);
        if (index != -1)
        {
            m_tab_widget->setCurrentIndex(index);
            m_tab_widget->widget(index)->setFocus();
            return;
        }

        add_graph_widget_tab(context);
    }

    void GraphTabWidget::handle_context_created(GraphContext* context)
    {
        add_graph_widget_tab(context);
    }

    void GraphTabWidget::handle_context_renamed(GraphContext* context)
    {
        m_tab_widget->setTabText(get_context_tab_index(context), context->name());
    }

    void GraphTabWidget::handle_context_removed(GraphContext* context)
    {
        handle_tab_close_requested(get_context_tab_index(context));
    }

    void GraphTabWidget::add_graph_widget_tab(GraphContext* context)
    {
        GraphWidget* new_graph_widget = new GraphWidget(context);
        //m_context_widget_map.insert(context, new_graph_widget);

        if(m_tab_widget->count() == 0)
        {
            connect(g_gui_api, &GuiApi::navigation_requested, new_graph_widget, &GraphWidget::ensure_selection_visible);
            m_current_widget = new_graph_widget;
        }

        int tab_index = addTab(new_graph_widget, context->name());
        m_tab_widget->setCurrentIndex(tab_index);
        m_tab_widget->widget(tab_index)->setFocus();
        context->schedule_scene_update();
    }

    void GraphTabWidget::zoom_in_shortcut()
    {
        GraphWidget* w = dynamic_cast<GraphWidget*>(m_tab_widget->currentWidget());
        if(w)
            w->view()->viewport_center_zoom(m_zoom_factor);
    }

    void GraphTabWidget::zoom_out_shortcut()
    {
        GraphWidget* w = dynamic_cast<GraphWidget*>(m_tab_widget->currentWidget());
        if(w)
            w->view()->viewport_center_zoom(1.0 / m_zoom_factor);

    }

    int GraphTabWidget::get_context_tab_index(GraphContext* context) const
    {
        for (int i = 0; i < m_tab_widget->count(); i++)
        {
            if (auto p = dynamic_cast<GraphWidget*>(m_tab_widget->widget(i)))
            {
                if (p->get_context() == context)
                {
                    return i;
                }
            }
        }
        return -1;
    }
}

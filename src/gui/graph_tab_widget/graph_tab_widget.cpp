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
namespace hal{
graph_tab_widget::graph_tab_widget(QWidget* parent) : content_widget("Graph-Views", parent), m_tab_widget(new QTabWidget()), m_layout(new QVBoxLayout()), m_zoom_factor(1.2)
{
    m_content_layout->addWidget(m_tab_widget);
    m_tab_widget->setTabsClosable(true);
    m_tab_widget->setMovable(true);

    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &graph_tab_widget::handle_tab_close_requested);
    connect(m_tab_widget, &QTabWidget::currentChanged, this, &graph_tab_widget::handle_tab_changed);
    connect(&g_graph_context_manager, &graph_context_manager::context_created, this, &graph_tab_widget::handle_context_created);
    connect(&g_graph_context_manager, &graph_context_manager::context_renamed, this, &graph_tab_widget::handle_context_renamed);
    connect(&g_graph_context_manager, &graph_context_manager::deleting_context, this, &graph_tab_widget::handle_context_removed);
}

QList<QShortcut *> graph_tab_widget::create_shortcuts()
{
    QShortcut* zoom_in_sc = g_keybind_manager.make_shortcut(this, "keybinds/graph_view_zoom_in");
    connect(zoom_in_sc, &QShortcut::activated, this, &graph_tab_widget::zoom_in_shortcut);

    QShortcut* zoom_out_sc = g_keybind_manager.make_shortcut(this, "keybinds/graph_view_zoom_out");
    connect(zoom_out_sc, &QShortcut::activated, this, &graph_tab_widget::zoom_out_shortcut);

    QList<QShortcut*> list;
    list.append(zoom_in_sc);
    list.append(zoom_out_sc);

    return list;
}

int graph_tab_widget::addTab(QWidget* tab, QString name)
{
    int tab_index = m_tab_widget->addTab(tab, name);
    return tab_index;
}

void graph_tab_widget::handle_tab_changed(int index)
{
    auto w = dynamic_cast<graph_widget*>(m_tab_widget->widget(index));
    if (w)
    {
        auto ctx = w->get_context();
        g_content_manager->get_context_manager_widget()->select_view_context(ctx);

        disconnect(g_gui_api, &gui_api::navigation_requested, m_current_widget, &graph_widget::ensure_selection_visible);
        connect(g_gui_api, &gui_api::navigation_requested, w, &graph_widget::ensure_selection_visible);
        m_current_widget = w;
    }
}

void graph_tab_widget::handle_tab_close_requested(int index)
{
    auto w = dynamic_cast<graph_widget*>(m_tab_widget->widget(index));
    disconnect(g_gui_api, &gui_api::navigation_requested, w, &graph_widget::ensure_selection_visible);
    m_tab_widget->removeTab(index);

    //right way to do it??
    //graph_widget* graph_wid = dynamic_cast<graph_widget*>(m_tab_widget->widget(index));
    //graph_context* dyn_con = g_graph_context_manager.get_context_by_name(m_tab_widget->tabText(index));
    //dyn_con->unsubscribe(graph_wid);
}

void graph_tab_widget::show_context(graph_context* context)
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

void graph_tab_widget::handle_context_created(graph_context* context)
{
    add_graph_widget_tab(context);
}

void graph_tab_widget::handle_context_renamed(graph_context* context)
{
    m_tab_widget->setTabText(get_context_tab_index(context), context->name());
}

void graph_tab_widget::handle_context_removed(graph_context* context)
{
    handle_tab_close_requested(get_context_tab_index(context));
}

void graph_tab_widget::add_graph_widget_tab(graph_context* context)
{
    graph_widget* new_graph_widget = new graph_widget(context);
    //m_context_widget_map.insert(context, new_graph_widget);
    
    if(m_tab_widget->count() == 0)
    {
        connect(g_gui_api, &gui_api::navigation_requested, new_graph_widget, &graph_widget::ensure_selection_visible);
        m_current_widget = new_graph_widget;
    }
    
    int tab_index = addTab(new_graph_widget, context->name());
    m_tab_widget->setCurrentIndex(tab_index);
    m_tab_widget->widget(tab_index)->setFocus();
    context->schedule_scene_update();
}

void graph_tab_widget::zoom_in_shortcut()
{
    graph_widget* w = dynamic_cast<graph_widget*>(m_tab_widget->currentWidget());
    if(w)
        w->view()->viewport_center_zoom(m_zoom_factor);
}

void graph_tab_widget::zoom_out_shortcut()
{
    graph_widget* w = dynamic_cast<graph_widget*>(m_tab_widget->currentWidget());
    if(w)
        w->view()->viewport_center_zoom(1.0 / m_zoom_factor);

}

int graph_tab_widget::get_context_tab_index(graph_context* context) const
{
    for (int i = 0; i < m_tab_widget->count(); i++)
    {
        if (auto p = dynamic_cast<graph_widget*>(m_tab_widget->widget(i)))
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

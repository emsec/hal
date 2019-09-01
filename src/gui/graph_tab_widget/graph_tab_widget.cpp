#include "graph_tab_widget/graph_tab_widget.h"

#include "gui/graph_widget/graph_widget.h"

#include "gui_globals.h"

#include <QTabWidget>
#include <QVBoxLayout>

#include <QDebug>

graph_tab_widget::graph_tab_widget(QWidget* parent) : content_widget("Graph-Views", parent), m_layout(new QVBoxLayout()), m_tab_widget(new QTabWidget())
{
    m_content_layout->addWidget(m_tab_widget);
    m_tab_widget->setTabsClosable(true);

    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &graph_tab_widget::handle_tab_close_requested);
    connect(&g_graph_context_manager, &graph_context_manager::context_created, this, &graph_tab_widget::handle_context_created);
    connect(&g_graph_context_manager, &graph_context_manager::context_renamed, this, &graph_tab_widget::handle_context_renamed);
    connect(&g_graph_context_manager, &graph_context_manager::context_removed, this, &graph_tab_widget::handle_context_removed);
}

int graph_tab_widget::addTab(QWidget* tab, QString name)
{
    int tab_index = m_tab_widget->addTab(tab, name);
    return tab_index;
}

void graph_tab_widget::handle_tab_close_requested(int index)
{
    m_tab_widget->removeTab(index);

    //right way to do it??
    //graph_widget* graph_wid = dynamic_cast<graph_widget*>(m_tab_widget->widget(index));
    //dynamic_context* dyn_con = g_graph_context_manager.get_dynamic_context(m_tab_widget->tabText(index));
    //dyn_con->unsubscribe(graph_wid);
}

void graph_tab_widget::show_context(dynamic_context* context)
{
    auto index = get_context_tab_index(context);
    if (index != -1)
    {
        m_tab_widget->setCurrentIndex(index);
        m_tab_widget->widget(index)->setFocus();
        return;
    }

    add_graph_widget_tab(g_graph_context_manager.get_dynamic_context(context->name()));
}

void graph_tab_widget::handle_context_created(dynamic_context* context)
{
    add_graph_widget_tab(context);
}

void graph_tab_widget::handle_context_renamed(dynamic_context* context)
{
    m_tab_widget->setTabText(get_context_tab_index(context), context->name());
}

void graph_tab_widget::handle_context_removed(dynamic_context* context)
{
    handle_tab_close_requested(get_context_tab_index(context));
}

void graph_tab_widget::add_graph_widget_tab(dynamic_context* context)
{
    graph_widget* new_graph_widget = new graph_widget(context);
    //m_context_widget_map.insert(context, new_graph_widget);
    int tab_index = addTab(new_graph_widget, context->name());
    m_tab_widget->setCurrentIndex(tab_index);
    m_tab_widget->widget(tab_index)->setFocus();
    context->update();
}

int graph_tab_widget::get_context_tab_index(dynamic_context* context) const
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

#include "graph_tab_widget/graph_tab_widget.h"

#include "gui/graph_widget/graph_widget.h"

#include "gui_globals.h"

#include <QTabWidget>
#include <QVBoxLayout>

#include <QDebug>

graph_tab_widget::graph_tab_widget(QWidget* parent) : content_widget("Graph-Views",parent), m_layout(new QVBoxLayout()), m_tab_widget(new QTabWidget())
{
    m_content_layout->addWidget(m_tab_widget);
    m_tab_widget->setTabsClosable(true);

    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &graph_tab_widget::handle_tab_close_requested);
}


int graph_tab_widget::addTab(QWidget *tab, QString name)
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

void graph_tab_widget::handle_context_created(dynamic_context* context)
{
    add_graph_widget_tab(context);
}

void graph_tab_widget::handle_context_open_request(dynamic_context* context)
{

    for(int i = 0; i < m_tab_widget->count(); i++)
    {
        if(m_tab_widget->tabText(i) == context->name())
        {
            qDebug() << "EXISTING";
            m_tab_widget->setCurrentIndex(i);
            return;
        } 
    }

    qDebug() << "NOT EXISTING";
    add_graph_widget_tab(g_graph_context_manager.get_dynamic_context(context->name()));

    /*
    //check if context is already open in a tab -> true = focus / false = create new tab
    QMap<dynamic_context*, QWidget*>::const_iterator dynamic_context_iterator = m_context_widget_map.find(context);

    if(dynamic_context_iterator != m_context_widget_map.end())
    {
        m_tab_widget->setCurrentIndex(m_tab_widget->indexOf(*dynamic_context_iterator));
    }
    else
    {
        add_graph_widget_tab(context, context_name);
    }
    */
}

void graph_tab_widget::add_graph_widget_tab(dynamic_context* context)
{
    graph_widget* new_graph_widget = new graph_widget(context);
    //m_context_widget_map.insert(context, new_graph_widget);
    int tab_index = addTab(new_graph_widget, context->name());
    m_tab_widget->setCurrentIndex(tab_index);
}

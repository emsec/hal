#include "graph_tab_widget/graph_tab_widget.h"

#include "gui/graph_widget/graph_widget.h"

#include <QTabWidget>
#include <QVBoxLayout>

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
}

void graph_tab_widget::handle_context_created(dynamic_context* context, QString context_name)
{
    graph_widget* new_graph_widget = new graph_widget(context);
    //new_graph_widget->change_context(context);

    int tab_index = addTab(new_graph_widget, context_name);
    m_tab_widget->setCurrentIndex(tab_index);
}
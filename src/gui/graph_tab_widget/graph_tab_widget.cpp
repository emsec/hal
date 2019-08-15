#include "graph_tab_widget/graph_tab_widget.h"
#include <QTabWidget>
#include <QVBoxLayout>

graph_tab_widget::graph_tab_widget(QWidget* parent) : content_widget("Graph-Views",parent), m_layout(new QVBoxLayout()), m_tab_widget(new QTabWidget())
{
    m_content_layout->addWidget(m_tab_widget);
    m_tab_widget->setTabsClosable(true);

    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &graph_tab_widget::handle_tab_close_requested);
}

void graph_tab_widget::addTab(QWidget *tab, QString name)
{
    m_tab_widget->addTab(tab, name);
}

void graph_tab_widget::handle_tab_close_requested(int index)
{
    m_tab_widget->removeTab(index);
}

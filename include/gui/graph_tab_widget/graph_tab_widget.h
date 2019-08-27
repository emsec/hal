#ifndef GRAPH_TAB_WIDGET
#define GRAPH_TAB_WIDGET

#include "content_widget/content_widget.h"

#include "graph_widget/contexts/dynamic_context.h"

#include <QMap>

class QTabWidget;
class QVBoxLayout;

class graph_tab_widget : public content_widget
{
    Q_OBJECT

public:
    graph_tab_widget(QWidget* parent = nullptr);

    int addTab(QWidget* tab, QString tab_name = "default");

    void show_context(dynamic_context* context);

public Q_SLOTS:
    void handle_context_created(dynamic_context* context);
    void handle_context_renamed(dynamic_context* context);
    void handle_context_removed(dynamic_context* context);

private:
    QTabWidget* m_tab_widget;
    QVBoxLayout* m_layout;

    QMap<dynamic_context*, QWidget*> m_context_widget_map;

    int get_context_tab_index(dynamic_context* context) const;

    //functions
    void handle_tab_close_requested(int index);

    void add_graph_widget_tab(dynamic_context* context);
};

#endif    //GRAPH_TAB_WIDGET

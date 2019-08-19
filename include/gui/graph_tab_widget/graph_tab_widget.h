#ifndef GRAPH_TAB_WIDGET
#define GRAPH_TAB_WIDGET

#include "content_widget/content_widget.h"

#include "graph_widget/contexts/dynamic_context.h"

class QTabWidget;
class QVBoxLayout;

class graph_tab_widget : public content_widget
{
    Q_OBJECT

public:
    graph_tab_widget(QWidget* parent = nullptr);

    int addTab(QWidget* tab, QString tab_name = "default");

public Q_SLOTS:
    void handle_context_created(dynamic_context* context, QString context_name);

private:
    QTabWidget* m_tab_widget;
    QVBoxLayout* m_layout;

    //functions
    void handle_tab_close_requested(int index);

};

#endif //GRAPH_TAB_WIDGET

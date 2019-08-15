#ifndef GRAPH_TAB_WIDGET
#define GRAPH_TAB_WIDGET

#include "content_widget/content_widget.h"

class QTabWidget;
class QVBoxLayout;

class graph_tab_widget : public content_widget
{

public:
    graph_tab_widget(QWidget* parent = nullptr);

    void addTab(QWidget* tab, QString tab_name = "default");

private:
    QTabWidget* m_tab_widget;
    QVBoxLayout* m_layout;

    //functions
    void handle_tab_close_requested(int index);

};

#endif //GRAPH_TAB_WIDGET

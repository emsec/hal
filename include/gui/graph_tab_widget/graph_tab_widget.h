#ifndef GRAPH_TAB_WIDGET
#define GRAPH_TAB_WIDGET

#include "content_widget/content_widget.h"

#include "graph_widget/contexts/graph_context.h"

#include <QMap>

class QTabWidget;
class QVBoxLayout;

class graph_tab_widget : public content_widget
{
    Q_OBJECT

public:
    graph_tab_widget(QWidget* parent = nullptr);

    virtual QList<QShortcut*> create_shortcuts() override;

    int addTab(QWidget* tab, QString tab_name = "default");
    void show_context(graph_context* context);

public Q_SLOTS:
    void handle_context_created(graph_context* context);
    void handle_context_renamed(graph_context* context);
    void handle_context_removed(graph_context* context);

    void handle_tab_changed(int index);

private:
    QTabWidget* m_tab_widget;
    QVBoxLayout* m_layout;

    float m_zoom_factor;

    QMap<graph_context*, QWidget*> m_context_widget_map;

    int get_context_tab_index(graph_context* context) const;

    //functions
    void handle_tab_close_requested(int index);

    void add_graph_widget_tab(graph_context* context);

    void zoom_in_shortcut();
    void zoom_out_shortcut();
};

#endif    //GRAPH_TAB_WIDGET

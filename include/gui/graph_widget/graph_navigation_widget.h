#ifndef GRAPH_NAVIGATION_WIDGET_H
#define GRAPH_NAVIGATION_WIDGET_H

#include "def.h"

#include "netlist/net.h"

#include <QTableWidget>
class graph_graphics_view;

class graph_navigation_widget : public QTableWidget
{
    Q_OBJECT

public:
    explicit graph_navigation_widget(QWidget *parent = nullptr);

    void setup();

Q_SIGNALS:
    void navigation_requested(const u32 via_net, const u32 to_gate);
    void close_requested();
    void reset_focus();

protected:
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

private:
    void fill_table(std::shared_ptr<net> n);
    graph_graphics_view* m_view;
    u32 m_via_net;
};

#endif // GRAPH_NAVIGATION_WIDGET_H

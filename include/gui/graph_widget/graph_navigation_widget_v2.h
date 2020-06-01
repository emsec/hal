#ifndef GRAPH_NAVIGATION_WIDGET_V2_H
#define GRAPH_NAVIGATION_WIDGET_V2_H

#include "def.h"

#include "netlist/net.h"

#include "gui/gui_def.h"

#include <QTreeWidget>

class graph_graphics_view;

class graph_navigation_widget_v2 : public QTreeWidget
{
    Q_OBJECT

public:
    explicit graph_navigation_widget_v2(QWidget *parent = nullptr);

    void setup(bool direction);
    void setup(hal::node origin, std::shared_ptr<net> via_net, bool direction);
    void hide_when_focus_lost(bool hide);

Q_SIGNALS:
    void navigation_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
    void close_requested();
    void reset_focus();

private Q_SLOTS:
    void handle_selection_changed();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void fill_table(bool direction);
    void resize_to_fit();
    int sum_row_heights(const QTreeWidgetItem *itm, bool top = true);
    void commit_selection();

    hal::node m_origin;
    std::shared_ptr<net> m_via_net;
    QSet<QTreeWidgetItem*> m_previous_selection;

};

#endif
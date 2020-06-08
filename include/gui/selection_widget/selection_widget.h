#pragma once

#include "content_widget/content_widget.h"

#include "def.h"

#include "searchbar/searchbar.h"
#include "selection_widget/models/gate_table_model.h"
#include "selection_widget/models/net_table_model.h"
#include "selection_widget/models/gate_table_proxy_model.h"
#include "selection_widget/models/net_table_proxy_model.h"

#include<QTableView>

class selection_widget : public content_widget
{
    Q_OBJECT

public:
    selection_widget(QWidget* parent = nullptr);

public Q_SLOTS:
    void handle_selection_update(void* sender);

private:
    searchbar m_searchbar;

    QTableView* m_gate_table_view;
    QTableView* m_net_table_view;

    gate_table_model* m_gate_table_model;
    net_table_model* m_net_table_model;

    gate_table_proxy_model* m_gate_table_proxy_model;
    net_table_proxy_model* m_net_table_proxy_model;

    void toggle_searchbar();
    QList<QShortcut*> create_shortcuts();

    void update();
};

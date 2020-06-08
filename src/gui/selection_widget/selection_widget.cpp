#include "selection_widget/selection_widget.h"

#include "gui_globals.h"
#include "content_widget/content_widget.h"

#include <QVBoxLayout>

selection_widget::selection_widget(QWidget* parent): content_widget("Selections", parent)
{    
    m_gate_table_view = new QTableView();
    m_gate_table_model = new gate_table_model();
    m_gate_table_proxy_model = new gate_table_proxy_model();
    m_gate_table_proxy_model->setSourceModel(m_gate_table_model);
    m_gate_table_view->setModel(m_gate_table_proxy_model);
    m_content_layout->addWidget(m_gate_table_view);

    m_net_table_view = new QTableView();
    m_net_table_model = new net_table_model();
    m_net_table_proxy_model = new net_table_proxy_model();
    m_net_table_proxy_model->setSourceModel(m_net_table_model);
    m_net_table_view->setModel(m_net_table_proxy_model);
    m_content_layout->addWidget(m_net_table_view);
    
    m_content_layout->addWidget(&m_searchbar);
    m_searchbar.hide();

    connect(&m_searchbar, &searchbar::text_edited, m_gate_table_proxy_model, &gate_table_proxy_model::handle_filter_text_changed);
    connect(&m_searchbar, &searchbar::text_edited, m_net_table_proxy_model, &net_table_proxy_model::handle_filter_text_changed);

    connect(&g_selection_relay, &selection_relay::selection_changed, this, &selection_widget::update);
}

void selection_widget::selection_widget::update()
{
    m_gate_table_model->fetch_selection();
    m_net_table_model->fetch_selection();
}

void selection_widget::toggle_searchbar()
{
    if (m_searchbar.isHidden())
    {
        m_searchbar.show();
        m_searchbar.setFocus();
    }
    else
    {
        m_searchbar.hide();
        this->setFocus();
    }
}

QList<QShortcut*> selection_widget::create_shortcuts()
{
    QShortcut* search_shortcut = g_keybind_manager.make_shortcut(this, "keybinds/searchbar_toggle");
    connect(search_shortcut, &QShortcut::activated, this, &selection_widget::toggle_searchbar);

    QList<QShortcut*> list;
    list.append(search_shortcut);

    return list;
}

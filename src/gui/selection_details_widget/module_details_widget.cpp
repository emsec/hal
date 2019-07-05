#include "selection_details_widget/module_details_widget.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include <QLabel>
#include <QVBoxLayout>

module_details_widget::module_details_widget(QWidget* parent) : QWidget(parent)
{
    m_content_layout = new QVBoxLayout(this);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    m_content_layout->setAlignment(Qt::AlignTop);

    m_label = new QLabel(this);
    m_label->setStyleSheet("QLabel {Background-Color: rgba(49, 51, 53, 1);}");
    m_content_layout->addWidget(m_label);

    connect(&g_netlist_relay, &netlist_relay::module_event, this, &module_details_widget::handle_module_event);
}

void module_details_widget::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(associated_data)
    if (m_current_id == module->get_id())
    {
        update(module->get_id());
    }
}

void module_details_widget::update(u32 module_id)
{
    m_current_id = module_id;

    //    gate *gate = g_netlist->get_gate_by_id(gate_id);
    //    QString name = QString::fromStdString(gate->get_name());
    //    QString type = QString::fromStdString(gate->get_type());
    //    QString id = QString::number(gate_id);

    //    QString name_html = "<b>" + name + "</b>";
    //    QString gate_html = "<font color=\"orange\">gate</font>";
    //    QString type_html = type;
    //    QString id_html = id;

    //QString richtext = name_html + "<br>" + gate_html + ", " + type_html + " (ID " + id_html + ")";
    QString richtext = "not implemented yet";

    m_label->setText(richtext);
}

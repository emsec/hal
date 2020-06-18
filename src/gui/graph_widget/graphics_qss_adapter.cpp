#include "graph_widget/graphics_qss_adapter.h"

#include <QStyle>
namespace hal{
graphics_qss_adapter::graphics_qss_adapter(QWidget* parent) : QWidget(parent),
    m_gate_base_color(255, 200, 0),
    m_net_base_color(255, 200, 0),
    m_gate_selection_color(255, 200, 0),
    m_net_selection_color(255, 200, 0),
    m_grid_base_line_color(255, 200, 0),
    m_grid_cluster_line_color(255, 200, 0),
    m_grid_base_dot_color(255, 200, 0),
    m_grid_cluster_dot_color(255, 200, 0)
{
    ensurePolished();
}

void graphics_qss_adapter::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    // SET STATICS OR ADRESS ADAPTER MEMBERS DIRECTLY
}

QColor graphics_qss_adapter::gate_base_color() const
{
    return m_gate_base_color;
}

QColor graphics_qss_adapter::net_base_color() const
{
    return m_net_base_color;
}

QColor graphics_qss_adapter::gate_selection_color() const
{
    return m_gate_selection_color;
}

QColor graphics_qss_adapter::net_selection_color() const
{
    return m_net_selection_color;
}

QFont graphics_qss_adapter::gate_name_font() const
{
    return m_gate_name_font;
}

QFont graphics_qss_adapter::gate_type_font() const
{
    return m_gate_type_font;
}

QFont graphics_qss_adapter::gate_pin_font() const
{
    return m_gate_pin_font;
}

QFont graphics_qss_adapter::net_font() const
{
    return m_net_font;
}

QColor graphics_qss_adapter::grid_base_line_color() const
{
    return m_grid_base_line_color;
}

QColor graphics_qss_adapter::grid_cluster_line_color() const
{
    return m_grid_cluster_line_color;
}

QColor graphics_qss_adapter::grid_base_dot_color() const
{
    return m_grid_base_dot_color;
}

QColor graphics_qss_adapter::grid_cluster_dot_color() const
{
    return m_grid_cluster_dot_color;
}

void graphics_qss_adapter::set_gate_base_color(const QColor& color)
{
    m_gate_base_color = color;
}

void graphics_qss_adapter::set_net_base_color(const QColor& color)
{
    m_net_base_color = color;
}

void graphics_qss_adapter::set_gate_selection_color(const QColor& color)
{
    m_gate_selection_color = color;
}

void graphics_qss_adapter::set_net_selection_color(const QColor& color)
{
    m_net_selection_color = color;
}

void graphics_qss_adapter::set_gate_name_font(const QFont& font)
{
    m_gate_name_font = font;
}

void graphics_qss_adapter::set_gate_type_font(const QFont& font)
{
    m_gate_type_font = font;
}

void graphics_qss_adapter::set_gate_pin_font(const QFont& font)
{
    m_gate_pin_font = font;
}

void graphics_qss_adapter::set_net_font(const QFont& font)
{
    m_net_font = font;
}

void graphics_qss_adapter::set_grid_base_line_color(const QColor& color)
{
    m_grid_base_line_color = color;
}

void graphics_qss_adapter::set_grid_cluster_line_color(const QColor& color)
{
    m_grid_cluster_line_color = color;
}

void graphics_qss_adapter::set_grid_base_dot_color(const QColor& color)
{
    m_grid_base_dot_color = color;
}

void graphics_qss_adapter::set_grid_cluster_dot_color(const QColor& color)
{
    m_grid_cluster_dot_color = color;
}
}

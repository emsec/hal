#include "gui/graph_widget/graphics_qss_adapter.h"

#include <QStyle>

namespace hal
{
    GraphicsQssAdapter::GraphicsQssAdapter(QWidget* parent) : QWidget(parent),
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

    void GraphicsQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        // SET STATICS OR ADRESS ADAPTER MEMBERS DIRECTLY
    }

    QColor GraphicsQssAdapter::gate_base_color() const
    {
        return m_gate_base_color;
    }

    QColor GraphicsQssAdapter::net_base_color() const
    {
        return m_net_base_color;
    }

    QColor GraphicsQssAdapter::gate_selection_color() const
    {
        return m_gate_selection_color;
    }

    QColor GraphicsQssAdapter::net_selection_color() const
    {
        return m_net_selection_color;
    }

    QFont GraphicsQssAdapter::gate_name_font() const
    {
        return m_gate_name_font;
    }

    QFont GraphicsQssAdapter::gate_type_font() const
    {
        return m_gate_type_font;
    }

    QFont GraphicsQssAdapter::gate_pin_font() const
    {
        return m_gate_pin_font;
    }

    QFont GraphicsQssAdapter::net_font() const
    {
        return m_net_font;
    }

    QColor GraphicsQssAdapter::grid_base_line_color() const
    {
        return m_grid_base_line_color;
    }

    QColor GraphicsQssAdapter::grid_cluster_line_color() const
    {
        return m_grid_cluster_line_color;
    }

    QColor GraphicsQssAdapter::grid_base_dot_color() const
    {
        return m_grid_base_dot_color;
    }

    QColor GraphicsQssAdapter::grid_cluster_dot_color() const
    {
        return m_grid_cluster_dot_color;
    }

    void GraphicsQssAdapter::set_gate_base_color(const QColor& color)
    {
        m_gate_base_color = color;
    }

    void GraphicsQssAdapter::set_net_base_color(const QColor& color)
    {
        m_net_base_color = color;
    }

    void GraphicsQssAdapter::set_gate_selection_color(const QColor& color)
    {
        m_gate_selection_color = color;
    }

    void GraphicsQssAdapter::set_net_selection_color(const QColor& color)
    {
        m_net_selection_color = color;
    }

    void GraphicsQssAdapter::set_gate_name_font(const QFont& font)
    {
        m_gate_name_font = font;
    }

    void GraphicsQssAdapter::set_gate_type_font(const QFont& font)
    {
        m_gate_type_font = font;
    }

    void GraphicsQssAdapter::set_gate_pin_font(const QFont& font)
    {
        m_gate_pin_font = font;
    }

    void GraphicsQssAdapter::set_net_font(const QFont& font)
    {
        m_net_font = font;
    }

    void GraphicsQssAdapter::set_grid_base_line_color(const QColor& color)
    {
        m_grid_base_line_color = color;
    }

    void GraphicsQssAdapter::set_grid_cluster_line_color(const QColor& color)
    {
        m_grid_cluster_line_color = color;
    }

    void GraphicsQssAdapter::set_grid_base_dot_color(const QColor& color)
    {
        m_grid_base_dot_color = color;
    }

    void GraphicsQssAdapter::set_grid_cluster_dot_color(const QColor& color)
    {
        m_grid_cluster_dot_color = color;
    }
}

#include "graph_widget/graphics_items/graphics_gate.h"
#include "core/log.h"
#include "netlist/gate.h"
#include <QtWidgets>
#include <core/log.h>
#include <limits>

graphics_gate::graphics_gate(std::shared_ptr<gate> g)
{
    if (g)
    {
        m_name = QString::fromStdString(g->get_name());
        m_id   = g->get_id();
        m_type = "(" + QString::fromStdString(g->get_type()) + ")";
        for (std::string input_pin : g->get_input_pin_types())
            m_input_pins.append(QString::fromStdString(input_pin));
        for (std::string output_pin : g->get_output_pin_types())
            m_output_pins.append(QString::fromStdString(output_pin));
    }
    else
    {
        m_name = "Nullptr";
        m_type = ":(";
        m_id   = -1;
        log_error("gui graph", "inconsistency error (node is nullptr).");
    }
    m_class = item_class::gate;
    m_gate  = g;
}

QRectF graphics_gate::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath graphics_gate::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}

qreal graphics_gate::get_width()
{
    return m_width;
}

qreal graphics_gate::get_height()
{
    return m_height;
}

std::string graphics_gate::get_input_pin_type_at_position(size_t pos)
{
    return (pos >= (size_t)m_input_pins.size()) ? "" : m_input_pins.at(pos).toStdString();
}

std::string graphics_gate::get_output_pin_type_at_position(size_t pos)
{
    return (pos >= (size_t)m_output_pins.size()) ? "" : m_output_pins.at(pos).toStdString();
}

std::shared_ptr<gate> graphics_gate::get_gate()
{
    return m_gate;
}

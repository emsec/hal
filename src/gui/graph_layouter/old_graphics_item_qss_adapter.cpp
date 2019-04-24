#include "graph_layouter/old_graphics_item_qss_adapter.h"
#include <QStyle>

old_graphics_item_qss_adapter::old_graphics_item_qss_adapter(QWidget *parent) : QWidget(parent)
{
    style()->unpolish(this);
    style()->polish(this);
    hide();
}

old_graphics_item_qss_adapter *old_graphics_item_qss_adapter::instance()
{
    static old_graphics_item_qss_adapter* instance;
    if(!instance)
        instance = new old_graphics_item_qss_adapter();

    return instance;
}

QColor old_graphics_item_qss_adapter::gate_default_color() const
{
    return m_gate_default_color;
}

QColor old_graphics_item_qss_adapter::gate_background_color() const
{
    return m_gate_background_color;
}

QColor old_graphics_item_qss_adapter::gate_font_color() const
{
    return m_gate_font_color;
}

QColor old_graphics_item_qss_adapter::gate_selected_background_color() const
{
    return m_gate_selected_background_color;
}

QColor old_graphics_item_qss_adapter::gate_selected_font_color() const
{
    return m_gate_selected_font_color;
}

QColor old_graphics_item_qss_adapter::net_default_color() const
{
    return m_net_default_color;
}

QColor old_graphics_item_qss_adapter::net_selected_color() const
{
    return m_net_selected_color;
}

QColor old_graphics_item_qss_adapter::net_global_input_output_color() const
{
    return m_net_global_input_output_color;
}

QString old_graphics_item_qss_adapter::tree_navigation_open_folder_style() const
{
    return m_tree_navigation_open_folder_style;
}

QString old_graphics_item_qss_adapter::tree_navigation_open_folder_path() const
{
    return m_tree_navigation_open_folder_path;
}

void old_graphics_item_qss_adapter::set_gate_default_color(const QColor &color)
{
    m_gate_default_color = color;
}

void old_graphics_item_qss_adapter::set_gate_background_color(const QColor &color)
{
    m_gate_background_color = color;
}

void old_graphics_item_qss_adapter::set_gate_font_color(const QColor &color)
{
    m_gate_font_color = color;
}

void old_graphics_item_qss_adapter::set_gate_selected_background_color(const QColor &color)
{
    m_gate_selected_background_color = color;
}

void old_graphics_item_qss_adapter::set_gate_selected_font_color(const QColor &color)
{
    m_gate_selected_font_color = color;
}

void old_graphics_item_qss_adapter::set_net_default_color(const QColor &color)
{
    m_net_default_color = color;
}

void old_graphics_item_qss_adapter::set_net_selected_color(const QColor &color)
{
    m_net_selected_color = color;
}

void old_graphics_item_qss_adapter::set_net_global_input_output_color(const QColor &color)
{
    m_net_global_input_output_color = color;
}

void old_graphics_item_qss_adapter::set_tree_navigation_open_folder_style(const QString &style)
{
    m_tree_navigation_open_folder_style = style;
}

void old_graphics_item_qss_adapter::set_tree_navigation_open_folder_path(const QString &path)
{
    m_tree_navigation_open_folder_path = path;
}

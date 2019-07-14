#ifndef NETLIST_GRAPHICS_QSS_ADAPTER_H
#define NETLIST_GRAPHICS_QSS_ADAPTER_H

#include <QWidget>

class netlist_graphics_qss_adapter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor gate_base_color READ gate_base_color WRITE set_gate_base_color)
    Q_PROPERTY(QColor net_base_color READ net_base_color WRITE set_net_base_color)

    Q_PROPERTY(QColor gate_selection_color READ gate_selection_color WRITE set_gate_selection_color)
    Q_PROPERTY(QColor net_selection_color READ net_selection_color WRITE set_net_selection_color)

//    Q_PROPERTY(QFont gate_name_font READ gate_name_font WRITE set_gate_name_font)
//    Q_PROPERTY(QFont gate_type_font READ gate_type_font WRITE set_gate_type_font)
//    Q_PROPERTY(QFont gate_pin_font READ gate_pin_font WRITE set_gate_pin_font)

//    Q_PROPERTY(QFont net_font READ net_font WRITE set_net_font)

    Q_PROPERTY(QColor grid_base_line_color READ grid_base_line_color WRITE set_grid_base_line_color)
    Q_PROPERTY(QColor grid_cluster_line_color READ grid_cluster_line_color WRITE set_grid_cluster_line_color)

    Q_PROPERTY(QColor grid_base_dot_color READ grid_base_dot_color WRITE set_grid_base_dot_color)
    Q_PROPERTY(QColor grid_cluster_dot_color READ grid_cluster_dot_color WRITE set_grid_cluster_dot_color)

public:
    explicit netlist_graphics_qss_adapter(QWidget* parent = nullptr);

    void repolish();

    QColor gate_base_color() const;
    QColor net_base_color() const;

    // GATE BACKGROUND COLOR
    // GATE OUTLINE COLOR

    QColor gate_selection_color() const;
    QColor net_selection_color() const;

//    QFont gate_name_font() const;
//    QFont gate_type_font() const;
//    QFont gate_pin_font() const;

//    QFont net_font() const;

    QColor grid_base_line_color() const;
    QColor grid_cluster_line_color() const;

    QColor grid_base_dot_color() const;
    QColor grid_cluster_dot_color() const;

    void set_gate_base_color(const QColor& color);
    void set_net_base_color(const QColor& color);

    void set_gate_selection_color(const QColor& color);
    void set_net_selection_color(const QColor& color);

//    void set_gate_name_font(const QFont& font);
//    void set_gate_type_font(const QFont& font);
//    void set_gate_pin_font(const QFont& font);

//    void set_net_font(const QFont& font);

    void set_grid_base_line_color(const QColor& color);
    void set_grid_cluster_line_color(const QColor& color);

    void set_grid_base_dot_color(const QColor& color);
    void set_grid_cluster_dot_color(const QColor& color);

    // LEFT PUBLIC INTENTIONALLY
    QColor m_gate_base_color;
    QColor m_net_base_color;

    QColor m_gate_selection_color;
    QColor m_net_selection_color;

//    QFont m_gate_name_font;
//    QFont m_gate_type_font;
//    QFont m_gate_pin_font;

//    QFont m_net_font;

    QColor m_grid_base_line_color;
    QColor m_grid_cluster_line_color;

    QColor m_grid_base_dot_color;
    QColor m_grid_cluster_dot_color;
};

#endif // NETLIST_GRAPHICS_QSS_ADAPTER_H

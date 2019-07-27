#ifndef STANDARD_GRAPH_LAYOUTER_H
#define STANDARD_GRAPH_LAYOUTER_H

#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/gui_def.h"

#include <QSet>
#include <QVector>

class graphics_node;

class standard_graph_layouter : public graph_layouter
{
    struct node_level
    {
        hal::node node;
        int level;
    };

    struct node_box
    {
        hal::node node;
        graphics_node* item;

        int x;
        int y;

        qreal input_padding;
        qreal output_padding;
    };

    struct road
    {
        road(const int x_coordinate, const int y_coordinate) :
            x(x_coordinate),
            y(y_coordinate),
            lanes(0)
        {}

        int x;
        int y;

        unsigned int lanes = 0;

        qreal vertical_width() const;
        qreal horizontal_height() const;
    };

    struct junction
    {
        junction(const int x_coordinate, const int y_coordinate) :
            x(x_coordinate),
            y(y_coordinate),
            h_lanes(0),
            v_lanes(0),
            close_left_lane_changes(0),
            close_right_lane_changes(0),
            close_top_lane_changes(0),
            close_bottom_lane_changes(0),
            far_left_lane_changes(0),
            far_right_lane_changes(0),
            far_top_lane_changes(0),
            far_bottom_lane_changes(0),
            max_left_lane_changes(0),
            max_right_lane_changes(0),
            max_top_lane_changes(0),
            max_bottom_lane_changes(0)

        {}

        int x;
        int y;

        unsigned int h_lanes = 0;
        unsigned int v_lanes = 0;

        unsigned int close_left_lane_changes = 0;
        unsigned int close_right_lane_changes = 0;
        unsigned int close_top_lane_changes = 0;
        unsigned int close_bottom_lane_changes = 0;

        unsigned int far_left_lane_changes = 0;
        unsigned int far_right_lane_changes = 0;
        unsigned int far_top_lane_changes = 0;
        unsigned int far_bottom_lane_changes = 0;

        unsigned int max_left_lane_changes = 0;
        unsigned int max_right_lane_changes = 0;
        unsigned int max_top_lane_changes = 0;
        unsigned int max_bottom_lane_changes = 0;

        qreal width() const;
        qreal height() const;
    };

    struct used_paths
    {
        QSet<road*> h_roads;
        QSet<road*> v_roads;

        QSet<junction*> h_junctions;
        QSet<junction*> v_junctions;

        QSet<junction*> close_left_junctions;
        QSet<junction*> close_right_junctions;
        QSet<junction*> close_top_junctions;
        QSet<junction*> close_bottom_junctions;

        QSet<junction*> far_left_junctions;
        QSet<junction*> far_right_junctions;
        QSet<junction*> far_top_junctions;
        QSet<junction*> far_bottom_junctions;
    };

public:
    standard_graph_layouter(graph_context* context);

    void layout() Q_DECL_OVERRIDE;

    void expand(const u32 from_gate, const u32 via_net, const u32 to_gate) Q_DECL_OVERRIDE;

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) Q_DECL_OVERRIDE;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) Q_DECL_OVERRIDE;

    virtual const QString name() const Q_DECL_OVERRIDE;
    virtual const QString description() const Q_DECL_OVERRIDE;

private:
    void create_boxes();
    void calculate_nets();
    void find_max_box_dimensions();
    void find_max_channel_lanes();
    void calculate_max_junction_spacing();
    void calculate_channel_spacing();
    void calculate_max_channel_dimensions();
    void calculate_gate_offsets();
    void place_gates();
    void update_scene_rect();
    void reset_roads_and_junctions();
    void draw_nets();

    void clear_net_layout_data();

    node_box create_box(const hal::node& node, const int x, const int y) const;

    void add_gate(const u32 gate_id, const int level);

    bool box_exists(const int x, const int y) const;

    bool h_road_jump_possible(const int x, const int y1, const int y2) const;
    bool h_road_jump_possible(const road* const r1, const road* const r2) const;

    bool v_road_jump_possible(const int x1, const int x2, const int y) const;
    bool v_road_jump_possible(const road* const r1, const road* const r2) const;

    road* get_h_road(const int x, const int y);
    road* get_v_road(const int x, const int y);
    junction* get_junction(const int x, const int y);

    qreal h_road_height(const unsigned int lanes) const;
    qreal v_road_width(const unsigned int lanes) const;

    qreal scene_y_for_h_channel_lane(const int y, const unsigned int lane) const;
    qreal scene_x_for_v_channel_lane(const int x, const unsigned int lane) const;

    qreal scene_x_for_close_left_lane_change(const int channel_x, unsigned int lane_change) const;
    qreal scene_x_for_far_left_lane_change(const int channel_x, unsigned int lane_change) const;

    qreal scene_x_for_close_right_lane_change(const int channel_x, unsigned int lane_change) const;
    qreal scene_x_for_far_right_lane_change(const int channel_x, unsigned int lane_change) const;

    qreal scene_y_for_close_top_lane_change(const int channel_y, unsigned int lane_change) const;
    qreal scene_y_for_far_top_lane_change(const int channel_y, unsigned int lane_change) const;

    qreal scene_y_for_close_bottom_lane_change(const int channel_y, unsigned int lane_change) const;
    qreal scene_y_for_far_bottom_lane_change(const int channel_y, unsigned int lane_change) const;

    //qreal scene_x_for_close_left_lane_change(const junction* const j) const;
    //qreal scene_x_for_close_right_lane_change(const junction* const j) const;
    //qreal scene_y_for_close_top_lane_change(const junction* const j) const;
    //qreal scene_y_for_close_bottom_lane_change(const junction* const j) const;

    template<typename T1, typename T2>
    void store_max(QMap<T1, T2>& map, T1 key, T2 value);

    void commit_used_paths(const used_paths& used);

    QVector<u32> m_modules;
    QVector<u32> m_gates;
    QVector<u32> m_nets;

    QMap<hal::node, int> m_node_levels;

    // COMBINE LEVEL VECTORS ???
    QVector<hal::node> m_zero_nodes;

    QVector<QVector<hal::node>> m_positive_nodes;
    QVector<QVector<hal::node>> m_negative_nodes;

    QVector<node_box> m_boxes;

    QVector<road*> m_h_roads;
    QVector<road*> m_v_roads;
    QVector<junction*> m_junctions;

    QMap<int, qreal> m_max_node_width_for_x;
    QMap<int, qreal> m_max_node_height_for_y;

    QMap<int, qreal> m_max_v_channel_width_for_x;
    QMap<int, qreal> m_max_h_channel_height_for_y;

    QMap<int, qreal> m_max_v_channel_left_spacing_for_x;
    QMap<int, qreal> m_max_v_channel_right_spacing_for_x;
    QMap<int, qreal> m_max_h_channel_top_spacing_for_y;
    QMap<int, qreal> m_max_h_channel_bottom_spacing_for_y;

    QMap<int, qreal> m_node_offset_for_x;
    QMap<int, qreal> m_node_offset_for_y;

    QMap<int, qreal> m_max_left_junction_spacing_for_x;
    QMap<int, qreal> m_max_right_junction_spacing_for_x;

    QMap<int, qreal> m_max_top_junction_spacing_for_y;
    QMap<int, qreal> m_max_bottom_junction_spacing_for_y;

    QMap<int, unsigned int> m_max_v_channel_lanes_for_x;
    QMap<int, unsigned int> m_max_h_channel_lanes_for_y;

    QMap<int, qreal> m_max_left_io_padding_for_channel_x;
    QMap<int, qreal> m_max_right_io_padding_for_channel_x;
};

#endif // STANDARD_GRAPH_LAYOUTER_H

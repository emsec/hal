#include "gui/graph_widget/layouters/graph_layouter.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/gui_globals.h"
#include "gui/implementations/qpoint_extension.h"

#include "qmath.h"

template<typename T1, typename T2>
static void store_max(QMap<T1, T2>& map, T1 key, T2 value)
{
    if (map.contains(key))
        if (map.value(key) >= value)
            return;

    map.insert(key, value);
}

const static qreal lane_spacing             = 10;
const static qreal junction_padding         = 10;
const static qreal h_road_padding           = 10;
const static qreal v_road_padding           = 10;
const static qreal minimum_v_channel_width  = 20;
const static qreal minimum_h_channel_height = 20;
const static qreal minimum_gate_io_padding  = 60;

graph_layouter::graph_layouter(const graph_context* const context, QObject* parent) : QObject(parent), m_scene(new graphics_scene(this)), m_context(context), m_done(false)
{
}

graphics_scene* graph_layouter::scene() const
{
    return m_scene;
}

const QMap<hal::node, QPoint> graph_layouter::node_to_position_map() const
{
    return m_node_to_position_map;
}

const QMap<QPoint, hal::node> graph_layouter::position_to_node_map() const
{
    return m_position_to_node_map;
}

void graph_layouter::set_node_position(const hal::node& n, const QPoint& p)
{
    if (m_node_to_position_map.contains(n))
    {
        QPoint old_p = m_node_to_position_map.value(n);
        m_position_to_node_map.remove(old_p);
    }

    m_node_to_position_map.insert(n, p);
    m_position_to_node_map.insert(p, n);

    //manual relayout call needed
}

void graph_layouter::swap_node_positions(const hal::node& n1, const hal::node& n2)
{
    assert(m_node_to_position_map.contains(n1));
    assert(m_node_to_position_map.contains(n2));

    QPoint p1 = m_node_to_position_map.value(n1);
    QPoint p2 = m_node_to_position_map.value(n2);

    m_node_to_position_map.insert(n1, p2); // implicit replace
    m_node_to_position_map.insert(n2, p1);

    m_position_to_node_map.insert(p1, n2);
    m_position_to_node_map.insert(p2, n1);
}

void graph_layouter::remove_node_from_maps(const hal::node& n)
{
    if (m_node_to_position_map.contains(n))
    {
        QPoint old_p = m_node_to_position_map.value(n);
        m_node_to_position_map.remove(n);
        m_position_to_node_map.remove(old_p);
    }
}

int graph_layouter::min_x_index() const
{
    return m_min_x_index;
}

int graph_layouter::min_y_index() const
{
    return m_min_y_index;
}

bool graph_layouter::done() const
{
    return m_done;
}

QVector<qreal> graph_layouter::x_values() const
{
    return m_x_values;
}

QVector<qreal> graph_layouter::y_values() const
{
    return m_y_values;
}

qreal graph_layouter::max_node_width() const
{
    return m_max_node_width;
}

qreal graph_layouter::max_node_height() const
{
    return m_max_node_height;
}

qreal graph_layouter::default_grid_width() const
{
    return m_max_node_width + minimum_v_channel_width;
}

qreal graph_layouter::default_grid_height() const
{
    return m_max_node_height + minimum_h_channel_height;
}

void graph_layouter::layout()
{
    m_scene->delete_all_items();
    clear_layout_data();

    create_boxes();
    calculate_nets();
    find_max_box_dimensions();
    find_max_channel_lanes();
    reset_roads_and_junctions();
    calculate_max_channel_dimensions();
    calculate_gate_offsets();
    place_gates();
    m_done = true;
    draw_nets();
    update_scene_rect();

    m_scene->move_nets_to_background();
    m_scene->handle_extern_selection_changed(nullptr);

    #ifdef GUI_DEBUG_GRID
    m_scene->debug_set_layouter_grid(x_values(), y_values(), default_grid_height(), default_grid_width());
    #endif
}

void graph_layouter::clear_layout_data()
{
    m_done = false;

    m_boxes.clear();

    for (const graph_layouter::road* r : m_h_roads)
        delete r;
    m_h_roads.clear();

    for (const graph_layouter::road* r : m_v_roads)
        delete r;
    m_v_roads.clear();

    for (const graph_layouter::junction* j : m_junctions)
        delete j;
    m_junctions.clear();

    m_max_node_width_for_x.clear();
    m_max_node_height_for_y.clear();

    m_max_v_channel_lanes_for_x.clear();
    m_max_h_channel_lanes_for_y.clear();

    m_max_v_channel_left_spacing_for_x.clear();
    m_max_v_channel_right_spacing_for_x.clear();
    m_max_h_channel_top_spacing_for_y.clear();
    m_max_h_channel_bottom_spacing_for_y.clear();

    m_max_v_channel_width_for_x.clear();
    m_max_h_channel_height_for_y.clear();

    m_node_offset_for_x.clear();
    m_node_offset_for_y.clear();

    m_max_left_junction_spacing_for_x.clear();
    m_max_right_junction_spacing_for_x.clear();

    m_max_top_junction_spacing_for_y.clear();
    m_max_bottom_junction_spacing_for_y.clear();

    m_max_left_io_padding_for_channel_x.clear();
    m_max_right_io_padding_for_channel_x.clear();

    m_min_x_index = 0;
    m_min_y_index = 0;

    m_max_x_index = 0;
    m_max_y_index = 0;

    m_x_values.clear();
    m_y_values.clear();

    m_max_node_width  = 0;
    m_max_node_height = 0;
}

void graph_layouter::create_boxes()
{
    QMap<QPoint, hal::node>::const_iterator i = position_to_node_map().constBegin();
    while (i != position_to_node_map().constEnd())
    {
        m_boxes.append(create_box(i.value(), i.key().x(), i.key().y()));
        ++i;
    }
}

void graph_layouter::calculate_nets()
{
    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->is_unrouted())
            continue;

        used_paths used;

        for (const endpoint& src : n->get_sources())
        {

            // FIND SRC BOX
            hal::node node;

            if (!m_context->node_for_gate(node, src.get_gate()->get_id()))
                continue;

            node_box* src_box = nullptr;

            for (node_box& box : m_boxes)
                if (box.node == node)
                {
                    src_box = &box;
                    break;
                }

            assert(src_box);

            // FOR EVERY DST
            for (const endpoint& dst : n->get_destinations())
            {
                // FIND DST BOX
                if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                    continue;

                node_box* dst_box = nullptr;

                for (node_box& box : m_boxes)
                    if (box.node == node)
                    {
                        dst_box = &box;
                        break;
                    }

                assert(dst_box);

                // ROAD BASED DISTANCE (x_distance - 1)
                const int x_distance = dst_box->x - src_box->x - 1;
                const int y_distance = dst_box->y - src_box->y;

                if (!y_distance && v_road_jump_possible(src_box->x + 1, dst_box->x, src_box->y))
                {
                    // SPECIAL CASE INDIRECT HORIZONTAL NEIGHBORS
                    road* dst_v_road = get_v_road(dst_box->x, dst_box->y);
                    used.v_roads.insert(dst_v_road);
                    continue;
                }

                road* src_v_road = get_v_road(src_box->x + 1, src_box->y);

                if (!(x_distance || y_distance))
                {
                    // SPECIAL CASE DIRECT HORIZONTAL NEIGHBORS
                    used.v_roads.insert(src_v_road);
                    continue;
                }

                // NORMAL CASE
                // CONNECT SRC TO V ROAD, TRAVEL X DISTANCE, TRAVEL Y DISTANCE, CONNECT V ROAD TO DST
                used.v_roads.insert(src_v_road);

                junction* initial_junction = nullptr;
                int remaining_y_distance   = y_distance;

                if (y_distance < 0)
                {
                    // TRAVEL UP
                    initial_junction = get_junction(src_v_road->x, src_v_road->y);

                    if (src_v_road->lanes != initial_junction->v_lanes)
                    {
                        if (src_v_road->lanes < initial_junction->v_lanes)
                            used.close_bottom_junctions.insert(initial_junction);
                        else
                            used.far_bottom_junctions.insert(initial_junction);
                    }
                }
                else
                {
                    // TRAVEL DOWN
                    initial_junction = get_junction(src_v_road->x, src_v_road->y + 1);

                    if (src_v_road->lanes != initial_junction->v_lanes)
                    {
                        if (src_v_road->lanes < initial_junction->v_lanes)
                            used.close_top_junctions.insert(initial_junction);
                        else
                            used.far_top_junctions.insert(initial_junction);
                    }

                    if (!y_distance)
                        remaining_y_distance = -1;
                }

                used.v_junctions.insert(initial_junction);

                junction* last_junction = initial_junction;

                if (x_distance)
                {
                    used.h_junctions.insert(initial_junction);

                    int remaining_x_distance = x_distance;

                    // TRAVEL REMAINING X DISTANCE
                    while (remaining_x_distance)
                    {
                        road* r     = nullptr;
                        junction* j = nullptr;

                        if (x_distance > 0)
                        {
                            // TRAVEL RIGHT
                            r = get_h_road(last_junction->x, last_junction->y);

                            if (last_junction->h_lanes != r->lanes)
                            {
                                if (last_junction->h_lanes < r->lanes)
                                    used.far_right_junctions.insert(last_junction);
                                else
                                    used.close_right_junctions.insert(last_junction);
                            }

                            j = get_junction(last_junction->x + 1, last_junction->y);

                            if (r->lanes != j->h_lanes)
                            {
                                if (r->lanes < j->h_lanes)
                                    used.close_left_junctions.insert(j);
                                else
                                    used.far_left_junctions.insert(j);
                            }

                            --remaining_x_distance;
                        }
                        else
                        {
                            // TRAVEL LEFT
                            r = get_h_road(last_junction->x - 1, last_junction->y);

                            if (last_junction->h_lanes != r->lanes)
                            {
                                if (last_junction->h_lanes < r->lanes)
                                    used.far_left_junctions.insert(last_junction);
                                else
                                    used.close_left_junctions.insert(last_junction);
                            }

                            j = get_junction(last_junction->x - 1, last_junction->y);

                            if (r->lanes != j->h_lanes)
                            {
                                if (r->lanes < j->h_lanes)
                                    used.close_right_junctions.insert(j);
                                else
                                    used.far_right_junctions.insert(j);
                            }

                            ++remaining_x_distance;
                        }

                        used.h_roads.insert(r);
                        used.h_junctions.insert(j);

                        last_junction = j;
                    }

                    used.v_junctions.insert(last_junction);
                }

                // TRAVEL REMAINING Y DISTANCE
                if (remaining_y_distance > 0)
                {
                    while (remaining_y_distance != 1)
                    {
                        // TRAVEL DOWN
                        road* r = get_v_road(last_junction->x, last_junction->y);

                        if (last_junction->v_lanes != r->lanes)
                        {
                            if (last_junction->v_lanes < r->lanes)
                                used.far_bottom_junctions.insert(last_junction);
                            else
                                used.close_bottom_junctions.insert(last_junction);
                        }

                        junction* j = get_junction(last_junction->x, last_junction->y + 1);

                        if (r->lanes != j->v_lanes)
                        {
                            if (r->lanes < j->v_lanes)
                                used.close_top_junctions.insert(j);
                            else
                                used.far_top_junctions.insert(j);
                        }

                        used.v_roads.insert(r);
                        used.v_junctions.insert(j);

                        last_junction = j;

                        --remaining_y_distance;
                    }
                }
                else
                {
                    while (remaining_y_distance != -1)
                    {
                        // TRAVEL UP
                        road* r = get_v_road(last_junction->x, last_junction->y - 1);

                        if (last_junction->v_lanes != r->lanes)
                        {
                            if (last_junction->v_lanes < r->lanes)
                                used.far_top_junctions.insert(last_junction);
                            else
                                used.close_top_junctions.insert(last_junction);
                        }

                        junction* j = get_junction(last_junction->x, last_junction->y - 1);

                        if (r->lanes != j->v_lanes)
                        {
                            if (r->lanes < j->v_lanes)
                                used.close_bottom_junctions.insert(j);
                            else
                                used.far_bottom_junctions.insert(j);
                        }

                        used.v_roads.insert(r);
                        used.v_junctions.insert(j);

                        last_junction = j;

                        ++remaining_y_distance;
                    }
                }

                road* dst_road = nullptr;

                if (y_distance > 0)
                {
                    // TRAVEL DOWN
                    dst_road = get_v_road(last_junction->x, last_junction->y);

                    if (last_junction->v_lanes != dst_road->lanes)
                    {
                        if (last_junction->v_lanes < dst_road->lanes)
                            used.far_bottom_junctions.insert(last_junction);
                        else
                            used.close_bottom_junctions.insert(last_junction);
                    }
                }
                else
                {
                    // TRAVEL UP
                    dst_road = get_v_road(last_junction->x, last_junction->y - 1);

                    if (last_junction->v_lanes != dst_road->lanes)
                    {
                        if (last_junction->v_lanes < dst_road->lanes)
                            used.far_top_junctions.insert(last_junction);
                        else
                            used.close_top_junctions.insert(last_junction);
                    }
                }

                used.v_junctions.insert(last_junction);
                used.v_roads.insert(dst_road);
            }
        }

        commit_used_paths(used);
    }
}

void graph_layouter::find_max_box_dimensions()
{
    for (const node_box& box : m_boxes)
    {
        if (box.x < m_min_x_index)
            m_min_x_index = box.x;
        else if (box.x > m_max_x_index)
            m_max_x_index = box.x;

        if (box.y < m_min_y_index)
            m_min_y_index = box.y;
        else if (box.y > m_max_y_index)
            m_max_y_index = box.y;

        if (m_max_node_width < box.item->width())
            m_max_node_width = box.item->width();

        if (m_max_node_height < box.item->height())
            m_max_node_height = box.item->height();

        store_max(m_max_node_width_for_x, box.x, box.item->width());
        store_max(m_max_node_height_for_y, box.y, box.item->height());

        store_max(m_max_right_io_padding_for_channel_x, box.x, box.input_padding);
        store_max(m_max_left_io_padding_for_channel_x, box.x + 1, box.output_padding);
    }
}

void graph_layouter::find_max_channel_lanes()
{
    for (const road* r : m_v_roads)
        store_max(m_max_v_channel_lanes_for_x, r->x, r->lanes);

    for (const road* r : m_h_roads)
        store_max(m_max_h_channel_lanes_for_y, r->y, r->lanes);

    for (const junction* j : m_junctions)
    {
        store_max(m_max_v_channel_lanes_for_x, j->x, j->v_lanes);
        store_max(m_max_h_channel_lanes_for_y, j->y, j->h_lanes);
    }
}

void graph_layouter::reset_roads_and_junctions()
{
    for (road* r : m_h_roads)
        r->lanes = 0;

    for (road* r : m_v_roads)
        r->lanes = 0;

    for (junction* j : m_junctions)
    {
        // LEFT
        unsigned int combined_lane_changes = j->close_left_lane_changes + j->far_left_lane_changes;
        qreal spacing                      = 0;

        if (combined_lane_changes)
            spacing = (combined_lane_changes - 1) * lane_spacing + junction_padding;

        store_max(m_max_left_junction_spacing_for_x, j->x, spacing);

        // RIGHT
        combined_lane_changes = j->close_right_lane_changes + j->far_right_lane_changes;
        spacing               = 0;

        if (combined_lane_changes)
            spacing = (combined_lane_changes - 1) * lane_spacing + junction_padding;

        store_max(m_max_right_junction_spacing_for_x, j->x, spacing);

        // TOP
        combined_lane_changes = j->close_top_lane_changes + j->far_top_lane_changes;
        spacing               = 0;

        if (combined_lane_changes)
            spacing = (combined_lane_changes - 1) * lane_spacing + junction_padding;

        store_max(m_max_top_junction_spacing_for_y, j->y, spacing);

        // BOTTOM
        combined_lane_changes = j->close_bottom_lane_changes + j->far_bottom_lane_changes;
        spacing               = 0;

        if (combined_lane_changes)
            spacing = (combined_lane_changes - 1) * lane_spacing + junction_padding;

        store_max(m_max_bottom_junction_spacing_for_y, j->y, spacing);

        j->h_lanes = 0;
        j->v_lanes = 0;

        j->close_left_lane_changes   = 0;
        j->close_right_lane_changes  = 0;
        j->close_top_lane_changes    = 0;
        j->close_bottom_lane_changes = 0;

        j->far_left_lane_changes   = 0;
        j->far_right_lane_changes  = 0;
        j->far_top_lane_changes    = 0;
        j->far_bottom_lane_changes = 0;
    }
}

void graph_layouter::calculate_max_channel_dimensions()
{
    auto i = m_max_v_channel_lanes_for_x.constBegin();
    while (i != m_max_v_channel_lanes_for_x.constEnd())
    {
        qreal left_spacing = std::max(v_road_padding + m_max_left_io_padding_for_channel_x.value(i.key()), m_max_left_junction_spacing_for_x.value(i.key()));
        m_max_v_channel_left_spacing_for_x.insert(i.key(), left_spacing);

        qreal right_spacing = std::max(v_road_padding + m_max_right_io_padding_for_channel_x.value(i.key()), m_max_right_junction_spacing_for_x.value(i.key()));
        m_max_v_channel_right_spacing_for_x.insert(i.key(), right_spacing);

        qreal width = left_spacing + right_spacing;

        if (i.value())
            width += (i.value() - 1) * lane_spacing;

        m_max_v_channel_width_for_x.insert(i.key(), std::max(width, minimum_v_channel_width));

        ++i;
    }

    i = m_max_h_channel_lanes_for_y.constBegin();
    while (i != m_max_h_channel_lanes_for_y.constEnd())
    {
        qreal top_spacing = std::max(h_road_padding, m_max_top_junction_spacing_for_y.value(i.key()));
        m_max_h_channel_top_spacing_for_y.insert(i.key(), top_spacing);

        qreal bottom_spacing = std::max(h_road_padding, m_max_bottom_junction_spacing_for_y.value(i.key()));
        m_max_h_channel_bottom_spacing_for_y.insert(i.key(), bottom_spacing);

        qreal height = top_spacing + bottom_spacing;

        if (i.value())
            height += (i.value() - 1) * lane_spacing;

        m_max_h_channel_height_for_y.insert(i.key(), std::max(height, minimum_h_channel_height));

        ++i;
    }
}

void graph_layouter::calculate_gate_offsets()
{
    m_node_offset_for_x.insert(0, 0);
    m_node_offset_for_y.insert(0, 0);

    m_x_values.append(0);
    m_y_values.append(0);

    if (m_max_x_index)
        for (int i = 1; i <= m_max_x_index; ++i)
        {
            qreal offset = m_node_offset_for_x.value(i - 1) + m_max_node_width_for_x.value(i - 1) + std::max(m_max_v_channel_width_for_x.value(i), minimum_v_channel_width);
            m_node_offset_for_x.insert(i, offset);
            m_x_values.append(offset);
        }

    if (m_min_x_index)
        for (int i = -1; i >= m_min_x_index; --i)
        {
            qreal offset = m_node_offset_for_x.value(i + 1) - m_max_node_width_for_x.value(i) - std::max(m_max_v_channel_width_for_x.value(i + 1), minimum_v_channel_width);
            m_node_offset_for_x.insert(i, offset);
            m_x_values.prepend(offset);
        }

    if (m_max_y_index)
        for (int i = 1; i <= m_max_y_index; ++i)
        {
            qreal offset = m_node_offset_for_y.value(i - 1) + m_max_node_height_for_y.value(i - 1) + std::max(m_max_h_channel_height_for_y.value(i), minimum_h_channel_height);
            m_node_offset_for_y.insert(i, offset);
            m_y_values.append(offset);
        }

    if (m_min_y_index)
        for (int i = -1; i >= m_min_y_index; --i)
        {
            qreal offset = m_node_offset_for_y.value(i + 1) - m_max_node_height_for_y.value(i) - std::max(m_max_h_channel_height_for_y.value(i + 1), minimum_h_channel_height);
            m_node_offset_for_y.insert(i, offset);
            m_y_values.prepend(offset);
        }
}

void graph_layouter::place_gates()
{
    for (node_box& box : m_boxes)
    {
        box.item->setPos(m_node_offset_for_x.value(box.x), m_node_offset_for_y.value(box.y));
        m_scene->add_item(box.item);
    }
}

void graph_layouter::draw_nets()
{
    // ROADS AND JUNCTIONS FILLED LEFT TO RIGHT, TOP TO BOTTOM
    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->is_unrouted())
        {
            // HANDLE GLOBAL NETS
            arrow_separated_net* net_item = new arrow_separated_net(n);

            for (const endpoint& src : n->get_sources())
            {
                if (src.get_gate())
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, src.get_gate()->get_id()))
                        continue;

                    for (const node_box& box : m_boxes)
                    {
                        if (box.node == node)
                        {
                            net_item->add_output(box.item->get_output_scene_position(n->get_id(), QString::fromStdString(src.get_pin())));
                            break;
                        }
                    }
                }
            }

            for (const endpoint& dst : n->get_destinations())
            {
                if (dst.get_gate())
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                        continue;

                    for (const node_box& box : m_boxes)
                    {
                        if (box.node == node)
                        {
                            net_item->add_input(box.item->get_input_scene_position(n->get_id(), QString::fromStdString(dst.get_pin())));
                            break;
                        }
                    }
                }
            }

            net_item->finalize();
            m_scene->add_item(net_item);
            continue;
        }

        bool use_label = false;

        for (const endpoint& src : n->get_sources())
        {
            if (src.get_gate()->is_gnd_gate() || src.get_gate()->is_vcc_gate())
            {
                use_label = true;
                break;
            }
        }

        if (use_label)
        {
            labeled_separated_net* net_item = new labeled_separated_net(n, QString::fromStdString(n->get_name()));

            for (const endpoint& src : n->get_sources())
            {
                hal::node node;

                if (m_context->node_for_gate(node, src.get_gate()->get_id()))
                {
                    for (const node_box& box : m_boxes)
                    {
                        if (box.node == node)
                        {
                            net_item->add_output(box.item->get_output_scene_position(n->get_id(), QString::fromStdString(src.get_pin())));
                            break;
                        }
                    }
                }
            }

            for (const endpoint& dst : n->get_destinations())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                    continue;

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                    {
                        net_item->add_input(box.item->get_input_scene_position(n->get_id(), QString::fromStdString(dst.get_pin())));
                        break;
                    }
                }
            }

            net_item->finalize();
            m_scene->add_item(net_item);

            continue;
        }

        bool incomplete_net = false; // PASS TO SHADER ???
        bool src_found = false;
        bool dst_found = false;

        for (const endpoint& src : n->get_sources())
        {
            hal::node node;

            if (m_context->node_for_gate(node, src.get_gate()->get_id()))
                src_found = true;
            else
                incomplete_net = true;
        }

        for (const endpoint& dst : n->get_destinations())
        {
            hal::node node;

            if (m_context->node_for_gate(node, dst.get_gate()->get_id()))
                dst_found = true;
            else
                incomplete_net = true;
        }

        if (src_found && !dst_found)
        {
            arrow_separated_net* net_item = new arrow_separated_net(n);

            for (const endpoint& src : n->get_sources())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, src.get_gate()->get_id()))
                    continue;

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                    {
                        net_item->add_output(box.item->get_output_scene_position(n->get_id(), QString::fromStdString(src.get_pin())));
                        break;
                    }
                }
            }

            net_item->finalize();
            m_scene->add_item(net_item);

            continue;
        }

        if (!src_found && dst_found)
        {
            arrow_separated_net* net_item = new arrow_separated_net(n);

            for (const endpoint& dst : n->get_destinations())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                    continue;

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                    {
                        net_item->add_input(box.item->get_input_scene_position(n->get_id(), QString::fromStdString(dst.get_pin())));
                        break;
                    }
                }
            }

            net_item->finalize();
            m_scene->add_item(net_item);

            continue;
        }

        // HANDLE NORMAL NETS
        used_paths used;
        standard_graphics_net::lines lines;

        // FOR EVERY SRC
        for (const endpoint& src : n->get_sources())
        {
            // FIND SRC BOX
            node_box* src_box = nullptr;
            {
                hal::node node;

                if (!m_context->node_for_gate(node, src.get_gate()->get_id()))
                    continue;

                for (node_box& box : m_boxes)
                    if (box.node == node)
                    {
                        src_box = &box;
                        break;
                    }
            }
            assert(src_box);

            const QPointF src_pin_position = src_box->item->get_output_scene_position(n->get_id(), QString::fromStdString(src.get_pin()));

            // FOR EVERY DST
            for (const endpoint& dst : n->get_destinations())
            {
                // FIND DST BOX
                node_box* dst_box = nullptr;

                hal::node node;

                if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                    continue;

                for (node_box& box : m_boxes)
                    if (box.node == node)
                    {
                        dst_box = &box;
                        break;
                    }

                assert(dst_box);

                QPointF dst_pin_position = dst_box->item->get_input_scene_position(n->get_id(), QString::fromStdString(dst.get_pin()));

                // ROAD BASED DISTANCE (x_distance - 1)
                const int x_distance = dst_box->x - src_box->x - 1;
                const int y_distance = dst_box->y - src_box->y;

                if (!y_distance && v_road_jump_possible(src_box->x + 1, dst_box->x, src_box->y))
                {
                    // SPECIAL CASE INDIRECT HORIZONTAL NEIGHBORS
                    road* dst_v_road = get_v_road(dst_box->x, dst_box->y);

                    qreal x = scene_x_for_v_channel_lane(dst_v_road->x, dst_v_road->lanes);

                    lines.append_h_line(src_pin_position.x(), x, src_pin_position.y());

                    if (src_pin_position.y() < dst_pin_position.y())
                        lines.append_v_line(x, src_pin_position.y(), dst_pin_position.y());
                    else if (src_pin_position.y() > dst_pin_position.y())
                        lines.append_v_line(x, dst_pin_position.y(), src_pin_position.y());

                    lines.append_h_line(x, dst_pin_position.x(), dst_pin_position.y());

                    used.v_roads.insert(dst_v_road);
                    continue;
                }

                road* src_v_road = get_v_road(src_box->x + 1, src_box->y);

                if (!(x_distance || y_distance))
                {
                    // SPECIAL CASE DIRECT HORIZONTAL NEIGHBORS
                    qreal x = scene_x_for_v_channel_lane(src_v_road->x, src_v_road->lanes);

                    lines.append_h_line(src_pin_position.x(), x, src_pin_position.y());

                    if (src_pin_position.y() < dst_pin_position.y())
                        lines.append_v_line(x, src_pin_position.y(), dst_pin_position.y());
                    else if (src_pin_position.y() > dst_pin_position.y())
                        lines.append_v_line(x, dst_pin_position.y(), src_pin_position.y());

                    lines.append_h_line(x, dst_pin_position.x(), dst_pin_position.y());

                    used.v_roads.insert(src_v_road);
                    continue;
                }

                // NORMAL CASE
                // CONNECT SRC TO V ROAD, TRAVEL X DISTANCE, TRAVEL Y DISTANCE, CONNECT V ROAD TO DST
                QPointF current_position(src_pin_position);
                current_position.setX(scene_x_for_v_channel_lane(src_v_road->x, src_v_road->lanes));
                lines.append_h_line(src_pin_position.x(), current_position.x(), src_pin_position.y());
                used.v_roads.insert(src_v_road);

                junction* initial_junction = nullptr;
                int remaining_y_distance   = y_distance;

                if (y_distance < 0)
                {
                    // TRAVEL UP
                    initial_junction = get_junction(src_v_road->x, src_v_road->y);

                    if (src_v_road->lanes != initial_junction->v_lanes)
                    {
                        // R -> J
                        if (src_v_road->lanes < initial_junction->v_lanes)
                        {
                            // POS
                            qreal y = scene_y_for_close_bottom_lane_change(initial_junction->y, initial_junction->close_bottom_lane_changes);
                            lines.append_v_line(current_position.x(), y, current_position.y());
                            current_position.setY(y);
                            used.close_bottom_junctions.insert(initial_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_far_bottom_lane_change(initial_junction->y, initial_junction->far_bottom_lane_changes);
                            lines.append_v_line(current_position.x(), y, current_position.y());
                            current_position.setY(y);
                            used.far_bottom_junctions.insert(initial_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(initial_junction->x, initial_junction->v_lanes);

                        if (current_position.x() < x)
                            lines.append_h_line(current_position.x(), x, current_position.y());
                        else
                            lines.append_h_line(x, current_position.x(), current_position.y());

                        current_position.setX(x);
                    }
                }
                else
                {
                    // TRAVEL DOWN
                    initial_junction = get_junction(src_v_road->x, src_v_road->y + 1);

                    if (src_v_road->lanes != initial_junction->v_lanes)
                    {
                        // R -> J
                        if (src_v_road->lanes < initial_junction->v_lanes)
                        {
                            // POS
                            qreal y = scene_y_for_close_top_lane_change(initial_junction->y, initial_junction->close_top_lane_changes);
                            lines.append_v_line(current_position.x(), current_position.y(), y);
                            current_position.setY(y);
                            used.close_top_junctions.insert(initial_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_far_top_lane_change(initial_junction->y, initial_junction->far_top_lane_changes);
                            lines.append_v_line(current_position.x(), current_position.y(), y);
                            current_position.setY(y);
                            used.far_top_junctions.insert(initial_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(initial_junction->x, initial_junction->v_lanes);

                        if (current_position.x() < x)
                            lines.append_h_line(current_position.x(), x, current_position.y());
                        else
                            lines.append_h_line(x, current_position.x(), current_position.y());

                        current_position.setX(x);
                    }

                    if (!y_distance)
                        remaining_y_distance = -1;
                }

                used.v_junctions.insert(initial_junction);

                junction* last_junction = initial_junction;

                if (x_distance)
                {
                    {
                        qreal y = scene_y_for_h_channel_lane(initial_junction->y, initial_junction->h_lanes);

                        if (current_position.y() < y)
                            lines.append_v_line(current_position.x(), current_position.y(), y);
                        else
                            lines.append_v_line(current_position.x(), y, current_position.y());

                        current_position.setY(y);
                        used.h_junctions.insert(initial_junction);
                    }

                    int remaining_x_distance = x_distance;

                    // TRAVEL REMAINING X DISTANCE
                    while (remaining_x_distance)
                    {
                        road* r     = nullptr;
                        junction* j = nullptr;

                        if (x_distance > 0)
                        {
                            // TRAVEL RIGHT
                            r = get_h_road(last_junction->x, last_junction->y);

                            if (last_junction->h_lanes != r->lanes)
                            {
                                // J -> R
                                if (last_junction->h_lanes < r->lanes)
                                {
                                    // POS
                                    qreal x = scene_x_for_far_right_lane_change(last_junction->x, last_junction->far_right_lane_changes);
                                    lines.append_h_line(current_position.x(), x, current_position.y());
                                    current_position.setX(x);
                                    used.far_right_junctions.insert(last_junction);
                                }
                                else
                                {
                                    // NEG
                                    qreal x = scene_x_for_close_right_lane_change(last_junction->x, last_junction->close_right_lane_changes);
                                    lines.append_h_line(current_position.x(), x, current_position.y());
                                    current_position.setX(x);
                                    used.close_right_junctions.insert(last_junction);
                                }

                                qreal y = scene_y_for_h_channel_lane(r->y, r->lanes);

                                if (current_position.y() < y)
                                    lines.append_v_line(current_position.x(), current_position.y(), y);
                                else
                                    lines.append_v_line(current_position.x(), y, current_position.y());

                                current_position.setY(y);
                            }

                            j = get_junction(last_junction->x + 1, last_junction->y);

                            if (r->lanes != j->h_lanes)
                            {
                                // R -> J
                                if (r->lanes < j->h_lanes)
                                {
                                    // POS
                                    qreal x = scene_x_for_close_left_lane_change(j->x, j->close_left_lane_changes);
                                    lines.append_h_line(current_position.x(), x, current_position.y());
                                    current_position.setX(x);
                                    used.close_left_junctions.insert(j);
                                }
                                else
                                {
                                    // NEG
                                    qreal x = scene_x_for_far_left_lane_change(j->x, j->far_left_lane_changes);
                                    lines.append_h_line(current_position.x(), x, current_position.y());
                                    current_position.setX(x);
                                    used.far_left_junctions.insert(j);
                                }

                                qreal y = scene_y_for_h_channel_lane(j->y, j->h_lanes);

                                // DUPLICATE CODE ?
                                if (current_position.y() < y)
                                    lines.append_v_line(current_position.x(), current_position.y(), y);
                                else
                                    lines.append_v_line(current_position.x(), y, current_position.y());

                                current_position.setY(y);
                            }

                            --remaining_x_distance;
                        }
                        else
                        {
                            // TRAVEL LEFT
                            r = get_h_road(last_junction->x - 1, last_junction->y);

                            if (last_junction->h_lanes != r->lanes)
                            {
                                // J -> R
                                if (last_junction->h_lanes < r->lanes)
                                {
                                    // POS
                                    qreal x = scene_x_for_far_left_lane_change(last_junction->x, last_junction->far_left_lane_changes);
                                    lines.append_h_line(x, current_position.x(), current_position.y());
                                    current_position.setX(x);
                                    used.far_left_junctions.insert(last_junction);
                                }
                                else
                                {
                                    // NEG
                                    qreal x = scene_x_for_close_left_lane_change(last_junction->x, last_junction->close_left_lane_changes);
                                    lines.append_h_line(x, current_position.x(), current_position.y());
                                    current_position.setX(x);
                                    used.close_left_junctions.insert(last_junction);
                                }

                                qreal y = scene_y_for_h_channel_lane(r->y, r->lanes);

                                // DUPLICATE CODE ?
                                if (current_position.y() < y)
                                    lines.append_v_line(current_position.x(), current_position.y(), y);
                                else
                                    lines.append_v_line(current_position.x(), y, current_position.y());

                                current_position.setY(y);
                            }

                            j = get_junction(last_junction->x - 1, last_junction->y);

                            if (r->lanes != j->h_lanes)
                            {
                                // R -> J
                                if (r->lanes < j->h_lanes)
                                {
                                    // POS
                                    qreal x = scene_x_for_close_right_lane_change(j->x, j->close_right_lane_changes);
                                    lines.append_h_line(x, current_position.x(), current_position.y());
                                    current_position.setX(x);
                                    used.close_right_junctions.insert(j);
                                }
                                else
                                {
                                    // NEG
                                    qreal x = scene_x_for_far_right_lane_change(j->x, j->far_right_lane_changes);
                                    lines.append_h_line(x, current_position.x(), current_position.y());
                                    current_position.setX(x);
                                    used.far_right_junctions.insert(j);
                                }

                                qreal y = scene_y_for_h_channel_lane(j->y, j->h_lanes);

                                // DUPLICATE CODE ?
                                if (current_position.y() < y)
                                    lines.append_v_line(current_position.x(), current_position.y(), y);
                                else
                                    lines.append_v_line(current_position.x(), y, current_position.y());

                                current_position.setY(y);
                            }

                            ++remaining_x_distance;
                        }

                        used.h_roads.insert(r);
                        used.h_junctions.insert(j);

                        last_junction = j;
                    }

                    qreal x = scene_x_for_v_channel_lane(last_junction->x, last_junction->v_lanes);

                    if (current_position.x() < x)
                        lines.append_h_line(current_position.x(), x, current_position.y());
                    else
                        lines.append_h_line(x, current_position.x(), current_position.y());

                    current_position.setX(x);
                    used.v_junctions.insert(last_junction);
                }

                // TRAVEL REMAINING Y DISTANCE
                if (remaining_y_distance > 0)
                {
                    while (remaining_y_distance != 1)
                    {
                        // TRAVEL DOWN
                        road* r = get_v_road(last_junction->x, last_junction->y);

                        if (last_junction->v_lanes != r->lanes)
                        {
                            // J -> R
                            if (last_junction->v_lanes < r->lanes)
                            {
                                // POS
                                qreal y = scene_y_for_far_bottom_lane_change(last_junction->y, last_junction->far_bottom_lane_changes);
                                lines.append_v_line(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.far_bottom_junctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = scene_y_for_close_bottom_lane_change(last_junction->y, last_junction->close_bottom_lane_changes);
                                lines.append_v_line(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.close_bottom_junctions.insert(last_junction);
                            }

                            qreal x = scene_x_for_v_channel_lane(r->x, r->lanes);

                            if (current_position.x() < x)
                                lines.append_h_line(current_position.x(), x, current_position.y());
                            else
                                lines.append_h_line(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }

                        junction* j = get_junction(last_junction->x, last_junction->y + 1);

                        if (r->lanes != j->v_lanes)
                        {
                            // R -> J
                            if (r->lanes < j->v_lanes)
                            {
                                // POS
                                qreal y = scene_y_for_close_top_lane_change(j->y, j->close_top_lane_changes);
                                lines.append_v_line(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.close_top_junctions.insert(j);
                            }
                            else
                            {
                                // NEG
                                qreal y = scene_y_for_far_top_lane_change(j->y, j->far_top_lane_changes);
                                lines.append_v_line(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.far_top_junctions.insert(j);
                            }

                            qreal x = scene_x_for_v_channel_lane(j->x, j->v_lanes);

                            if (current_position.x() < x)
                                lines.append_h_line(current_position.x(), x, current_position.y());
                            else
                                lines.append_h_line(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }

                        used.v_roads.insert(r);
                        used.v_junctions.insert(j);

                        last_junction = j;

                        --remaining_y_distance;
                    }
                }
                else
                {
                    while (remaining_y_distance != -1)
                    {
                        // TRAVEL UP
                        road* r = get_v_road(last_junction->x, last_junction->y - 1);

                        if (last_junction->v_lanes != r->lanes)
                        {
                            // J -> R
                            if (last_junction->v_lanes < r->lanes)
                            {
                                // POS
                                qreal y = scene_y_for_far_top_lane_change(last_junction->y, last_junction->far_top_lane_changes);
                                lines.append_v_line(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.far_top_junctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = scene_y_for_close_top_lane_change(last_junction->y, last_junction->close_top_lane_changes);
                                lines.append_v_line(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.close_top_junctions.insert(last_junction);
                            }

                            qreal x = scene_x_for_v_channel_lane(r->x, r->lanes);

                            if (current_position.x() < x)
                                lines.append_h_line(current_position.x(), x, current_position.y());
                            else
                                lines.append_h_line(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }

                        junction* j = get_junction(last_junction->x, last_junction->y - 1);

                        if (r->lanes != j->v_lanes)
                        {
                            // R -> J
                            if (r->lanes < j->v_lanes)
                            {
                                // POS
                                qreal y = scene_y_for_close_bottom_lane_change(j->y, j->close_bottom_lane_changes);
                                lines.append_v_line(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.close_bottom_junctions.insert(j);
                            }
                            else
                            {
                                // NEG
                                qreal y = scene_y_for_far_bottom_lane_change(j->y, j->far_bottom_lane_changes);
                                lines.append_v_line(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.far_bottom_junctions.insert(j);
                            }

                            qreal x = scene_x_for_v_channel_lane(j->x, j->v_lanes);

                            if (current_position.x() < x)
                                lines.append_h_line(current_position.x(), x, current_position.y());
                            else
                                lines.append_h_line(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }

                        used.v_roads.insert(r);
                        used.v_junctions.insert(j);

                        last_junction = j;

                        ++remaining_y_distance;
                    }
                }

                road* dst_road = nullptr;

                if (y_distance > 0)
                {
                    // TRAVEL DOWN
                    dst_road = get_v_road(last_junction->x, last_junction->y);

                    if (last_junction->v_lanes != dst_road->lanes)
                    {
                        // J -> R
                        if (last_junction->v_lanes < dst_road->lanes)
                        {
                            // POS
                            qreal y = scene_y_for_far_bottom_lane_change(last_junction->y, last_junction->far_bottom_lane_changes);
                            lines.append_v_line(current_position.x(), current_position.y(), y);
                            current_position.setY(y);
                            used.far_bottom_junctions.insert(last_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_close_bottom_lane_change(last_junction->y, last_junction->close_bottom_lane_changes);
                            lines.append_v_line(current_position.x(), current_position.y(), y);
                            current_position.setY(y);
                            used.close_bottom_junctions.insert(last_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(dst_road->x, dst_road->lanes);

                        if (current_position.x() < x)
                            lines.append_h_line(current_position.x(), x, current_position.y());
                        else
                            lines.append_h_line(x, current_position.x(), current_position.y());

                        current_position.setX(x);
                    }
                }
                else
                {
                    // TRAVEL UP
                    dst_road = get_v_road(last_junction->x, last_junction->y - 1);

                    if (last_junction->v_lanes != dst_road->lanes)
                    {
                        // J -> R
                        if (last_junction->v_lanes < dst_road->lanes)
                        {
                            // POS
                            qreal y = scene_y_for_far_top_lane_change(last_junction->y, last_junction->far_top_lane_changes);
                            lines.append_v_line(current_position.x(), y, current_position.y());
                            current_position.setY(y);
                            used.far_top_junctions.insert(last_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_close_top_lane_change(last_junction->y, last_junction->close_top_lane_changes);
                            lines.append_v_line(current_position.x(), y, current_position.y());
                            current_position.setY(y);
                            used.close_top_junctions.insert(last_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(dst_road->x, dst_road->lanes);

                        if (current_position.x() < x)
                            lines.append_h_line(current_position.x(), x, current_position.y());
                        else
                            lines.append_h_line(x, current_position.x(), current_position.y());

                        current_position.setX(x);
                    }
                }

                used.v_junctions.insert(last_junction);

                if (current_position.y() < dst_pin_position.y())
                    lines.append_v_line(current_position.x(), current_position.y(), dst_pin_position.y());
                else
                    lines.append_v_line(current_position.x(), dst_pin_position.y(), current_position.y());

                current_position.setY(dst_pin_position.y());

                used.v_roads.insert(dst_road);

                lines.append_h_line(current_position.x(), dst_pin_position.x(), current_position.y());

                current_position = src_pin_position;
            }
        }

        lines.merge_lines();
        standard_graphics_net* graphics_net = new standard_graphics_net(n, lines);
        m_scene->add_item(graphics_net);

        commit_used_paths(used);
    }
}

void graph_layouter::update_scene_rect()
{
    // SCENE RECT STUFF BEHAVES WEIRDLY, FURTHER RESEARCH REQUIRED
    //QRectF rect = m_scene->sceneRect();

    QRectF rect(m_scene->itemsBoundingRect());
    rect.adjust(-200, -200, 200, 200);
    m_scene->setSceneRect(rect);
}

graph_layouter::node_box graph_layouter::create_box(const hal::node& node, const int x, const int y) const
{
    node_box box;
    box.node = node;

    switch (node.type)
    {
        case hal::node_type::module:
        {
            box.item = graphics_factory::create_graphics_module(g_netlist->get_module_by_id(node.id), 0);
            break;
        }
        case hal::node_type::gate:
        {
            box.item = graphics_factory::create_graphics_gate(g_netlist->get_gate_by_id(node.id), 0);
            break;
        }
    }

    box.x = x;
    box.y = y;

    // GATE IO SPACING SHOULD BE CALCULATED HERE, FOR NOW IT IS JUST ASSUMED TO BE THE MINIMUM ACROSS THE BORD
    box.input_padding  = minimum_gate_io_padding;
    box.output_padding = minimum_gate_io_padding;

    return box;
}

bool graph_layouter::box_exists(const int x, const int y) const
{
    for (const graph_layouter::node_box& box : m_boxes)
        if (box.x == x && box.y == y)
            return true;

    return false;
}

bool graph_layouter::h_road_jump_possible(const int x, const int y1, const int y2) const
{
    if (y1 == y2)
        return false;

    int bottom_y   = y1;
    int difference = y1 - y2;

    if (y1 < y2)
    {
        bottom_y   = y2;
        difference = y2 - y1;
    }

    while (difference)
    {
        if (box_exists(x, bottom_y - difference))
            return false;

        --difference;
    }

    return true;
}

bool graph_layouter::h_road_jump_possible(const graph_layouter::road* const r1, const graph_layouter::road* const r2) const
{
    // CONVENIENCE METHOD
    assert(r1 && r2);
    assert(r1->x != r2->x);

    return h_road_jump_possible(r1->x, r1->y, r2->y);
}

bool graph_layouter::v_road_jump_possible(const int x1, const int x2, const int y) const
{
    if (x1 == x2)
        return false;

    int right_x    = x1;
    int difference = x1 - x2;

    if (x1 < x2)
    {
        right_x    = x2;
        difference = x2 - x1;
    }

    while (difference)
    {
        if (box_exists(right_x - difference, y))
            return false;

        --difference;
    }

    return true;
}

bool graph_layouter::v_road_jump_possible(const graph_layouter::road* const r1, const graph_layouter::road* const r2) const
{
    // CONVENIENCE METHOD
    assert(r1 && r2);
    assert(r1->y != r2->y);

    return v_road_jump_possible(r1->x, r2->x, r1->y);
}

graph_layouter::road* graph_layouter::get_h_road(const int x, const int y)
{
    for (graph_layouter::road* r : m_h_roads)
        if (r->x == x && r->y == y)
            return r;

    graph_layouter::road* new_r = new road(x, y);
    m_h_roads.append(new_r);
    return new_r;
}

graph_layouter::road* graph_layouter::get_v_road(const int x, const int y)
{
    for (graph_layouter::road* r : m_v_roads)
        if (r->x == x && r->y == y)
            return r;

    graph_layouter::road* new_r = new road(x, y);
    m_v_roads.append(new_r);
    return new_r;
}

graph_layouter::junction* graph_layouter::get_junction(const int x, const int y)
{
    for (graph_layouter::junction* j : m_junctions)
        if (j->x == x && j->y == y)
            return j;

    graph_layouter::junction* new_j = new junction(x, y);
    m_junctions.append(new_j);
    return new_j;
}

qreal graph_layouter::h_road_height(const unsigned int lanes) const
{
    // LANES COUNTED FROM 1
    qreal height = h_road_padding * 2;

    if (lanes > 1)
        height += (lanes - 1) * lane_spacing;

    return height;
}

qreal graph_layouter::v_road_width(const unsigned int lanes) const
{
    // LANES COUNTED FROM 1
    qreal width = v_road_padding * 2;

    if (lanes > 1)
        width += (lanes - 1) * lane_spacing;

    return width;
}

qreal graph_layouter::scene_y_for_h_channel_lane(const int y, const unsigned int lane) const
{
    // LINES NUMBERED FROM 0
    assert(m_node_offset_for_y.contains(y) || m_node_offset_for_y.contains(y - 1));

    const qreal offset = lane * lane_spacing;

    if (y == 0)
        return m_node_offset_for_y.value(y) - m_max_h_channel_height_for_y.value(y) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
    else
        return m_node_offset_for_y.value(y - 1) + m_max_node_height_for_y.value(y - 1) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
}

qreal graph_layouter::scene_x_for_v_channel_lane(const int x, const unsigned int lane) const
{
    // LINES NUMBERED FROM 0
    assert(m_node_offset_for_x.contains(x) || m_node_offset_for_x.contains(x - 1));

    const qreal offset = lane * lane_spacing;

    if (m_node_offset_for_x.contains(x))
        return m_node_offset_for_x.value(x) - m_max_v_channel_width_for_x.value(x) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
    else
        return m_node_offset_for_x.value(x - 1) + m_max_node_width_for_x.value(x - 1) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
}

qreal graph_layouter::scene_x_for_close_left_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_width_for_x.value(channel_x) + m_max_v_channel_left_spacing_for_x.value(channel_x) - junction_padding
               - lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_left_spacing_for_x.value(channel_x) - junction_padding
               - lane_change * lane_spacing;
}

qreal graph_layouter::scene_x_for_far_left_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_width_for_x.value(channel_x) + m_max_v_channel_left_spacing_for_x.value(channel_x)
               - m_max_left_junction_spacing_for_x.value(channel_x) + lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_left_spacing_for_x.value(channel_x)
               - m_max_left_junction_spacing_for_x.value(channel_x) + lane_change * lane_spacing;
}

qreal graph_layouter::scene_x_for_close_right_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
               - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
}

qreal graph_layouter::scene_x_for_far_right_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
               - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
}

qreal graph_layouter::scene_y_for_close_top_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
               - lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
               - lane_change * lane_spacing;
}

qreal graph_layouter::scene_y_for_far_top_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y)
               - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y)
               - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
}

qreal graph_layouter::scene_y_for_close_bottom_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
               - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
}

qreal graph_layouter::scene_y_for_far_bottom_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
               - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
}

qreal graph_layouter::scene_x_for_close_left_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_close_left_lane_change(j->x, j->close_left_lane_changes);
}

qreal graph_layouter::scene_x_for_far_left_lane_change(const graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_far_left_lane_change(j->x, j->far_left_lane_changes);
}

qreal graph_layouter::scene_x_for_close_right_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_close_right_lane_change(j->x, j->close_right_lane_changes);
}

qreal graph_layouter::scene_x_for_far_right_lane_change(const graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_far_right_lane_change(j->x, j->far_right_lane_changes);
}

qreal graph_layouter::scene_y_for_close_top_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_close_top_lane_change(j->y, j->close_top_lane_changes);
}

qreal graph_layouter::scene_y_for_far_top_lane_change(const graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_far_top_lane_change(j->y, j->far_top_lane_changes);
}

qreal graph_layouter::scene_y_for_close_bottom_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_close_bottom_lane_change(j->y, j->close_bottom_lane_changes);
}

qreal graph_layouter::scene_y_for_far_bottom_lane_change(const graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_far_bottom_lane_change(j->y, j->far_bottom_lane_changes);
}

void graph_layouter::commit_used_paths(const graph_layouter::used_paths& used)
{
    for (road* r : used.h_roads)
        r->lanes += 1;

    for (road* r : used.v_roads)
        r->lanes += 1;

    for (junction* j : used.h_junctions)
        j->h_lanes += 1;

    for (junction* j : used.v_junctions)
        j->v_lanes += 1;

    for (junction* j : used.close_left_junctions)
        j->close_left_lane_changes += 1;

    for (junction* j : used.close_right_junctions)
        j->close_right_lane_changes += 1;

    for (junction* j : used.close_top_junctions)
        j->close_top_lane_changes += 1;

    for (junction* j : used.close_bottom_junctions)
        j->close_bottom_lane_changes += 1;

    for (junction* j : used.far_left_junctions)
        j->far_left_lane_changes += 1;

    for (junction* j : used.far_right_junctions)
        j->far_right_lane_changes += 1;

    for (junction* j : used.far_top_junctions)
        j->far_top_lane_changes += 1;

    for (junction* j : used.far_bottom_junctions)
        j->far_bottom_lane_changes += 1;
}

void graph_layouter::append_non_zero_h_line(standard_graphics_net::lines& lines, const qreal small_x, const qreal big_x, const qreal y)
{
   if (small_x < big_x)
       lines.append_h_line(small_x, big_x, y);
}

void graph_layouter::append_non_zero_v_line(standard_graphics_net::lines& lines, const qreal x, const qreal small_y, const qreal big_y)
{
    if (small_y < big_y)
        lines.append_h_line(x, small_y, big_y);
}

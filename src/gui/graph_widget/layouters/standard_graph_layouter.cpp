#include "graph_widget/layouters/standard_graph_layouter.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/standard_graphics_net.h"
#include "gui/gui_globals.h"

#include "qmath.h"
#include <QSet>
#include <QTime>

const static qreal lane_spacing             = 10;
const static qreal junction_padding         = 10;
const static qreal h_road_padding           = 10;
const static qreal v_road_padding           = 10;
const static qreal minimum_v_channel_width  = 20;
const static qreal minimum_h_channel_height = 20;
const static qreal minimum_gate_io_padding  = 40;

standard_graph_layouter::standard_graph_layouter(const graph_context* const context) : graph_layouter(context)
{
}

const QString standard_graph_layouter::name() const
{
    return "Standard Layouter";
}

const QString standard_graph_layouter::description() const
{
    return "<p>The standard layouting algorithm</p>";
}

void standard_graph_layouter::expand(const u32 from_gate, const u32 via_net, const u32 to_gate)
{
    Q_UNUSED(from_gate)
    Q_UNUSED(via_net)
    Q_UNUSED(to_gate)
}

void standard_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    recalculate_levels();
}

void standard_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    recalculate_levels();
}

void standard_graph_layouter::layout()
{
    // CLEANUP
    m_scene->delete_all_items();
    m_boxes.clear();
    clear_net_layout_data();

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

    // LOGICAL NET LAYOUT
    create_boxes();
    calculate_nets();
    find_max_box_dimensions();
    find_max_channel_lanes();

    // RESET ROAD / JUNCTIONS, ALTERNATIVELY SAVE ALL NECESSARY DATA ON FIRST PASS
    reset_roads_and_junctions();

    // PHYSICAL NET LAYOUT
    calculate_max_channel_dimensions();
    calculate_gate_offsets();
    place_gates();
    update_scene_rect();

    draw_nets();
    m_scene->move_nets_to_background();

    m_scene->setSceneRect(m_scene->itemsBoundingRect());

    m_scene->handle_extern_selection_changed(nullptr);
}

void standard_graph_layouter::create_boxes()
{
    // ZERO GATES
    int level_x = 0;

    // ARTIFICIAL SCOPE TO AVOID SHADOWING
    {
        int x_offset = 0;
        int y        = 0;

        int root = sqrt(m_zero_nodes.size());
        for (const hal::node& node : m_zero_nodes)
        {
            m_boxes.append(create_box(node, level_x + x_offset, y));

            if (x_offset + 1 == root)
            {
                x_offset = 0;
                ++y;
            }
            else
                ++x_offset;
        }
        level_x += root;
    }

    // POSITIVE GATES
    for (QVector<hal::node>& v : m_positive_nodes)
    {
        int x_offset = 0;
        int y        = 0;

        int root = sqrt(v.size());
        for (const hal::node& node : v)
        {
            m_boxes.append(create_box(node, level_x + x_offset, y));

            if (x_offset + 1 == root)
            {
                x_offset = 0;
                ++y;
            }
            else
                ++x_offset;
        }
        level_x += root;
    }

    // NEGATIVE GATES
    level_x = -1;

    for (QVector<hal::node>& v : m_negative_nodes)
    {
        int x_offset = 0;
        int y        = 0;

        int root = sqrt(v.size());
        for (const hal::node& node : v)
        {
            m_boxes.append(create_box(node, level_x + x_offset, y));

            if (abs(x_offset - 1) == root)
            {
                x_offset = 0;
                ++y;
            }
            else
                --x_offset;
        }
        level_x -= root;
    }
}

void standard_graph_layouter::calculate_nets()
{
    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);

        if (!n)
            continue;

        if (n->is_unrouted())
            continue;

        // FIND SRC BOX
        node_box* src_box = nullptr;

        hal::node node;

        if (!m_context->node_for_gate(node, n->get_src().get_gate()->get_id()))
            continue;

        for (node_box& box : m_boxes)
            if (box.node == node)
                src_box = &box;

        if (!src_box)
            continue;

        used_paths used;

        // FOR EVERY DST
        for (const endpoint& dst : n->get_dsts())
        {
            // FIND DST BOX
            node_box* dst_box = nullptr;

            if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                return;

            for (node_box& box : m_boxes)
                if (box.node == node)
                    dst_box = &box;

            if (!dst_box)
                continue;

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

        commit_used_paths(used);
    }
}

void standard_graph_layouter::find_max_box_dimensions()
{
    for (const node_box& box : m_boxes)
    {
        store_max(m_max_node_width_for_x, box.x, box.item->width());
        store_max(m_max_node_height_for_y, box.y, box.item->height());

        store_max(m_max_right_io_padding_for_channel_x, box.x, box.input_padding);
        store_max(m_max_left_io_padding_for_channel_x, box.x + 1, box.output_padding);
    }
}

void standard_graph_layouter::find_max_channel_lanes()
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

void standard_graph_layouter::calculate_max_channel_dimensions()
{
    auto i = m_max_v_channel_lanes_for_x.constBegin();
    while (i != m_max_v_channel_lanes_for_x.constEnd())
    {
        // LEFT
        qreal spacing = std::max(v_road_padding + m_max_left_io_padding_for_channel_x.value(i.key()), m_max_left_junction_spacing_for_x.value(i.key()));
        m_max_v_channel_left_spacing_for_x.insert(i.key(), spacing);

        // RIGHT
        spacing = std::max(v_road_padding + m_max_right_io_padding_for_channel_x.value(i.key()), m_max_right_junction_spacing_for_x.value(i.key()));
        m_max_v_channel_right_spacing_for_x.insert(i.key(), spacing);

        ++i;
    }

    i = m_max_v_channel_lanes_for_x.constBegin();
    while (i != m_max_v_channel_lanes_for_x.constEnd())
    {
        qreal width = m_max_v_channel_left_spacing_for_x.value(i.key()) + m_max_v_channel_right_spacing_for_x.value(i.key());

        if (i.value())
            width += (i.value() - 1) * lane_spacing;

        m_max_v_channel_width_for_x.insert(i.key(), std::max(width, minimum_v_channel_width));

        ++i;
    }

    i = m_max_h_channel_lanes_for_y.constBegin();
    while (i != m_max_h_channel_lanes_for_y.constEnd())
    {
        // TOP
        qreal spacing = std::max(h_road_padding, m_max_top_junction_spacing_for_y.value(i.key()));
        m_max_h_channel_top_spacing_for_y.insert(i.key(), spacing);

        // BOTTOM
        spacing = std::max(h_road_padding, m_max_bottom_junction_spacing_for_y.value(i.key()));
        m_max_h_channel_bottom_spacing_for_y.insert(i.key(), spacing);

        ++i;
    }

    i = m_max_h_channel_lanes_for_y.constBegin();
    while (i != m_max_h_channel_lanes_for_y.constEnd())
    {
        qreal height = m_max_h_channel_top_spacing_for_y.value(i.key()) + m_max_h_channel_bottom_spacing_for_y.value(i.key());

        if (i.value())
            height += (i.value() - 1) * lane_spacing;

        m_max_h_channel_height_for_y.insert(i.key(), std::max(height, minimum_h_channel_height));

        ++i;
    }
}

void standard_graph_layouter::calculate_gate_offsets()
{
    // USE METHOD TO ACCESS MAP AND RETURN MINIMUM VALUE IF NO VALUE IS FOUND
    int min_x = 0;
    int max_x = 0;

    int max_y = 0;

    for (node_box& box : m_boxes)
    {
        if (box.x < min_x)
            min_x = box.x;
        else if (box.x > max_x)
            max_x = box.x;

        if (box.y > max_y)
            max_y = box.y;
    }

    m_node_offset_for_x.insert(0, 0);

    if (max_x)
        for (int i = 1; i <= max_x; ++i)
        {
            //qreal offset = m_gate_offset_for_x.value(i-1) + m_max_gate_width_for_x.value(i-1) + (gate_io_padding * 2) + m_max_v_channel_width_for_x.value(i);
            qreal offset = m_node_offset_for_x.value(i - 1) + m_max_node_width_for_x.value(i - 1) + m_max_v_channel_width_for_x.value(i);
            m_node_offset_for_x.insert(i, offset);
        }

    if (min_x)
        for (int i = -1; i >= min_x; --i)
        {
            //qreal offset = m_gate_offset_for_x.value(i+1) - m_max_gate_width_for_x.value(i) - (gate_io_padding * 2) - m_max_v_channel_width_for_x.value(i+1);
            qreal offset = m_node_offset_for_x.value(i + 1) - m_max_node_width_for_x.value(i) - m_max_v_channel_width_for_x.value(i + 1);
            m_node_offset_for_x.insert(i, offset);
        }

    m_node_offset_for_y.insert(0, 0);

    if (max_y)
        for (int i = 1; i <= max_y; ++i)
        {
            qreal channel_height = 0;

            if (m_max_h_channel_height_for_y.contains(i))
                channel_height = m_max_h_channel_height_for_y.value(i);
            else
                channel_height = minimum_h_channel_height;

            qreal offset = m_node_offset_for_y.value(i - 1) + m_max_node_height_for_y.value(i - 1) + channel_height;
            m_node_offset_for_y.insert(i, offset);
        }
}

void standard_graph_layouter::place_gates()
{
    for (node_box& box : m_boxes)
    {
        box.item->setPos(m_node_offset_for_x.value(box.x), m_node_offset_for_y.value(box.y));
        m_scene->add_item(box.item);
    }
}

void standard_graph_layouter::update_scene_rect()
{
    // SCENE RECT STUFF BEHAVES WEIRDLY, FURTHER RESEARCH REQUIRED

    //    QRectF rect = m_graphics_scene->sceneRect();
    //    rect.adjust(-100, -100, 100, 100);
    //    m_graphics_scene->setSceneRect(rect);

    //    QRectF rect(m_graphics_scene->itemsBoundingRect());

    m_scene->setSceneRect(QRectF());
    QRectF rect = m_scene->sceneRect();
    rect.adjust(-200, -200, 200, 200);
    m_scene->setSceneRect(rect);
}

void standard_graph_layouter::reset_roads_and_junctions()
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

void standard_graph_layouter::draw_nets()
{
    // ROADS AND JUNCTIONS FILLED LEFT TO RIGHT, TOP TO BOTTOM
    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);

        if (!n)
            continue;

        // USE SEPARATE NET VECTORS ???
        if (n->is_unrouted())
        {
            // HANDLE GLOBAL NETS
            io_graphics_net* net_item = new io_graphics_net(n);

            endpoint src_end = n->get_src();

            if (src_end.get_gate())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, src_end.get_gate()->get_id()))
                    continue;

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                    {
                        net_item->setPos(box.item->get_output_scene_position(n->get_id(), QString::fromStdString(src_end.pin_type)));
                        net_item->add_output();
                    }
                }
            }

            for (const endpoint& dst_end : n->get_dsts())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, dst_end.get_gate()->get_id()))
                    continue;

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                        net_item->add_input(box.item->get_input_scene_position(n->get_id(), QString::fromStdString(dst_end.pin_type)));
                }
            }

            net_item->finalize();
            m_scene->add_item(net_item);
            continue;
        }

        if (n->get_src().gate)
        {
            if (n->get_src().gate->is_global_gnd_gate() || n->get_src().gate->is_global_vcc_gate())
            {
                // HANDLE SEPARATED NETS
                hal::node node;

                if (!m_context->node_for_gate(node, n->get_src().get_gate()->get_id()))
                    continue;

                separated_graphics_net* net_item = new separated_graphics_net(n, QString::fromStdString(n->get_name()));

                for (const node_box& box : m_boxes)
                {
                    if (box.node == node)
                    {
                        net_item->setPos(box.item->get_output_scene_position(n->get_id(), QString::fromStdString(n->get_src().pin_type)));
                        net_item->add_output();
                    }
                }

                for (endpoint& dst_end : n->get_dsts())
                {
                    if (!m_context->node_for_gate(node, dst_end.get_gate()->get_id()))
                        continue;

                    for (const node_box& box : m_boxes)
                    {
                        if (box.node == node)
                        {
                            net_item->add_input(box.item->get_input_scene_position(n->get_id(), QString::fromStdString(dst_end.pin_type)));
                        }
                    }
                }

                net_item->finalize();
                m_scene->add_item(net_item);

                continue;
            }
        }

        // HANDLE NORMAL NETS
        // FIND SRC BOX
        node_box* src_box = nullptr;

        hal::node node;

        if (!m_context->node_for_gate(node, n->get_src().get_gate()->get_id()))
            continue;

        for (node_box& box : m_boxes)
            if (box.node == node)
                src_box = &box;

        if (!src_box)
            continue;

        used_paths used;

        const QPointF src_pin_position = src_box->item->get_output_scene_position(n->get_id(), QString::fromStdString(n->get_src().pin_type));
        standard_graphics_net::lines lines;
        lines.src_x = src_pin_position.x();
        lines.src_y = src_pin_position.y();

        // FOR EVERY DST
        for (const endpoint& dst : n->get_dsts())
        {
            // FIND DST BOX
            node_box* dst_box = nullptr;

            hal::node node;

            if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                continue;

            for (node_box& box : m_boxes)
                if (box.node == node)
                    dst_box = &box;

            if (!dst_box)
                continue;

            QPointF dst_pin_position = dst_box->item->get_input_scene_position(n->get_id(), QString::fromStdString(dst.pin_type));

            // ROAD BASED DISTANCE (x_distance - 1)
            const int x_distance = dst_box->x - src_box->x - 1;
            const int y_distance = dst_box->y - src_box->y;

            if (!y_distance && v_road_jump_possible(src_box->x + 1, dst_box->x, src_box->y))
            {
                // SPECIAL CASE INDIRECT HORIZONTAL NEIGHBORS
                road* dst_v_road = get_v_road(dst_box->x, dst_box->y);

                qreal x = scene_x_for_v_channel_lane(dst_v_road->x, dst_v_road->lanes);
                lines.h_lines.append(standard_graphics_net::h_line{src_pin_position.x(), x, src_pin_position.y()});

                if (src_pin_position.y() < dst_pin_position.y())
                    lines.v_lines.append(standard_graphics_net::v_line{x, src_pin_position.y(), dst_pin_position.y()});
                else
                    lines.v_lines.append(standard_graphics_net::v_line{x, dst_pin_position.y(), src_pin_position.y()});

                lines.h_lines.append(standard_graphics_net::h_line{x, dst_pin_position.x(), dst_pin_position.y()});

                used.v_roads.insert(dst_v_road);
                continue;
            }

            road* src_v_road = get_v_road(src_box->x + 1, src_box->y);

            if (!(x_distance || y_distance))
            {
                // SPECIAL CASE DIRECT HORIZONTAL NEIGHBORS
                qreal x = scene_x_for_v_channel_lane(src_v_road->x, src_v_road->lanes);
                lines.h_lines.append(standard_graphics_net::h_line{src_pin_position.x(), x, src_pin_position.y()});

                if (src_pin_position.y() < dst_pin_position.y())
                    lines.v_lines.append(standard_graphics_net::v_line{x, src_pin_position.y(), dst_pin_position.y()});
                else
                    lines.v_lines.append(standard_graphics_net::v_line{x, dst_pin_position.y(), src_pin_position.y()});

                lines.h_lines.append(standard_graphics_net::h_line{x, dst_pin_position.x(), dst_pin_position.y()});

                used.v_roads.insert(src_v_road);
                continue;
            }

            // NORMAL CASE
            // CONNECT SRC TO V ROAD, TRAVEL X DISTANCE, TRAVEL Y DISTANCE, CONNECT V ROAD TO DST
            QPointF current_position(src_pin_position);
            current_position.setX(scene_x_for_v_channel_lane(src_v_road->x, src_v_road->lanes));
            lines.h_lines.append(standard_graphics_net::h_line{src_pin_position.x(), current_position.x(), src_pin_position.y()});
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
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                        current_position.setY(y);
                        used.close_bottom_junctions.insert(initial_junction);
                    }
                    else
                    {
                        // NEG
                        qreal y = scene_y_for_far_bottom_lane_change(initial_junction->y, initial_junction->far_bottom_lane_changes);
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                        current_position.setY(y);
                        used.far_bottom_junctions.insert(initial_junction);
                    }

                    qreal x = scene_x_for_v_channel_lane(initial_junction->x, initial_junction->v_lanes);

                    if (current_position.x() < x)
                        lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                    else
                        lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                        current_position.setY(y);
                        used.close_top_junctions.insert(initial_junction);
                    }
                    else
                    {
                        // NEG
                        qreal y = scene_y_for_far_top_lane_change(initial_junction->y, initial_junction->far_top_lane_changes);
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                        current_position.setY(y);
                        used.far_top_junctions.insert(initial_junction);
                    }

                    qreal x = scene_x_for_v_channel_lane(initial_junction->x, initial_junction->v_lanes);

                    // DUPLICATE CODE ?
                    if (current_position.x() < x)
                        lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                    else
                        lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

                    current_position.setX(x);
                }

                if (!y_distance)
                    remaining_y_distance = -1;
            }

            used.v_junctions.insert(initial_junction);

            junction* last_junction = initial_junction;

            if (x_distance)
            {
                qreal y = scene_y_for_h_channel_lane(initial_junction->y, initial_junction->h_lanes);

                if (current_position.y() < y)
                    lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                else
                    lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});

                current_position.setY(y);
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
                            // J -> R
                            if (last_junction->h_lanes < r->lanes)
                            {
                                // POS
                                qreal x = scene_x_for_far_right_lane_change(last_junction->x, last_junction->far_right_lane_changes);
                                lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                                current_position.setX(x);
                                used.far_right_junctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal x = scene_x_for_close_right_lane_change(last_junction->x, last_junction->close_right_lane_changes);
                                lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                                current_position.setX(x);
                                used.close_right_junctions.insert(last_junction);
                            }

                            qreal y = scene_y_for_h_channel_lane(r->y, r->lanes);

                            if (current_position.y() < y)
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            else
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});

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
                                lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                                current_position.setX(x);
                                used.close_left_junctions.insert(j);
                            }
                            else
                            {
                                // NEG
                                qreal x = scene_x_for_far_left_lane_change(j->x, j->far_left_lane_changes);
                                lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                                current_position.setX(x);
                                used.far_left_junctions.insert(j);
                            }

                            qreal y = scene_y_for_h_channel_lane(j->y, j->h_lanes);

                            // DUPLICATE CODE ?
                            if (current_position.y() < y)
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            else
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});

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
                                lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});
                                current_position.setX(x);
                                used.far_left_junctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal x = scene_x_for_close_left_lane_change(last_junction->x, last_junction->close_left_lane_changes);
                                lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});
                                current_position.setX(x);
                                used.close_left_junctions.insert(last_junction);
                            }

                            qreal y = scene_y_for_h_channel_lane(r->y, r->lanes);

                            // DUPLICATE CODE ?
                            if (current_position.y() < y)
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            else
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});

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
                                lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});
                                current_position.setX(x);
                                used.close_right_junctions.insert(j);
                            }
                            else
                            {
                                // NEG
                                qreal x = scene_x_for_far_right_lane_change(j->x, j->far_right_lane_changes);
                                lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});
                                current_position.setX(x);
                                used.far_right_junctions.insert(j);
                            }

                            qreal y = scene_y_for_h_channel_lane(j->y, j->h_lanes);

                            // DUPLICATE CODE ?
                            if (current_position.y() < y)
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            else
                                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});

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
                    lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                else
                    lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            current_position.setY(y);
                            used.far_bottom_junctions.insert(last_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_close_bottom_lane_change(last_junction->y, last_junction->close_bottom_lane_changes);
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            current_position.setY(y);
                            used.close_bottom_junctions.insert(last_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(r->x, r->lanes);

                        if (current_position.x() < x)
                            lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                        else
                            lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            current_position.setY(y);
                            used.close_top_junctions.insert(j);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_far_top_lane_change(j->y, j->far_top_lane_changes);
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                            current_position.setY(y);
                            used.far_top_junctions.insert(j);
                        }

                        qreal x = scene_x_for_v_channel_lane(j->x, j->v_lanes);

                        if (current_position.x() < x)
                            lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                        else
                            lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                            current_position.setY(y);
                            used.far_top_junctions.insert(last_junction);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_close_top_lane_change(last_junction->y, last_junction->close_top_lane_changes);
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                            current_position.setY(y);
                            used.close_top_junctions.insert(last_junction);
                        }

                        qreal x = scene_x_for_v_channel_lane(r->x, r->lanes);

                        if (current_position.x() < x)
                            lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                        else
                            lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                            current_position.setY(y);
                            used.close_bottom_junctions.insert(j);
                        }
                        else
                        {
                            // NEG
                            qreal y = scene_y_for_far_bottom_lane_change(j->y, j->far_bottom_lane_changes);
                            lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                            current_position.setY(y);
                            used.far_bottom_junctions.insert(j);
                        }

                        qreal x = scene_x_for_v_channel_lane(j->x, j->v_lanes);

                        if (current_position.x() < x)
                            lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                        else
                            lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                        current_position.setY(y);
                        used.far_bottom_junctions.insert(last_junction);
                    }
                    else
                    {
                        // NEG
                        qreal y = scene_y_for_close_bottom_lane_change(last_junction->y, last_junction->close_bottom_lane_changes);
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), y});
                        current_position.setY(y);
                        used.close_bottom_junctions.insert(last_junction);
                    }

                    qreal x = scene_x_for_v_channel_lane(dst_road->x, dst_road->lanes);

                    if (current_position.x() < x)
                        lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                    else
                        lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

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
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                        current_position.setY(y);
                        used.far_top_junctions.insert(last_junction);
                    }
                    else
                    {
                        // NEG
                        qreal y = scene_y_for_close_top_lane_change(last_junction->y, last_junction->close_top_lane_changes);
                        lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), y, current_position.y()});
                        current_position.setY(y);
                        used.close_top_junctions.insert(last_junction);
                    }

                    qreal x = scene_x_for_v_channel_lane(dst_road->x, dst_road->lanes);

                    if (current_position.x() < x)
                        lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), x, current_position.y()});
                    else
                        lines.h_lines.append(standard_graphics_net::h_line{x, current_position.x(), current_position.y()});

                    current_position.setX(x);
                }
            }

            used.v_junctions.insert(last_junction);

            if (current_position.y() < dst_pin_position.y())
                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), current_position.y(), dst_pin_position.y()});
            else
                lines.v_lines.append(standard_graphics_net::v_line{current_position.x(), dst_pin_position.y(), current_position.y()});

            current_position.setY(dst_pin_position.y());

            used.v_roads.insert(dst_road);

            lines.h_lines.append(standard_graphics_net::h_line{current_position.x(), dst_pin_position.x(), current_position.y()});

            current_position = src_pin_position;
        }

        standard_graphics_net* graphics_net = new standard_graphics_net(n, lines);
        graphics_net->setPos(src_pin_position);
        m_scene->add_item(graphics_net);

        commit_used_paths(used);
    }
}

void standard_graph_layouter::clear_net_layout_data()
{
    for (const standard_graph_layouter::road* r : m_h_roads)
        delete r;
    m_h_roads.clear();

    for (const standard_graph_layouter::road* r : m_v_roads)
        delete r;
    m_v_roads.clear();

    for (const standard_graph_layouter::junction* j : m_junctions)
        delete j;
    m_junctions.clear();
}

standard_graph_layouter::node_box standard_graph_layouter::create_box(const hal::node& node, const int x, const int y) const
{
    node_box box;
    box.node = node;

    switch (node.type)
    {
        case hal::node_type::module:
        {
            box.item = graphics_factory::create_graphics_module(g_netlist->get_module_by_id(node.id), 0);    // USE VARIABLE
            break;
        }
        case hal::node_type::gate:
        {
            box.item = graphics_factory::create_graphics_gate(g_netlist->get_gate_by_id(node.id), 0);    // USE VARIABLE
            break;
        }
    }

    box.x = x;
    box.y = y;

    // GATE IO SPACING SHOULD BE CALCULATED HERE, FOR NOW IT IS JUST ASSUMED TO BE 40 ACROSS THE BORD
    box.input_padding  = minimum_gate_io_padding;
    box.output_padding = minimum_gate_io_padding;

    return box;
}

void standard_graph_layouter::add_gate(const u32 gate_id, const int level)
{
    m_node_levels.insert(hal::node{hal::node_type::gate, gate_id}, level);

    if (level == 0)
    {
        m_zero_nodes.append(hal::node{hal::node_type::gate, gate_id});
    }
    else
    {
        if (level > 0)
        {
            if (level > m_positive_nodes.size())
            {
                m_positive_nodes.append(QVector<hal::node>());
                m_positive_nodes.last().append(hal::node{hal::node_type::gate, gate_id});
            }
            else
                m_positive_nodes[level - 1].append(hal::node{hal::node_type::gate, gate_id});
        }
        else    // if (new_level < 0)
        {
            int abs_level = abs(level);

            if (abs_level > m_negative_nodes.size())
            {
                m_negative_nodes.append(QVector<hal::node>());
                m_negative_nodes.last().append(hal::node{hal::node_type::gate, gate_id});
            }
            else
                m_negative_nodes[abs_level - 1].append(hal::node{hal::node_type::gate, gate_id});
        }
    }
}

bool standard_graph_layouter::box_exists(const int x, const int y) const
{
    for (const standard_graph_layouter::node_box& box : m_boxes)
        if (box.x == x && box.y == y)
            return true;

    return false;
}

bool standard_graph_layouter::h_road_jump_possible(const int x, const int y1, const int y2) const
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

bool standard_graph_layouter::h_road_jump_possible(const standard_graph_layouter::road* const r1, const standard_graph_layouter::road* const r2) const
{
    // CONVENIENCE METHOD
    assert(r1 && r2);

    if (r1->x != r2->x)    // CHECK OR ASSERT ???
        return false;

    return h_road_jump_possible(r1->x, r1->y, r2->y);
}

bool standard_graph_layouter::v_road_jump_possible(const int x1, const int x2, const int y) const
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

bool standard_graph_layouter::v_road_jump_possible(const standard_graph_layouter::road* const r1, const standard_graph_layouter::road* const r2) const
{
    // CONVENIENCE METHOD
    assert(r1 && r2);

    if (r1->y != r2->y)    // CHECK OR ASSERT ???
        return false;

    return v_road_jump_possible(r1->x, r2->x, r1->y);
}

standard_graph_layouter::road* standard_graph_layouter::get_h_road(const int x, const int y)
{
    for (standard_graph_layouter::road* r : m_h_roads)
        if (r->x == x && r->y == y)
            return r;

    standard_graph_layouter::road* new_r = new road(x, y);
    m_h_roads.append(new_r);
    return new_r;
}

standard_graph_layouter::road* standard_graph_layouter::get_v_road(const int x, const int y)
{
    for (standard_graph_layouter::road* r : m_v_roads)
        if (r->x == x && r->y == y)
            return r;

    standard_graph_layouter::road* new_r = new road(x, y);
    m_v_roads.append(new_r);
    return new_r;
}

standard_graph_layouter::junction* standard_graph_layouter::get_junction(const int x, const int y)
{
    for (standard_graph_layouter::junction* j : m_junctions)
        if (j->x == x && j->y == y)
            return j;

    standard_graph_layouter::junction* new_j = new junction(x, y);
    m_junctions.append(new_j);
    return new_j;
}

qreal standard_graph_layouter::h_road_height(const unsigned int lanes) const
{
    // LANES COUNTED FROM 1
    qreal height = h_road_padding * 2;

    if (lanes > 1)
        height += (lanes - 1) * lane_spacing;

    return height;
}

qreal standard_graph_layouter::v_road_width(const unsigned int lanes) const
{
    // LANES COUNTED FROM 1
    qreal width = v_road_padding * 2;

    if (lanes > 1)
        width += (lanes - 1) * lane_spacing;

    return width;
}

qreal standard_graph_layouter::scene_y_for_h_channel_lane(const int y, const unsigned int lane) const
{
    // LINES NUMBERED FROM 0
    assert(m_node_offset_for_y.contains(y) || m_node_offset_for_y.contains(y - 1));

    const qreal offset = lane * lane_spacing;

    if (y == 0)
        return m_node_offset_for_y.value(y) - m_max_h_channel_height_for_y.value(y) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
    else
        return m_node_offset_for_y.value(y - 1) + m_max_node_height_for_y.value(y - 1) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
}

qreal standard_graph_layouter::scene_x_for_v_channel_lane(const int x, const unsigned int lane) const
{
    // LINES NUMBERED FROM 0
    assert(m_node_offset_for_x.contains(x) || m_node_offset_for_x.contains(x - 1));

    const qreal offset = lane * lane_spacing;

    if (m_node_offset_for_x.contains(x))
        return m_node_offset_for_x.value(x) - m_max_v_channel_width_for_x.value(x) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
    else
        return m_node_offset_for_x.value(x - 1) + m_max_node_width_for_x.value(x - 1) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
}

qreal standard_graph_layouter::scene_x_for_close_left_lane_change(const int channel_x, unsigned int lane_change) const
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

qreal standard_graph_layouter::scene_x_for_far_left_lane_change(const int channel_x, unsigned int lane_change) const
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

qreal standard_graph_layouter::scene_x_for_close_right_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
               - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_x_for_far_right_lane_change(const int channel_x, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

    if (m_node_offset_for_x.contains(channel_x))
        return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
    else
        return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
               - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_y_for_close_top_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
               - lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
               - lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_y_for_far_top_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y)
               - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y)
               - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_y_for_close_bottom_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
               - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_y_for_far_bottom_lane_change(const int channel_y, unsigned int lane_change) const
{
    // LANE CHANGES COUNTED FROM 0
    if (channel_y == 0)
        return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
    else
        return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
               - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
}

qreal standard_graph_layouter::scene_x_for_close_left_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_close_left_lane_change(j->x, j->close_left_lane_changes);
}

qreal standard_graph_layouter::scene_x_for_far_left_lane_change(const standard_graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_far_left_lane_change(j->x, j->far_left_lane_changes);
}

qreal standard_graph_layouter::scene_x_for_close_right_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_close_right_lane_change(j->x, j->close_right_lane_changes);
}

qreal standard_graph_layouter::scene_x_for_far_right_lane_change(const standard_graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_x_for_far_right_lane_change(j->x, j->far_right_lane_changes);
}

qreal standard_graph_layouter::scene_y_for_close_top_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_close_top_lane_change(j->y, j->close_top_lane_changes);
}

qreal standard_graph_layouter::scene_y_for_far_top_lane_change(const standard_graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_far_top_lane_change(j->y, j->far_top_lane_changes);
}

qreal standard_graph_layouter::scene_y_for_close_bottom_lane_change(const junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_close_bottom_lane_change(j->y, j->close_bottom_lane_changes);
}

qreal standard_graph_layouter::scene_y_for_far_bottom_lane_change(const standard_graph_layouter::junction* const j) const
{
    // CONVENIENCE METHOD
    assert(j);

    return scene_y_for_far_bottom_lane_change(j->y, j->far_bottom_lane_changes);
}

void standard_graph_layouter::recalculate_levels()
{
    // INEFFICIENT BUT EASIER, OPTIMIZE LATER...
    m_node_levels.clear();
    m_zero_nodes.clear();
    m_positive_nodes.clear();
    m_negative_nodes.clear();

    QVector<hal::node> unvisited;

    for (u32 id : m_context->gates())
        unvisited.append(hal::node{hal::node_type::gate, id});

    for (u32 id : m_context->modules())
        unvisited.append(hal::node{hal::node_type::module, id});

    // LEVEL ZERO
    QSet<u32> level_zero_gates;

    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->get_src().get_gate())
            level_zero_gates.insert(n->get_src().get_gate()->get_id());

        for (const endpoint& e : n->get_dsts())
            if (e.get_gate())
                level_zero_gates.remove(e.get_gate()->get_id());
    }

    for (const u32 id : m_context->nets())
    {
        // SEEMS HACKY
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->is_unrouted())
            for (const endpoint& e : n->get_dsts())
                if (e.get_gate())
                    level_zero_gates.insert(e.get_gate()->get_id());
    }

    for (const u32 id : level_zero_gates)
    {
        hal::node node;

        if (!m_context->node_for_gate(node, id))
            continue;

        if (!m_zero_nodes.contains(node))
        {
            m_node_levels.insert(node, 0);
            m_zero_nodes.append(node);

            unvisited.removeOne(node);
        }
    }

    // REMAINING LEVELS
    int level = 1;

    while (!unvisited.isEmpty())
    {
        QVector<hal::node> visited;

        for (const hal::node& node : unvisited)
        {
            switch (node.type)
            {
                case hal::node_type::module:
                {
                    std::shared_ptr<module> m = g_netlist->get_module_by_id(node.id);

                    for (const std::shared_ptr<net>& n : m->get_input_nets())
                    {
                        hal::node src_node;

                        if (n->is_unrouted())
                            continue;

                        if (!m_context->node_for_gate(src_node, n->get_src().gate->get_id()))
                            continue;

                        if (m_node_levels.contains(src_node))
                        {
                            visited.append(node);
                            break;
                        }
                    }

                    break;
                }
                case hal::node_type::gate:
                {
                    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(node.id);

                    for (std::shared_ptr<net> n : g->get_fan_in_nets())
                    {
                        hal::node src_node;

                        if (n->is_unrouted())
                            continue;

                        if (!m_context->node_for_gate(src_node, n->get_src().gate->get_id()))
                            continue;

                        if (m_node_levels.contains(src_node))
                        {
                            visited.append(node);
                            break;
                        }
                    }

                    break;
                }
            }
        }

        if (visited.isEmpty())
        {
            // HACK SOLUTIONS TM
            // PROBABLY GOING TO NEED SOME KIND OF GROUP SYSTEM
            for (const hal::node& node : unvisited)
            {
                m_node_levels.insert(node, 0);
                m_zero_nodes.append(node);
            }

            return;
        }
        else
        {
            if (m_positive_nodes.size() < level)
                m_positive_nodes.append(QVector<hal::node>());

            for (const hal::node& node : visited)
            {
                m_node_levels.insert(node, level);
                m_positive_nodes[level - 1].append(node);

                unvisited.removeOne(node);
            }

            ++level;
        }
    }
}

template<typename T1, typename T2>
void standard_graph_layouter::store_max(QMap<T1, T2>& map, T1 key, T2 value)
{
    if (map.contains(key))
        if (map.value(key) >= value)
            return;

    map.insert(key, value);
}

void standard_graph_layouter::commit_used_paths(const standard_graph_layouter::used_paths& used)
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

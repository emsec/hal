#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/implementations/qpoint_extension.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include <qmath.h>
#include <QDebug>
#include <QElapsedTimer>

uint qHash(const QPoint& p)
{
    uint retval = ( p.x() << 16);
    retval |= ( p.y() & 0xFFFF);
    return retval;
}


namespace hal
{
    uint qHash(const hal::node& n)
    {
        uint retval = ( n.id << 1);
        if (n.type == hal::node_type::module) ++retval;
        return retval;
    }

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

    GraphLayouter::GraphLayouter(const GraphContext* const context, QObject* parent) : QObject(parent), m_scene(new GraphicsScene(this)), m_context(context), m_done(false)
    {
        SelectionDetailsWidget* details = g_content_manager->getSelectionDetailsWidget();
        if (details) connect(details, &SelectionDetailsWidget::triggerHighlight, m_scene, &GraphicsScene::handleHighlight);
    }

    GraphicsScene* GraphLayouter::scene() const
    {
        return m_scene;
    }

    const QMap<hal::node, QPoint> GraphLayouter::node_to_position_map() const
    {
        return m_node_to_position_map;
    }

    const QMap<QPoint, hal::node> GraphLayouter::position_to_node_map() const
    {
        return m_position_to_node_map;
    }

    void GraphLayouter::set_node_position(const hal::node& n, const QPoint& p)
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

    void GraphLayouter::swap_node_positions(const hal::node& n1, const hal::node& n2)
    {
        assert(m_node_to_position_map.contains(n1));
        assert(m_node_to_position_map.contains(n2));

        QPoint p1 = m_node_to_position_map.value(n1);
        QPoint p2 = m_node_to_position_map.value(n2);

        m_node_to_position_map.insert(n1, p2);    // implicit replace
        m_node_to_position_map.insert(n2, p1);

        m_position_to_node_map.insert(p1, n2);
        m_position_to_node_map.insert(p2, n1);
    }

    void GraphLayouter::remove_node_from_maps(const hal::node& n)
    {
        if (m_node_to_position_map.contains(n))
        {
            QPoint old_p = m_node_to_position_map.value(n);
            m_node_to_position_map.remove(n);
            m_position_to_node_map.remove(old_p);
        }
    }

    int GraphLayouter::min_x_index() const
    {
        return m_min_x_index;
    }

    int GraphLayouter::min_y_index() const
    {
        return m_min_y_index;
    }

    bool GraphLayouter::done() const
    {
        return m_done;
    }

    QVector<qreal> GraphLayouter::x_values() const
    {
        return m_x_values;
    }

    QVector<qreal> GraphLayouter::y_values() const
    {
        return m_y_values;
    }

    qreal GraphLayouter::max_node_width() const
    {
        return m_max_node_width;
    }

    qreal GraphLayouter::max_node_height() const
    {
        return m_max_node_height;
    }

    qreal GraphLayouter::default_grid_width() const
    {
        return m_max_node_width + minimum_v_channel_width;
    }

    qreal GraphLayouter::default_grid_height() const
    {
        return m_max_node_height + minimum_h_channel_height;
    }

    void GraphLayouter::alternateLayout()
    {
   //     m_scene->delete_all_items();
  //      clear_layout_data();

  //      create_boxes();
 //       getWireHash();
  //      calculate_nets();
        find_max_box_dimensions();
        findMaxChannelLanes();
  //      reset_roads_and_junctions();
  //      calculate_max_channel_dimensions();
        calculateJunctionMinDistance();
        calculateGateOffsets();
  //      calculate_gate_offsets();
        placeGates();
//        place_gates();
        m_done = true;
  //      draw_nets();
        drawNets();
        update_scene_rect();

        m_scene->move_nets_to_background();
        m_scene->handle_extern_selection_changed(nullptr);

    #ifdef GUI_DEBUG_GRID
        m_scene->debug_set_layouter_grid(x_values(), y_values(), default_grid_height(), default_grid_width());
    #endif
    }

    void GraphLayouter::layout()
    {
        QElapsedTimer timer;
        timer.start();
        m_scene->delete_all_items();
        clear_layout_data();

        create_boxes();
        getWireHash();
        if (!mWireHash.isEmpty() && g_settings_manager->get("graph_view/layout_nets").toBool())
        {
            alternateLayout();
            qDebug() << "elapsed time (experimental new) layout [ms]" << timer.elapsed();
            return;
        }
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
        qDebug() << "elapsed time (classic) layout [ms]" << timer.elapsed();
    }

    void GraphLayouter::clear_layout_data()
    {
        m_done = false;

        m_boxes.clear();
        m_boxNode.clear();
        m_boxPosition.clear();

        for (const GraphLayouter::road* r : m_h_roads.values())
            delete r;
        m_h_roads.clear();

        for (const GraphLayouter::road* r : m_v_roads.values())
            delete r;
        m_v_roads.clear();

        for (const GraphLayouter::junction* j : m_junctions.values())
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

        mConnectionMetric.clearAll();
        mJunctionHash.clearAll();
        mEndpointHash.clear();
        mWireHash.clear();
        mJunctionEntries.clear();
        mSeparatedWidth.clear();
        mCoordX.clear();
        mCoordY.clear();
        mJunctionMinDistanceY.clear();
        mWireEndpoint.clear();
    }

    void GraphLayouter::create_boxes()
    {
        QMap<QPoint, hal::node>::const_iterator i = position_to_node_map().constBegin();
        while (i != position_to_node_map().constEnd())
        {
            int n = m_boxes.size();
            m_boxes.append(create_box(i.value(), i.key().x(), i.key().y()));
            m_boxNode.insert(i.value(),n);
            m_boxPosition.insert(i.key(),n);
            ++i;
        }
    }

    void GraphLayouter::getWireHash()
    {
        for (const u32 id : m_context->nets())
        {
            Net* n = g_netlist->get_net_by_id(id);
            Q_ASSERT(n);

            QList<NetLayoutPoint> srcPoints;
            QList<NetLayoutPoint> dstPoints;

            mWireEndpoint[id] = EndpointList();

            for (const Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                node srcNode;

                if (!m_context->node_for_gate(srcNode, src->get_gate()->get_id()))
                    continue;

                node_box* srcBox = nullptr;
                int ibox = m_boxNode.value(srcNode,-1);
                if (ibox >= 0)
                    srcBox = & (m_boxes[ibox]);

                if (srcBox)
                {
                    NetLayoutPoint srcPnt(srcBox->x+1,2*srcBox->y);
                    srcPoints.append(srcPnt);
                    mWireEndpoint[id].addSource(srcPnt);
                }
            }

            for (const Endpoint* dst : n->get_destinations())
            {
                node dstNode;

                if (!m_context->node_for_gate(dstNode, dst->get_gate()->get_id()))
                    continue;

                node_box* dstBox = nullptr;
                int ibox = m_boxNode.value(dstNode,-1);
                if (ibox >= 0)
                    dstBox = & (m_boxes[ibox]);

                if (dstBox)
                {
                    NetLayoutPoint dstPnt(dstBox->x,2*dstBox->y);
                    dstPoints.append(dstPnt);
                    mWireEndpoint[id].addDestination(dstPnt);
                }
            }

            if (isConstNet(n))
                mWireEndpoint[id].setConstantLevel();

            const EndpointList& epl = mWireEndpoint.value(id);

            switch (epl.hasEndpoint()) {
            case EndpointList::OnlySource:
            case EndpointList::OnlyDestination:
            case EndpointList::ConstantLevel:
            {
                int ipnt = 0;
                for (const NetLayoutPoint& pnt : epl)
                {
                    bool isInput = epl.isInput(ipnt++);
                    SeparatedGraphicsNet* net_item = epl.hasEndpoint() == EndpointList::ConstantLevel
                            ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n,QString::fromStdString(n->get_name())))
                            : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));
                    if (isInput)
                        mSeparatedWidth[pnt].requireInputSpace(net_item->input_width()+lane_spacing);
                    else
                        mSeparatedWidth[pnt].requireOutputSpace(net_item->output_width()+lane_spacing);
                    delete net_item;
                }
            }
                break;
            case EndpointList::SourceAndDestination:
            {
                NetLayoutConnectionFactory nlcf(srcPoints,dstPoints);
                // nlcf.dump(QString("wire %1").arg(id));
                mConnectionMetric.insert(NetLayoutMetric(id,nlcf.connection),nlcf.connection);
            }
                break;
            default:
                break;
            }
        }

        /// logic nets -> wire lanes
        for (auto it = mConnectionMetric.constBegin(); it!=mConnectionMetric.constEnd(); ++it)
        {
            u32 id = it.key().getId();
            const NetLayoutConnection* nlc = it.value();
            for (const NetLayoutWire& w : *nlc)
            {
                mWireHash[w].append(id);
            }
        }

        /// wires -> junction entries
        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            for (int iend=0; iend<2; iend++)
            {
                // iend == 0 =>  horizontal wire: right endpoint   junction: left entry
                NetLayoutPoint pnt = iend ? it.key().endPoint(true) : it.key().endPoint(false);
                int idirBase = it.key().isHorizontal() ? NetLayoutDirection::Left : NetLayoutDirection::Up;
                mJunctionEntries[pnt].setEntries(idirBase+iend, it.value());
            }
        }

        for (const node_box& nbox : m_boxes)
        {
            NetLayoutPoint inPnt(nbox.x,nbox.y*2);
            QList<u32> inpNets = nbox.item->inputNets();
            mJunctionEntries[inPnt].setEntries(NetLayoutDirection::Right, nbox.item->inputNets());
            mEndpointHash[inPnt].setInputPins(nbox.item->inputNets(),
                                              nbox.item->yTopPinDistance(),
                                              nbox.item->yEndpointDistance());
            NetLayoutPoint outPnt(nbox.x+1,nbox.y*2);
            mJunctionEntries[outPnt].setEntries(NetLayoutDirection::Left, nbox.item->outputNets());
            mEndpointHash[outPnt].setOutputPins(nbox.item->outputNets(),
                                                nbox.item->yTopPinDistance(),
                                                nbox.item->yEndpointDistance());
        }

        for (auto it = mJunctionEntries.constBegin(); it != mJunctionEntries.constEnd(); ++it)
        {
            it.value().dumpFile(it.key());
            NetLayoutJunction* nlj = new NetLayoutJunction(it.value());
            if (nlj->lastError() != NetLayoutJunction::Ok)
                qDebug() << "Junction route error" << nlj->lastError() << it.key();
            mJunctionHash.insert(it.key(),nlj);
        }
    }

    void GraphLayouter::calculate_nets()
    {
        for (const u32 id : m_context->nets())
        {
            Net* n = g_netlist->get_net_by_id(id);
            assert(n);

            if (n->is_unrouted())
                continue;

            used_paths used;

            for (Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                hal::node node;

                if (!m_context->node_for_gate(node, src->get_gate()->get_id()))
                    continue;

                node_box* src_box = nullptr;

                int ibox = m_boxNode.value(node,-1);
                if (ibox >= 0)
                    src_box = & (m_boxes[ibox]);

                assert(src_box);

                // FOR EVERY DST
                for (Endpoint* dst : n->get_destinations())
                {
                    // FIND DST BOX
                    if (!m_context->node_for_gate(node, dst->get_gate()->get_id()))
                        continue;

                    node_box* dst_box = nullptr;

                    int jbox = m_boxNode.value(node,-1);
                    if (jbox >= 0)
                        dst_box = & (m_boxes[jbox]);

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

    void GraphLayouter::find_max_box_dimensions()
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

    void GraphLayouter::findMaxChannelLanes()
    {
        // maximum parallel wires for atomic network
        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            const NetLayoutPoint& pnt = it.key().endPoint(true);  // wire start point
            unsigned int nw = it.value().size();
            if (it.key().isHorizontal())
                mCoordX[pnt.y()].testMinMax(nw);
            else
                mCoordY[pnt.x()].testMinMax(nw);
        }

        // maximal roads per junction
        for (auto it = mJunctionHash.constBegin(); it != mJunctionHash.constEnd(); ++it)
        {
            const NetLayoutPoint& pnt = it.key();
            const QRect& rect = it.value()->rect();
            mCoordX[pnt.x()].testMinMax(rect.left());
            mCoordX[pnt.x()].testMinMax(rect.right());
            mCoordY[pnt.y()].testMinMax(rect.top());
            mCoordY[pnt.y()].testMinMax(rect.bottom());
        }
    }

    void GraphLayouter::find_max_channel_lanes()
    {
        for (const road* r : m_v_roads.values())
            store_max(m_max_v_channel_lanes_for_x, r->x, r->lanes);

        for (const road* r : m_h_roads.values())
            store_max(m_max_h_channel_lanes_for_y, r->y, r->lanes);

        for (const junction* j : m_junctions.values())
        {
            store_max(m_max_v_channel_lanes_for_x, j->x, j->v_lanes);
            store_max(m_max_h_channel_lanes_for_y, j->y, j->h_lanes);
        }
    }

    void GraphLayouter::reset_roads_and_junctions()
    {
        for (road* r : m_h_roads.values())
            r->lanes = 0;

        for (road* r : m_v_roads.values())
            r->lanes = 0;

        for (junction* j : m_junctions.values())
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

    void GraphLayouter::calculate_max_channel_dimensions()
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

    void GraphLayouter::calculateJunctionMinDistance()
    {
        for (auto itJun = mJunctionHash.constBegin();
             itJun!=mJunctionHash.constEnd(); ++itJun)
        {
            const NetLayoutPoint& pnt0 = itJun.key();
            NetLayoutPoint pnt1 = pnt0 + QPoint(0,1);
            const NetLayoutJunction* j1 = mJunctionHash.value(pnt1);
            if (!j1) continue;
            const NetLayoutJunction* j0 = itJun.value();
            auto itEdp = mEndpointHash.find(pnt1.isEndpoint()? pnt1 : pnt0);
            if (itEdp == mEndpointHash.constEnd()) continue;
            float minDistance = 0;
            int iy = pnt1.y();
            if (pnt1.isEndpoint())
            {
                // net junction -> endpoint
                minDistance = (j0->rect().bottom() + 1) * lane_spacing
                        - itEdp.value().lanePosition(j1->rect().top(),false);
            }
            else
            {
                // endpoint -> net junction
                minDistance = itEdp.value().lanePosition(j0->rect().bottom(),false)
                        + (1-j1->rect().top()) * lane_spacing;
            }
            if (minDistance > mJunctionMinDistanceY[iy]) mJunctionMinDistanceY[iy] = minDistance;
        }
    }

    void GraphLayouter::calculateGateOffsets()
    {
        m_x_values.append(0);
        m_y_values.append(0);

        float x0 = mCoordX[0].preLanes() * lane_spacing + h_road_padding;
        mCoordX[0].setOffset(x0);
        auto itxLast = mCoordX.begin();
        for(auto itNext = itxLast + 1; itNext!= mCoordX.end(); ++itNext)
        {
            int ix0 = itxLast.key();
            int ix1 = itNext.key();
            float xsum = 0;
            float xsepOut = 0;
            float xsepInp = 0;

            for (auto itSep = mSeparatedWidth.constBegin(); itSep != mSeparatedWidth.constEnd(); itSep++)
            {
                if (itSep.key().x() != ix1) continue;
                NetLayoutJunction* jx = mJunctionHash.value(itSep.key());
                if (!jx) continue;
                float xout = itSep.value().mOutputSpace - jx->rect().left() * lane_spacing;
                if (xout > xsepOut) xsepOut = xout;
                float xinp = jx->rect().right() * lane_spacing + itSep.value().mInputSpace;
                if (xinp > xsepInp) xsepInp = xinp;
            }

            // loop in case that we span several columns
            for (int ix=ix0; ix<ix1;ix++)
            {
                auto xn = m_max_node_width_for_x.find(ix);
                if (xn != m_max_node_width_for_x.end())
                    xsum += xn.value();
            }
            itNext->setOffsetX(itxLast.value(), xsum + 4*h_road_padding, xsepOut, xsepInp);
            float lastX  = itxLast.value().lanePosition(0);
            float deltaX = itNext.value().lanePosition(0) - lastX;
            int   nx     = ix1 - ix0;
            if (nx<=0) nx=1;
            for (int ix=1; ix<=nx; ix++)
                m_x_values.append(lastX + deltaX * ix / nx);
            itxLast = itNext;
        }

        float y0 = mCoordY[0].preLanes() * lane_spacing + v_road_padding;
        mCoordY[0].setOffset(y0);
        auto ityLast = mCoordY.begin();
        for(auto itNext = ityLast + 1; itNext!= mCoordY.end(); ++itNext)
        {
            int iy0 = ityLast.key();
            int iy1 = itNext.key();

            Q_ASSERT(iy1 = iy0+1);

            if (iy0 % 2 == 1)
            {
                // netjunction -> endpoint
                itNext->setOffsetYje(ityLast.value(), mJunctionMinDistanceY.value(iy1));
            }
            else
            {
                // endpoint -> netjunction
                float ydelta = v_road_padding;
                auto yn = m_max_node_height_for_y.find(iy0/2);
                if (yn != m_max_node_height_for_y.constEnd())
                    ydelta += yn.value();
                itNext->setOffsetYej(ityLast.value(), ydelta, mJunctionMinDistanceY.value(iy1));
                m_y_values.append(itNext.value().lanePosition(0));
            }
            ityLast = itNext;
        }
    }

    void GraphLayouter::calculate_gate_offsets()
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

    void GraphLayouter::placeGates()
    {
        for (node_box& box : m_boxes)
        {
            box.item->setPos(mCoordX[box.x].xBoxOffset(),
                             mCoordY[box.y*2].lanePosition(0));
            m_scene->add_item(box.item);

            NetLayoutPoint outPnt(box.x+1,box.y*2);
            QPointF outPos = box.item->endpointPositionByIndex(0,false);
            mEndpointHash[outPnt].setOutputPosition(outPos);

            NetLayoutPoint inPnt(box.x,box.y*2);
            QPointF inPos = box.item->endpointPositionByIndex(0,true);
            mEndpointHash[inPnt].setInputPosition(inPos);
        }
    }

    void GraphLayouter::place_gates()
    {
        for (node_box& box : m_boxes)
        {
            box.item->setPos(m_node_offset_for_x.value(box.x), m_node_offset_for_y.value(box.y));
            m_scene->add_item(box.item);
        }
    }

    void GraphLayouter::drawNets()
    {
        // lane for given wire and net id
        QHash<u32,QHash<NetLayoutWire,int>> laneMap;

        for (auto it=mWireHash.constBegin(); it!=mWireHash.constEnd(); ++it)
        {
            int ilane = 0;
            for(u32 id : it.value())
                laneMap[id].insert(it.key(),ilane++);
        }

        for (const u32 id : m_context->nets())
        {
            Net* n = g_netlist->get_net_by_id(id);

            const EndpointList& epl = mWireEndpoint.value(id);
            bool regularNet = false;

            switch(epl.hasEndpoint())
            {
            case EndpointList::NoEndpoint:
                break;
            case EndpointList::OnlySource:
            case EndpointList::OnlyDestination:
            case EndpointList::ConstantLevel:
                drawNetsIsolated(id,n,epl);
                break;;
            default:
                regularNet = true;
                break;
            }


            if (!regularNet)
                continue;

            StandardGraphicsNet::lines lines;

            const QHash<NetLayoutWire,int>& wMap = laneMap.value(id);
            for (auto it=wMap.constBegin(); it!=wMap.constEnd(); ++it)
            {
                NetLayoutPoint wFromPoint = it.key().endPoint(true);
                NetLayoutPoint wToPoint   = it.key().endPoint(false);
                NetLayoutJunction* j0 = mJunctionHash.value(wFromPoint);
                NetLayoutJunction* j1 = mJunctionHash.value(wToPoint);
                int ilane = it.value();
                int ix0 = wFromPoint.x();
                int iy0 = wFromPoint.y();
                int ix1 = wToPoint.x();
                int iy1 = wToPoint.y();

                if (it.key().isHorizontal())
                {
                    float x0 = j0
                            ? mCoordX[ix0].lanePosition(j0->rect().right())
                            : mCoordX[ix0].junctionExit();
                    float x1 = j1
                            ? mCoordX[ix1].lanePosition(j1->rect().left())
                            : mCoordX[ix1].junctionEntry();
                    float yy = mCoordY[iy0].lanePosition(ilane);
                    lines.append_h_line(x0,x1,yy);
                }
                else
                {
                    float y0, y1;
                    float xx = mCoordX[ix0].lanePosition(ilane);
                    if (wToPoint.isEndpoint())
                    {
                        // netjunction -> endpoint
                        auto itEpc = mEndpointHash.find(wToPoint);
                        y0 = j0
                                ? mCoordY[iy0].lanePosition(j0->rect().bottom())
                                : mCoordY[iy0].junctionExit();
                        y1 = itEpc != mEndpointHash.constEnd()
                                ? itEpc.value().lanePosition(j1->rect().top(),true)
                                : mCoordY[iy1].junctionEntry();
                    }
                    else
                    {
                        // endpoint -> netjunction
                        auto itEpc = mEndpointHash.find(wFromPoint);
                        y0 = itEpc != mEndpointHash.constEnd()
                                ? itEpc.value().lanePosition(j0->rect().bottom(),true)
                                : mCoordY[iy0].junctionExit();
                        y1 = j1
                                ? mCoordY[iy1].lanePosition(j1->rect().top())
                                : mCoordY[iy1].junctionEntry();
                    }
                    lines.append_v_line(xx,y0,y1);
                }
            }
            drawNetsJunction(lines,id);
            drawNetsEndpoint(lines,id);

            lines.merge_lines();
            StandardGraphicsNet* GraphicsNet = new StandardGraphicsNet(n, lines);
            m_scene->add_item(GraphicsNet);

        }
    }

    void GraphLayouter::drawNetsIsolated(u32 id, Net* n, const EndpointList &epl)
    {
        SeparatedGraphicsNet* net_item = epl.hasEndpoint() == EndpointList::ConstantLevel
                ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n,QString::fromStdString(n->get_name())))
                : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));

        int ipnt = 0;
        for (const NetLayoutPoint& pnt : epl)
        {
            bool isInput = epl.isInput(ipnt++);
            auto itPnt = mEndpointHash.find(pnt);
            Q_ASSERT(itPnt != mEndpointHash.constEnd());
            int inx = isInput ? itPnt.value().inputPinIndex(id) : itPnt.value().outputPinIndex(id);
            float x = isInput ? itPnt.value().xInput() : itPnt.value().xOutput();
            QPointF scenePnt(x,itPnt.value().lanePosition(inx,true));
            if (isInput)
                net_item->add_input(scenePnt);
            else
                net_item->add_output(scenePnt);
        }
        net_item->finalize();
        m_scene->add_item(net_item);
    }

    void GraphLayouter::drawNetsEndpoint(StandardGraphicsNet::lines &lines, u32 id)
    {
        for (auto it=mEndpointHash.constBegin(); it!=mEndpointHash.constEnd(); ++it)
        {
            const EndpointCoordinate& epc = it.value();
            int inpInx = epc.inputPinIndex(id);
            int outInx = epc.outputPinIndex(id);
            if (inpInx < 0 && outInx < 0) continue;

            const NetLayoutJunction* nlj = mJunctionHash.value(it.key());
            const SceneCoordinate& xScenePos = mCoordX.value(it.key().x());
            float xjLeft  = xScenePos.lanePosition(nlj->rect().left());
            float xjRight = xScenePos.lanePosition(nlj->rect().right());
            Q_ASSERT(nlj);
            if (inpInx>=0)
            {
                if (xjRight >= epc.xInput())
                    qDebug() << "cannot connect input pin" << id << it.key().x() << it.key().y()/2 << xjRight << epc.xInput();
                else
                    lines.append_h_line(xjRight, epc.xInput(), epc.lanePosition(inpInx,true));
            }
            if (outInx>=0)
            {
                if (epc.xOutput() >= xjLeft)
                    qDebug() << "cannot connect output pin" << id << it.key().x() << it.key().y()/2 << xjLeft << epc.xOutput();
                else
                    lines.append_h_line(epc.xOutput(), xjLeft, epc.lanePosition(outInx,true));
            }
        }
    }

    void GraphLayouter::drawNetsJunction(StandardGraphicsNet::lines& lines, u32 id)
    {
        for (auto jt = mJunctionHash.constBegin(); jt!=mJunctionHash.constEnd(); ++jt)
        {
            const EndpointCoordinate& epc = mEndpointHash.value(jt.key());
            int x = jt.key().x();
            int y = jt.key().y();
            bool isEndpoint = (y%2 == 0);

            for (const NetLayoutJunctionWire& jw : jt.value()->netById(id).mWires)
            {
                if (jw.mHorizontal==0)
                {
                    float x0 = mCoordX.value(x).lanePosition(jw.mFirst);
                    float x1 = mCoordX.value(x).lanePosition(jw.mLast);
                    float yy = isEndpoint
                            ? epc.lanePosition(jw.mRoad,true)
                            : mCoordY.value(y).lanePosition(jw.mRoad);
                    lines.append_h_line(x0,x1,yy);
                }
                else
                {
                   float y0 = isEndpoint
                            ? epc.lanePosition(jw.mFirst,true)
                            : mCoordY.value(y).lanePosition(jw.mFirst);
                    float y1 = isEndpoint
                            ? epc.lanePosition(jw.mLast,true)
                            : mCoordY.value(y).lanePosition(jw.mLast);
                    float xx = mCoordX.value(x).lanePosition(jw.mRoad);
                    lines.append_v_line(xx,y0,y1);
                }
            }
        }
    }

    void GraphLayouter::draw_nets()
    {
        // ROADS AND JUNCTIONS FILLED LEFT TO RIGHT, TOP TO BOTTOM
        for (const u32 id : m_context->nets())
        {
            Net* n = g_netlist->get_net_by_id(id);
            assert(n);

            if (n->is_unrouted())
            {
                // HANDLE GLOBAL NETS
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* src : n->get_sources())
                {
                    if (src->get_gate())
                    {
                        hal::node node;

                        if (!m_context->node_for_gate(node, src->get_gate()->get_id()))
                            continue;

                        int ibox = m_boxNode.value(node,-1);
                        if (ibox >= 0)
                            net_item->add_output(m_boxes.at(ibox).item->get_output_scene_position(n->get_id(),
                                                 QString::fromStdString(src->get_pin())));
                    }
                }

                for (Endpoint* dst : n->get_destinations())
                {
                    if (dst->get_gate())
                    {
                        hal::node node;

                        if (!m_context->node_for_gate(node, dst->get_gate()->get_id()))
                            continue;

                        int ibox = m_boxNode.value(node,-1);
                        if (ibox >= 0)
                            net_item->add_input(m_boxes.at(ibox).item->get_input_scene_position(n->get_id(),
                                                QString::fromStdString(dst->get_pin())));
                    }
                }

                net_item->finalize();
                m_scene->add_item(net_item);
                continue;
            }

            bool use_label = isConstNet(n);

            if (use_label)
            {
                LabeledSeparatedNet* net_item = new LabeledSeparatedNet(n, QString::fromStdString(n->get_name()));

                for (Endpoint* src : n->get_sources())
                {
                    hal::node node;

                    if (m_context->node_for_gate(node, src->get_gate()->get_id()))
                    {
                        int ibox = m_boxNode.value(node,-1);
                        if (ibox >= 0)
                            net_item->add_output(m_boxes.at(ibox).item->get_output_scene_position(n->get_id(),
                                                 QString::fromStdString(src->get_pin())));
                    }
                }

                for (Endpoint* dst : n->get_destinations())
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, dst->get_gate()->get_id()))
                        continue;

                    int ibox = m_boxNode.value(node,-1);
                    if (ibox >= 0)
                        net_item->add_input(m_boxes.at(ibox).item->get_input_scene_position(n->get_id(),
                                            QString::fromStdString(dst->get_pin())));
                }

                net_item->finalize();
                m_scene->add_item(net_item);

                continue;
            }

            bool incomplete_net = false;    // PASS TO SHADER ???
            bool src_found      = false;
            bool dst_found      = false;

            for (Endpoint* src : n->get_sources())
            {
                hal::node node;

                if (m_context->node_for_gate(node, src->get_gate()->get_id()))
                    src_found = true;
                else
                    incomplete_net = true;
            }

            for (Endpoint* dst : n->get_destinations())
            {
                hal::node node;

                if (m_context->node_for_gate(node, dst->get_gate()->get_id()))
                    dst_found = true;
                else
                    incomplete_net = true;
            }

            if (src_found && !dst_found)
            {
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* src : n->get_sources())
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, src->get_gate()->get_id()))
                        continue;

                    int ibox = m_boxNode.value(node,-1);
                    if (ibox >= 0)
                        net_item->add_output(m_boxes.at(ibox).item->get_output_scene_position(n->get_id(),
                                             QString::fromStdString(src->get_pin())));
                }

                net_item->finalize();
                m_scene->add_item(net_item);

                continue;
            }

            if (!src_found && dst_found)
            {
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* dst : n->get_destinations())
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, dst->get_gate()->get_id()))
                        continue;

                    int ibox = m_boxNode.value(node,-1);
                    if (ibox >= 0)
                        net_item->add_input(m_boxes.at(ibox).item->get_input_scene_position(n->get_id(), QString::fromStdString(dst->get_pin())));
                }

                net_item->finalize();
                m_scene->add_item(net_item);

                continue;
            }

            // HANDLE NORMAL NETS
            used_paths used;
            StandardGraphicsNet::lines lines;

            // FOR EVERY SRC
            for (Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                node_box* src_box = nullptr;
                {
                    hal::node node;

                    if (!m_context->node_for_gate(node, src->get_gate()->get_id()))
                        continue;

                    int ibox = m_boxNode.value(node,-1);
                    if (ibox >= 0)
                        src_box = & (m_boxes[ibox]);
                }
                assert(src_box);

                const QPointF src_pin_position = src_box->item->get_output_scene_position(n->get_id(), QString::fromStdString(src->get_pin()));

                // FOR EVERY DST
                for (Endpoint* dst : n->get_destinations())
                {
                    // FIND DST BOX
                    node_box* dst_box = nullptr;

                    hal::node node;

                    if (!m_context->node_for_gate(node, dst->get_gate()->get_id()))
                        continue;

                    int ibox = m_boxNode.value(node,-1);
                    if (ibox >= 0)
                        dst_box = & (m_boxes[ibox]);

                    assert(dst_box);

                    // don't attempt to loop back a module output into its input
                    // (if this triggers, we found the net because it also has
                    // destinations outside the module)
                    if (src_box == dst_box && src_box->node.type == hal::node_type::module)
                        continue;

                    QPointF dst_pin_position = dst_box->item->get_input_scene_position(n->get_id(), QString::fromStdString(dst->get_pin()));

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
            StandardGraphicsNet* GraphicsNet = new StandardGraphicsNet(n, lines);
            m_scene->add_item(GraphicsNet);

            commit_used_paths(used);
        }
    }

    void GraphLayouter::update_scene_rect()
    {
        // SCENE RECT STUFF BEHAVES WEIRDLY, FURTHER RESEARCH REQUIRED
        //QRectF rect = m_scene->sceneRect();

        QRectF rect(m_scene->itemsBoundingRect());
        rect.adjust(-200, -200, 200, 200);
        m_scene->setSceneRect(rect);
    }

    GraphLayouter::node_box GraphLayouter::create_box(const hal::node& node, const int x, const int y) const
    {
        node_box box;
        box.node = node;

        switch (node.type)
        {
            case hal::node_type::module: {
                box.item = GraphicsFactory::create_graphics_module(g_netlist->get_module_by_id(node.id), 0);
                break;
            }
            case hal::node_type::gate: {
                box.item = GraphicsFactory::create_graphics_gate(g_netlist->get_gate_by_id(node.id), 0);
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

    bool GraphLayouter::box_exists(const int x, const int y) const
    {
        return m_boxPosition.contains(QPoint(x,y));
    }

    bool GraphLayouter::h_road_jump_possible(const int x, const int y1, const int y2) const
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

    bool GraphLayouter::h_road_jump_possible(const GraphLayouter::road* const r1, const GraphLayouter::road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->x != r2->x);

        return h_road_jump_possible(r1->x, r1->y, r2->y);
    }

    bool GraphLayouter::v_road_jump_possible(const int x1, const int x2, const int y) const
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

    bool GraphLayouter::v_road_jump_possible(const GraphLayouter::road* const r1, const GraphLayouter::road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->y != r2->y);

        return v_road_jump_possible(r1->x, r2->x, r1->y);
    }

    GraphLayouter::road* GraphLayouter::get_h_road(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = m_h_roads.find(p);
        if (it != m_h_roads.end())
            return it.value();

        GraphLayouter::road* r = new road(x, y);
        m_h_roads.insert(p,r);
        return r;
    }

    GraphLayouter::road* GraphLayouter::get_v_road(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = m_v_roads.find(p);
        if (it != m_v_roads.end())
            return it.value();

        GraphLayouter::road* r = new road(x, y);
        m_v_roads.insert(p,r);
        return r;
    }

    GraphLayouter::junction* GraphLayouter::get_junction(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = m_junctions.find(p);
        if (it != m_junctions.end())
        {
            return it.value();
        }

        GraphLayouter::junction* j = new junction(x, y);
        m_junctions.insert(p,j);
        return j;
    }

    qreal GraphLayouter::h_road_height(const unsigned int lanes) const
    {
        // LANES COUNTED FROM 1
        qreal height = h_road_padding * 2;

        if (lanes > 1)
            height += (lanes - 1) * lane_spacing;

        return height;
    }

    qreal GraphLayouter::v_road_width(const unsigned int lanes) const
    {
        // LANES COUNTED FROM 1
        qreal width = v_road_padding * 2;

        if (lanes > 1)
            width += (lanes - 1) * lane_spacing;

        return width;
    }

    qreal GraphLayouter::scene_y_for_h_channel_lane(const int y, const unsigned int lane) const
    {
        // LINES NUMBERED FROM 0
        assert(m_node_offset_for_y.contains(y) || m_node_offset_for_y.contains(y - 1));

        const qreal offset = lane * lane_spacing;

        if (y == 0)
            return m_node_offset_for_y.value(y) - m_max_h_channel_height_for_y.value(y) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
        else
            return m_node_offset_for_y.value(y - 1) + m_max_node_height_for_y.value(y - 1) + m_max_h_channel_top_spacing_for_y.value(y) + offset;
    }

    qreal GraphLayouter::scene_x_for_v_channel_lane(const int x, const unsigned int lane) const
    {
        // LINES NUMBERED FROM 0
        assert(m_node_offset_for_x.contains(x) || m_node_offset_for_x.contains(x - 1));

        const qreal offset = lane * lane_spacing;

        if (m_node_offset_for_x.contains(x))
            return m_node_offset_for_x.value(x) - m_max_v_channel_width_for_x.value(x) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
        else
            return m_node_offset_for_x.value(x - 1) + m_max_node_width_for_x.value(x - 1) + m_max_v_channel_left_spacing_for_x.value(x) + offset;
    }

    qreal GraphLayouter::scene_x_for_close_left_lane_change(const int channel_x, unsigned int lane_change) const
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

    qreal GraphLayouter::scene_x_for_far_left_lane_change(const int channel_x, unsigned int lane_change) const
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

    qreal GraphLayouter::scene_x_for_close_right_lane_change(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

        if (m_node_offset_for_x.contains(channel_x))
            return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
        else
            return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
                   - m_max_v_channel_right_spacing_for_x.value(channel_x) + junction_padding + lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_x_for_far_right_lane_change(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(m_node_offset_for_x.contains(channel_x) || m_node_offset_for_x.contains(channel_x - 1));

        if (m_node_offset_for_x.contains(channel_x))
            return m_node_offset_for_x.value(channel_x) - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
        else
            return m_node_offset_for_x.value(channel_x - 1) + m_max_node_width_for_x.value(channel_x - 1) + m_max_v_channel_width_for_x.value(channel_x)
                   - m_max_v_channel_right_spacing_for_x.value(channel_x) + m_max_right_junction_spacing_for_x.value(channel_x) - lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_y_for_close_top_lane_change(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
                   - lane_change * lane_spacing;
        else
            return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y) - junction_padding
                   - lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_y_for_far_top_lane_change(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return m_node_offset_for_y.value(channel_y) - m_max_h_channel_height_for_y.value(channel_y) + m_max_h_channel_top_spacing_for_y.value(channel_y)
                   - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
        else
            return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_top_spacing_for_y.value(channel_y)
                   - m_max_top_junction_spacing_for_y.value(channel_y) + lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_y_for_close_bottom_lane_change(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
        else
            return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
                   - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + junction_padding + lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_y_for_far_bottom_lane_change(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return m_node_offset_for_y.value(channel_y) - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
        else
            return m_node_offset_for_y.value(channel_y - 1) + m_max_node_height_for_y.value(channel_y - 1) + m_max_h_channel_height_for_y.value(channel_y)
                   - m_max_h_channel_bottom_spacing_for_y.value(channel_y) + m_max_bottom_junction_spacing_for_y.value(channel_y) - lane_change * lane_spacing;
    }

    qreal GraphLayouter::scene_x_for_close_left_lane_change(const junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_x_for_close_left_lane_change(j->x, j->close_left_lane_changes);
    }

    qreal GraphLayouter::scene_x_for_far_left_lane_change(const GraphLayouter::junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_x_for_far_left_lane_change(j->x, j->far_left_lane_changes);
    }

    qreal GraphLayouter::scene_x_for_close_right_lane_change(const junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_x_for_close_right_lane_change(j->x, j->close_right_lane_changes);
    }

    qreal GraphLayouter::scene_x_for_far_right_lane_change(const GraphLayouter::junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_x_for_far_right_lane_change(j->x, j->far_right_lane_changes);
    }

    qreal GraphLayouter::scene_y_for_close_top_lane_change(const junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_y_for_close_top_lane_change(j->y, j->close_top_lane_changes);
    }

    qreal GraphLayouter::scene_y_for_far_top_lane_change(const GraphLayouter::junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_y_for_far_top_lane_change(j->y, j->far_top_lane_changes);
    }

    qreal GraphLayouter::scene_y_for_close_bottom_lane_change(const junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_y_for_close_bottom_lane_change(j->y, j->close_bottom_lane_changes);
    }

    qreal GraphLayouter::scene_y_for_far_bottom_lane_change(const GraphLayouter::junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return scene_y_for_far_bottom_lane_change(j->y, j->far_bottom_lane_changes);
    }

    void GraphLayouter::commit_used_paths(const GraphLayouter::used_paths& used)
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

    void GraphLayouter::SceneCoordinate::testMinMax(int ilane)
    {
        if (ilane  < minLane) minLane = ilane;
        if (ilane+1> maxLane) maxLane = ilane+1;
    }

    void GraphLayouter::SceneCoordinate::setOffsetX(const SceneCoordinate& previous, float maximumBlock, float sepOut, float sepInp)
    {
        float delta = (previous.maxLane - minLane - 1) * lane_spacing + previous.mPadding + maximumBlock;
        if (delta < sepOut) delta = sepOut;
        mOffset = previous.mOffset + delta;
        float xDefaultBoxPadding = maxLane * lane_spacing;
        if (xDefaultBoxPadding < sepInp)
            mPadding = sepInp - xDefaultBoxPadding;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYje(const SceneCoordinate& previous, float minimumJunction)
    {
        float delta = (previous.maxLane) * lane_spacing + v_road_padding;
        if (delta < minimumJunction) delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYej(const SceneCoordinate& previous, float maximumBlock, float minimumJunction)
    {
        float delta = (-minLane - 1) * lane_spacing + maximumBlock + v_road_padding;
        if (delta < minimumJunction) delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    float GraphLayouter::SceneCoordinate::lanePosition(int ilane) const
    {
        return mOffset + ilane * lane_spacing;
    }

    float GraphLayouter::SceneCoordinate::xBoxOffset() const
    {
        return junctionExit() + h_road_padding + mPadding;
    }

    float GraphLayouter::EndpointCoordinate::lanePosition(int ilane, bool absolute) const
    {
        float y0 = absolute ? mYoffset : mTopPin;
        if (ilane < 0) return y0 + ilane * lane_spacing;
        int n = numberPins() - 1;
        if (ilane <= n) return y0 + ilane * mPinDistance;
        return y0 + n * mPinDistance + (ilane-n) * lane_spacing;
    }

    GraphLayouter::EndpointCoordinate::EndpointCoordinate()
        : mYoffset(0), mXoutput(0), mXinput(0), mPinDistance(0), mTopPin(0), mNumberPins(0)
    {;}

    int GraphLayouter::EndpointCoordinate::numberPins() const
    {
        int nInp = mInputHash.size();
        int nOut = mOutputHash.size();
        return nInp > nOut ? nInp : nOut;
    }

    void GraphLayouter::EndpointCoordinate::setInputPosition(QPointF p0pos)
    {
        mXinput  = p0pos.x();
        mYoffset = p0pos.y();
    }

    void GraphLayouter::EndpointCoordinate::setOutputPosition(QPointF p0pos)
    {
        mXoutput = p0pos.x();
        if (mXinput < mXoutput) mXinput = mXoutput;
        mYoffset = p0pos.y();
    }

    int GraphLayouter::EndpointCoordinate::inputPinIndex(u32 id) const
    {
        return mInputHash.value(id,-1);
    }

    int GraphLayouter::EndpointCoordinate::outputPinIndex(u32 id) const
    {
        return mOutputHash.value(id,-1);
    }

    void GraphLayouter::EndpointCoordinate::setInputPins(const QList<u32> &pinList, float p0dist, float pdist)
    {
        for (int i=0; i<pinList.size(); i++)
        {
            u32 id = pinList.at(i);
            if (id) mInputHash.insert(id,i);
        }
        if (p0dist > mTopPin) mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointCoordinate::setOutputPins(const QList<u32>& pinList, float p0dist, float pdist)
    {
        for (int i=0; i<pinList.size(); i++)
        {
            u32 id = pinList.at(i);
            if (id) mOutputHash.insert(id,i);
        }
        if (p0dist > mTopPin) mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointList::addSource(const NetLayoutPoint &pnt)
    {
        mHasEndpoint = static_cast<hasEndpoint_t>(mHasEndpoint | OnlySource);
        append(pnt);
        mPointIsInput.append(false);
    }

    void GraphLayouter::EndpointList::addDestination(const NetLayoutPoint &pnt)
    {
        mHasEndpoint = static_cast<hasEndpoint_t>(mHasEndpoint | OnlyDestination);
        append(pnt);
        mPointIsInput.append(true);
    }

    void GraphLayouter::EndpointList::setConstantLevel()
    {
        mHasEndpoint = ConstantLevel;
    }

    void GraphLayouter::SeparatedNetWidth::requireInputSpace(float spc)
    {
        if (spc > mInputSpace) mInputSpace = spc;
    }

    void GraphLayouter::SeparatedNetWidth::requireOutputSpace(float spc)
    {
        if (spc > mOutputSpace) mOutputSpace = spc;
    }

    bool GraphLayouter::isConstNet(const Net* n)
    {
        for (Endpoint* src : n->get_sources())
        {
            if (src->get_gate()->is_gnd_gate() || src->get_gate()->is_vcc_gate())
                return true;
        }
        return false;
    }
}

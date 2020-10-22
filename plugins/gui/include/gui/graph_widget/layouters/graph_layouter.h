//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

#include "gui/gui_def.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/layouters/net_layout_point.h"
#include "gui/graph_widget/layouters/net_layout_junction.h"
#include "gui/netlist_relay/netlist_relay.h"

#include <QObject>
#include <QPoint>
#include <QSet>
#include <QMap>
#include <QVector>

namespace hal
{
    class GraphContext;
    class GraphicsGate;
    class GraphicsItem;
    class GraphicsNet;
    class GraphicsNode;
    class GraphicsScene;
    class SeparatedGraphicsNet;
    class StandardGraphicsNet;

    class NetLayoutJunctionHash;
    class NetLayoutJunctionEntries;

    class GraphLayouter : public QObject
    {
        Q_OBJECT

        class SceneCoordinate
        {
            int minLane;
            int maxLane;
            float mOffset;
            float mPadding;
        public:
            SceneCoordinate() : minLane(0), maxLane(0), mOffset(0), mPadding(0) {;}
            void testMinMax(int ilane);
            void setOffset(float off) { mOffset = off; }
            void setOffsetYje(const SceneCoordinate& previous, float minimumJunction);
            void setOffsetYej(const SceneCoordinate& previous, float maximumBlock, float minimumJunction);
            void setOffsetX(const SceneCoordinate& previous, float maximumBlock, float sepOut, float sepInp);
            float lanePosition(int ilane) const;
            int preLanes() const { return -minLane; }
            float junctionEntry() const { return lanePosition(minLane); }
            float junctionExit() const { return lanePosition(maxLane-1); }
            float xBoxOffset() const;
        };

        class EndpointCoordinate
        {
            float mYoffset;
            float mXoutput;
            float mXinput;
            float mPinDistance;
            float mTopPin;
            int   mNumberPins;
            QHash<u32,int> mInputHash;
            QHash<u32,int> mOutputHash;
        public:
            EndpointCoordinate();
            void setInputPosition(QPointF p0pos);
            void setOutputPosition(QPointF p0pos);
            float lanePosition(int ilane, bool absolute) const;
            float xInput() const { return mXinput; }
            float xOutput() const { return mXoutput; }
            void setInputPins(const QList<u32>& pinList, float p0dist, float pdist);
            void setOutputPins(const QList<u32>& pinList, float p0dist, float pdist);
            int numberPins() const;
            int inputPinIndex(u32 id) const;
            int outputPinIndex(u32 id) const;
        };

        class EndpointList : public QList<NetLayoutPoint>
        {
        public:
            enum hasEndpoint_t { NoEndpoint = 0, OnlySource = 1, OnlyDestination = 2, SourceAndDestination = 3, ConstantLevel = 4 };
            EndpointList() : mHasEndpoint(NoEndpoint) {;}
            void addSource(const NetLayoutPoint& pnt);
            void addDestination(const NetLayoutPoint& pnt);
            void setConstantLevel();
            hasEndpoint_t hasEndpoint() const { return mHasEndpoint; }
            bool isInput(int index) const {return mPointIsInput.at(index); }
        private:
            hasEndpoint_t mHasEndpoint;
            QList<bool> mPointIsInput;
        };

        class SeparatedNetWidth
        {
        public:
            float mInputSpace;
            float mOutputSpace;
            SeparatedNetWidth() : mInputSpace(0), mOutputSpace(0) {;}
            void requireInputSpace(float spc);
            void requireOutputSpace(float spc);
        };

        struct node_level
        {
            hal::node node;
            int level;
        };

        struct node_box
        {
            hal::node node;
            GraphicsNode* item;

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
                far_bottom_lane_changes(0)
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
        explicit GraphLayouter(const GraphContext* const context, QObject* parent = nullptr);

        virtual QString name() const        = 0;
        virtual QString description() const = 0;

        virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, hal::placement_hint placement = hal::placement_hint{hal::placement_mode::standard, hal::node()})    = 0;
        virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) = 0;

        void layout();
        void alternateLayout();

        GraphicsScene* scene() const;

        const QMap<hal::node, QPoint> node_to_position_map() const;
        const QMap<QPoint, hal::node> position_to_node_map() const;
        QPoint gridPointByItem(GraphicsNode* item) const;

        void dumpNodePositions(const QPoint &search) const;

        void set_node_position(const hal::node& n, const QPoint& p);
        void swap_node_positions(const hal::node& n1, const hal::node& n2);
        void remove_node_from_maps(const hal::node& n);

        int min_x_index() const;
        int min_y_index() const;

        bool done() const;

        QVector<qreal> x_values() const;
        QVector<qreal> y_values() const;

        qreal max_node_width() const;
        qreal max_node_height() const;

        qreal default_grid_width() const;
        qreal default_grid_height() const;

    Q_SIGNALS:
        void status_update(const int percent);
        void status_update(const QString& message);

    protected:
        GraphicsScene* m_scene;
        const GraphContext* const m_context;

    private:
        void clear_layout_data();
        void create_boxes();
        void calculate_nets();
        void getWireHash();
        void find_max_box_dimensions();
        void findMaxChannelLanes();
        void find_max_channel_lanes();
        void calculate_max_channel_dimensions();
        void calculateJunctionMinDistance();
        void calculateGateOffsets();
        void calculate_gate_offsets();
        void placeGates();
        void place_gates();
        void reset_roads_and_junctions();
        void draw_nets();
        void drawNets();
        void drawNetsJunction(StandardGraphicsNet::lines &lines, u32 id);
        void drawNetsEndpoint(StandardGraphicsNet::lines &lines, u32 id);
        void drawNetsIsolated(u32 id, Net *n, const EndpointList& epl);
        void update_scene_rect();

        node_box create_box(const hal::node& node, const int x, const int y) const;

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

        qreal scene_x_for_close_left_lane_change(const junction* const j) const;
        qreal scene_x_for_far_left_lane_change(const junction* const j) const;

        qreal scene_x_for_close_right_lane_change(const junction* const j) const;
        qreal scene_x_for_far_right_lane_change(const junction* const j) const;

        qreal scene_y_for_close_top_lane_change(const junction* const j) const;
        qreal scene_y_for_far_top_lane_change(const junction* const j) const;

        qreal scene_y_for_close_bottom_lane_change(const junction* const j) const;
        qreal scene_y_for_far_bottom_lane_change(const junction* const j) const;

        void commit_used_paths(const used_paths& used);
        static bool isConstNet(const Net* n);

        QVector<node_box>        m_boxes;
        QHash<QPoint,int>        m_boxPosition;
        QHash<node,int>          m_boxNode;
        QHash<GraphicsNode*,int> m_boxGraphItem;

        QHash<QPoint,road*> m_h_roads;
        QHash<QPoint,road*> m_v_roads;
        QHash<QPoint,junction*> m_junctions;

        QMap<int, qreal> m_max_node_width_for_x;
        QMap<int, qreal> m_max_node_height_for_y;

        QMap<int, unsigned int> m_max_v_channel_lanes_for_x;
        QMap<int, unsigned int> m_max_h_channel_lanes_for_y;

        QMap<int, qreal> m_max_v_channel_left_spacing_for_x;
        QMap<int, qreal> m_max_v_channel_right_spacing_for_x;
        QMap<int, qreal> m_max_h_channel_top_spacing_for_y;
        QMap<int, qreal> m_max_h_channel_bottom_spacing_for_y;

        QMap<int, qreal> m_max_v_channel_width_for_x;
        QMap<int, qreal> m_max_h_channel_height_for_y;

        QMap<int, qreal> m_node_offset_for_x;
        QMap<int, qreal> m_node_offset_for_y;

        QMap<int, qreal> m_max_left_junction_spacing_for_x;
        QMap<int, qreal> m_max_right_junction_spacing_for_x;

        QMap<int, qreal> m_max_top_junction_spacing_for_y;
        QMap<int, qreal> m_max_bottom_junction_spacing_for_y;

        QMap<int, qreal> m_max_left_io_padding_for_channel_x;
        QMap<int, qreal> m_max_right_io_padding_for_channel_x;

        QMap<hal::node, QPoint> m_node_to_position_map;
        QMap<QPoint, hal::node> m_position_to_node_map;

        int m_min_x_index;
        int m_min_y_index;

        int m_max_x_index;
        int m_max_y_index;

        QVector<qreal> m_x_values;
        QVector<qreal> m_y_values;

        qreal m_max_node_width;
        qreal m_max_node_height;

        bool m_done;

        NetLayoutConnectionMetric mConnectionMetric;
        QHash<NetLayoutWire,QList<u32>> mWireHash;
        QHash<NetLayoutPoint,NetLayoutJunctionEntries> mJunctionEntries;
        QHash<NetLayoutPoint,EndpointCoordinate> mEndpointHash;
        QHash<NetLayoutPoint,SeparatedNetWidth> mSeparatedWidth;
        QHash<NetLayoutPoint,float> mSpaceSeparatedOutputs;
        NetLayoutJunctionHash mJunctionHash;
        QMap<int,SceneCoordinate> mCoordX;
        QMap<int,SceneCoordinate> mCoordY;
        QMap<int,float> mJunctionMinDistanceY;
        QHash<u32,EndpointList> mWireEndpoint;
    };

    uint qHash(const hal::node& n);
}


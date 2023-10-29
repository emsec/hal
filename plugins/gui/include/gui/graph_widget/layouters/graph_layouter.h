// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/layouters/net_layout_junction.h"
#include "gui/graph_widget/layouters/net_layout_point.h"
#include "gui/graph_widget/layouters/node_box.h"
#include "gui/gui_def.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

#include <QMap>
#include <QMultiHash>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSet>
#include <QVector>
#include <QThread>

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
    class CommentSpeechBubble;
    class CommentEntry;
    class JunctionThread;
    class DrawNetThread;

    /**
     * @ingroup graph-layouter
     * @brief Base class for all specific layouters.
     *
     * The abstract parent class for all graph layouters. The graph layouter is responsible for building up the scene
     * using the information given in the passed GraphContext. It places the respective GraphicItems for the given gates
     * and modules in positions that are calculated intelligently considering their order, connections and dimensions.
     * Afterwards, the paths of GraphicsNet objects will be computed to visualize the nets between gates/modules. <br>
     * If the GraphContext changes later on (e.g. gates/modules are added/removed), the GraphLayouter has the task to
     * apply the changes in the scene.
     */
    class GraphLayouter : public QObject
    {
        Q_OBJECT

        friend class DrawNetThread;

        class SceneCoordinate
        {
            int minLane;
            int maxLane;
            float mOffset;
            float mPadding;

        public:
            SceneCoordinate() : minLane(0), maxLane(0), mOffset(0), mPadding(0)
            {
                ;
            }
            void testMinMax(int ilane);
            void setOffset(float off)
            {
                mOffset = off;
            }
            void setPadding(float pad)
            {
                mPadding = pad;
            }
            void setOffsetYje(const SceneCoordinate& previous, float minimumJunction);
            void setOffsetYej(const SceneCoordinate& previous, float maximumBlock, float minimumJunction);
            void setOffsetX(const SceneCoordinate& previous, float maximumBlock, float sepOut, float sepInp);
            float lanePosition(int ilane) const;
            int preLanes() const
            {
                return -minLane;
            }
            float junctionEntry() const
            {
                return lanePosition(minLane);
            }
            float junctionExit() const
            {
                return lanePosition(maxLane - 1);
            }
            float xBoxOffset() const;
        };

        class EndpointCoordinate
        {
            float mYoffset;
            float mXoutput;
            float mXinput;
            float mPinDistance;
            float mTopPin;
            int mNumberPins;
            QMultiHash<u32, int> mInputHash;
            QMultiHash<u32, int> mOutputHash;

        public:
            EndpointCoordinate();
            void setInputPosition(QPointF p0pos);
            void setOutputPosition(QPointF p0pos);
            float lanePosition(int ilane, bool absolute) const;
            float xInput() const
            {
                return mXinput;
            }
            float xOutput() const
            {
                return mXoutput;
            }
            void setInputPins(const QList<u32>& pinList, float p0dist, float pdist);
            void setOutputPins(const QList<u32>& pinList, float p0dist, float pdist);
            int numberPins() const;
            QList<int> inputPinIndex(u32 id) const;
            QList<int> outputPinIndex(u32 id) const;
        };

        class EndpointList : public QList<NetLayoutPoint>
        {
        public:
            enum EndpointType
            {
                NoEndpoint           = 0,
                SingleSource         = 1,
                SingleDestination    = 2,
                SourceAndDestination = 3,
                ConstantLevel        = 4,
                HasGlobalEndpoint    = 5
            };
            EndpointList()
                : mNetType(NoEndpoint), mInputArrow(false), mOutputArrow(false)
            {;}
            void addSource(const NetLayoutPoint& pnt);
            void addDestination(const NetLayoutPoint& pnt);
            void setNetType(EndpointType tp) { mNetType = tp; }
            EndpointType netType() const { return mNetType; }
            bool isInput(int index) const { return mPointIsInput.at(index); }
            void setInputArrow() { mInputArrow = true; }
            bool hasInputArrow() const { return mInputArrow; }
            void setOutputArrow() { mOutputArrow = true; }
            bool hasOutputArrow() const { return  mOutputArrow; }

        private:
            EndpointType mNetType;
            QList<bool> mPointIsInput;
            bool mInputArrow;
            bool mOutputArrow;
        };

        class SeparatedNetWidth
        {
        public:
            float mInputSpace;
            float mOutputSpace;
            SeparatedNetWidth() : mInputSpace(0), mOutputSpace(0)
            {
                ;
            }
            void requireInputSpace(float spc);
            void requireOutputSpace(float spc);
        };

        struct Road
        {
            Road(const int x_coordinate, const int y_coordinate) : x(x_coordinate), y(y_coordinate), mLanes(0)
            {
            }

            int x;
            int y;

            unsigned int mLanes = 0;
        };

        struct Junction
        {
            Junction(const int x_coordinate, const int y_coordinate)
                : x(x_coordinate), y(y_coordinate), mHLanes(0), mVLanes(0), mCloseLeftLaneChanges(0), mCloseRightLaneChanges(0), mCloseTopLaneChanges(0), mCloseBottomLaneChanges(0),
                  mFarLeftLaneChanges(0), mFarRightLaneChanges(0), mFarTopLaneChanges(0), mFarBottomLaneChanges(0)
            {
            }

            int x;
            int y;

            unsigned int mHLanes = 0;
            unsigned int mVLanes = 0;

            unsigned int mCloseLeftLaneChanges   = 0;
            unsigned int mCloseRightLaneChanges  = 0;
            unsigned int mCloseTopLaneChanges    = 0;
            unsigned int mCloseBottomLaneChanges = 0;

            unsigned int mFarLeftLaneChanges   = 0;
            unsigned int mFarRightLaneChanges  = 0;
            unsigned int mFarTopLaneChanges    = 0;
            unsigned int mFarBottomLaneChanges = 0;
        };

        struct UsedPaths
        {
            QSet<Road*> mHRoads;
            QSet<Road*> mVRoads;

            QSet<Junction*> mHJunctions;
            QSet<Junction*> mVJunctions;

            QSet<Junction*> mCloseLeftJunctions;
            QSet<Junction*> mCloseRightJunctions;
            QSet<Junction*> mCloseTopJunctions;
            QSet<Junction*> mCloseBottomJunctions;

            QSet<Junction*> mFarLeftJunctions;
            QSet<Junction*> mFarRightJunctions;
            QSet<Junction*> mFarTopJunctions;
            QSet<Junction*> mFarBottomJunctions;
        };

    public:
        /**
         * Constructor.
         *
         * @param context - The GraphContext the layouter works on
         * @param parent - The parent QObject
         */
        explicit GraphLayouter(GraphContext *context, QObject* parent = nullptr);

        /**
         * Destructor.
         */
        ~GraphLayouter();

        /**
         * Gets the name of the layouter.
         *
         * @returns the name of the layouter.
         */
        virtual QString name() const = 0;

        /**
         * Gets an inforaml description of the layouter.
         *
         * @returns the description of the layouter.
         */
        virtual QString mDescription() const = 0;

        virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, PlacementHint placement = PlacementHint()) = 0;

        virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) = 0;

        /**
         * Does the actual layout process.
         */
        void layout();
        void alternateLayout();

        /**
         * Gets the GraphicsScene the layouter works on.
         *
         * @returns the GraphicScene.
         */
        GraphicsScene* scene() const;

        const QMap<Node, QPoint> nodeToPositionMap() const;
        const QMap<QPoint, Node> positionToNodeMap() const;
        NetLayoutPoint positonForNode(const Node& nd) const;

        QPoint gridPointByItem(GraphicsNode* item) const;

        void dumpNodePositions(const QPoint& search) const;

        void setNodePosition(const Node& n, const QPoint& p);
        void swapNodePositions(const Node& n1, const Node& n2);
        void removeNodeFromMaps(const Node& n);

        int minXIndex() const;
        int minYIndex() const;

        bool done() const;

        bool dumpJunctionEnabled();
        void setDumpJunctionEnabled(bool enabled);

        QVector<qreal> xValues() const;
        QVector<qreal> yValues() const;

        qreal maxNodeWidth() const;
        qreal maxNodeHeight() const;

        qreal defaultGridWidth() const;
        qreal defaultGridHeight() const;

        qreal gridXposition(int ix) const;
        qreal gridYposition(int iy) const;

        const NodeBoxes& boxes() const
        {
            return mBoxes;
        }

        void prepareRollback();
        bool canRollback() const;
        bool rollback();

    protected:
        GraphicsScene* mScene;
        GraphContext* mParentContext;
        QMap<Node, QPoint> mNodeToPositionMap;
        QMap<QPoint, Node> mPositionToNodeMap;
        QMap<Node, QPoint> mNodeToPositionRollback;

    private Q_SLOTS:
        void handleDrawNetThreadFinished();
        void handleJunctionThreadFinished();

    private:
        void clearLayoutData();
        void clearComments();
        void createBoxes();
        void getWireHash();
        void findMaxBoxDimensions();
        void findMaxChannelLanes();
        void calculateJunctionMinDistance();
        void calculateGateOffsets();
        void placeGates();
        void drawNets();
        void drawComments();
        void drawNetsIsolated(u32 id, Net* n, const EndpointList& epl);
        void updateSceneRect();
        static bool verifyModulePort(Net* n, const Node& modNode, bool isModInput);
        void handleCommentAboutToDeleted(CommentEntry* entry);
        void handleCommentAdded(CommentEntry* entry);

        bool boxExists(const int x, const int y) const;

        bool hRoadJumpPossible(const int x, const int y1, const int y2) const;
        bool hRoadJumpPossible(const Road* const r1, const Road* const r2) const;

        bool vRoadJumpPossible(const int x1, const int x2, const int y) const;
        bool vRoadJumpPossible(const Road* const r1, const Road* const r2) const;

        Junction* getJunction(const int x, const int y);

        qreal hRoadHeight(const unsigned int mLanes) const;
        qreal vRoadWidth(const unsigned int mLanes) const;

        void commitUsedPaths(const UsedPaths& used);
        static bool isConstNet(const Net* n);

        NodeBoxes mBoxes;

        QHash<QPoint, Junction*> mJunctions;

        QMap<int, qreal> mMaxNodeWidthForX;
        QMap<int, qreal> mMaxNodeHeightForY;

        QMap<int, qreal> mNodeOffsetForX;
        QMap<int, qreal> mNodeOffsetForY;

        QMap<int, qreal> mMaxLeftIoPaddingForChannelX;
        QMap<int, qreal> mMaxRightIoPaddingForChannelX;

        QSet<u32> mViewInput;
        QSet<u32> mViewOutput;

        int mMinXIndex;
        int mMinYIndex;

        int mMaxXIndex;
        int mMaxYIndex;

        QVector<qreal> mXValues;
        QVector<qreal> mYValues;

        qreal mMaxNodeWidth;
        qreal mMaxNodeHeight;

        bool mDone;
        int mRollbackStatus;

        QRect mNodeBoundingBox;
        NetLayoutConnectionMetric mConnectionMetric;
        QHash<NetLayoutWire, QList<u32>> mWireHash;
        QHash<NetLayoutPoint, NetLayoutJunctionEntries> mJunctionEntries;
        QHash<NetLayoutPoint, EndpointCoordinate> mEndpointHash;
        QHash<NetLayoutPoint, SeparatedNetWidth> mSeparatedWidth;
        QHash<NetLayoutPoint, float> mSpaceSeparatedOutputs;
        NetLayoutJunctionHash mJunctionHash;
        QMap<int, SceneCoordinate> mCoordX;
        QMap<int, SceneCoordinate> mCoordY;
        QMap<int, float> mJunctionMinDistanceY;
        QHash<u32, EndpointList> mWireEndpoint;
        QHash<u32, int> mGlobalInputHash;
        QHash<u32, int> mGlobalOutputHash;

        bool mDumpJunctions;
        QList<CommentSpeechBubble*> mCommentBubbles;
        QSet<u32> mNetsToDraw;
        QSet<u32>::const_iterator mNetIterator;
        QList<DrawNetThread*> mDrawNetThreads;
        QList<JunctionThread*> mJunctionThreads;
        QHash<u32, QHash<NetLayoutWire, int>> mLaneMap;
    };

    class DrawNetThread : public QThread
    {
        Q_OBJECT
        u32 mId;
        GraphLayouter* mLayouter;
        void drawJunction();
        void drawEndpoint();
    public:
        StandardGraphicsNet::Lines mLines;
        QList<QPointF> mKnots;
        DrawNetThread(u32 id, GraphLayouter* parent) : QThread(parent), mId(id), mLayouter(parent) {;}
        u32 id() const { return mId; }
        void run() override;
    };

    class JunctionThread : public QThread
    {
        Q_OBJECT
    public:
        NetLayoutPoint mNetLayoutPoint;
        NetLayoutJunctionEntries mEntries;
        NetLayoutJunction* mJunction;
        JunctionThread(const NetLayoutPoint& nlp, const NetLayoutJunctionEntries& entr)
            : mNetLayoutPoint(nlp), mEntries(entr), mJunction(nullptr) {;}
        void run() override;
    };
}    // namespace hal

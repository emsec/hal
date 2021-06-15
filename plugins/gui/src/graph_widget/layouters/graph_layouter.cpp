#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nets/standard_arrow_net.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/implementations/qpoint_extension.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <qmath.h>
#include <QDebug>
#include <QElapsedTimer>

namespace hal
{
    template<typename T1, typename T2>
    static void storeMax(QMap<T1, T2>& map, T1 key, T2 value)
    {
        if (map.contains(key))
            if (map.value(key) >= value)
                return;

        map.insert(key, value);
    }

    const static qreal sLaneSpacing             = 10;
    const static qreal sJunctionPadding         = 10;
    const static qreal sHRoadPadding           = 10;
    const static qreal sVRoadPadding           = 10;
    const static qreal sMinimumVChannelWidth  = 20;
    const static qreal sMinimumHChannelHeight = 20;

    GraphLayouter::GraphLayouter(const GraphContext* const context, QObject* parent) : QObject(parent), mScene(new GraphicsScene(this)), mContext(context), mDone(false), mOptimizeNetLayout(true)
    {
        SelectionDetailsWidget* details = gContentManager->getSelectionDetailsWidget();
        if (details) connect(details, &SelectionDetailsWidget::triggerHighlight, mScene, &GraphicsScene::handleHighlight);
    }

    GraphLayouter::~GraphLayouter()
    {
        mScene->deleteLater();
    }

    GraphicsScene* GraphLayouter::scene() const
    {
        return mScene;
    }

    const QMap<Node, QPoint> GraphLayouter::nodeToPositionMap() const
    {
        return mNodeToPositionMap;
    }

    void GraphLayouter::dumpNodePositions(const QPoint& search) const
    {
        QTextStream xout(stderr, QIODevice::WriteOnly);
        xout << "Node positions " << search.x() << " " << search.y() << "\n";
        xout.setFieldAlignment(QTextStream::AlignRight);
        xout.setFieldWidth(4);
        for (auto it = mPositionToNodeMap.constBegin(); it != mPositionToNodeMap.constEnd(); ++it)
        {
            xout << (it.key() == search ? "*" : " ") << it.key().x() << it.key().y() << (it.value().type() == Node::Module ? "M" : "G") << it.value().id() << "\n";
        }
        xout << "=======\n";
    }

    const QMap<QPoint,Node> GraphLayouter::positionToNodeMap() const
    {
        return mPositionToNodeMap;
    }

    void GraphLayouter::setNodePosition(const Node &n, const QPoint& p)
    {
        if (mNodeToPositionMap.contains(n))
        {
            QPoint old_p = mNodeToPositionMap.value(n);
            mPositionToNodeMap.remove(old_p);
        }

        mNodeToPositionMap.insert(n, p);
        mPositionToNodeMap.insert(p, n);

        //manual relayout call needed
    }

    void GraphLayouter::swapNodePositions(const Node &n1, const Node &n2)
    {
        assert(mNodeToPositionMap.contains(n1));
        assert(mNodeToPositionMap.contains(n2));

        QPoint p1 = mNodeToPositionMap.value(n1);
        QPoint p2 = mNodeToPositionMap.value(n2);

        mNodeToPositionMap.insert(n1, p2);    // implicit replace
        mNodeToPositionMap.insert(n2, p1);

        mPositionToNodeMap.insert(p1, n2);
        mPositionToNodeMap.insert(p2, n1);
    }

    void GraphLayouter::removeNodeFromMaps(const Node &n)
    {
        if (mNodeToPositionMap.contains(n))
        {
            QPoint old_p = mNodeToPositionMap.value(n);
            mNodeToPositionMap.remove(n);
            mPositionToNodeMap.remove(old_p);
        }
    }

    QPoint GraphLayouter::gridPointByItem(GraphicsNode* item) const
    {
        QPoint retval(INT_MIN,INT_MIN);
        if (!item) return retval;
        const NodeBox* nbox = mBoxes.boxForItem(item);
        if (!nbox) return retval;
        return QPoint(nbox->x(),nbox->y());
    }

    int GraphLayouter::minXIndex() const
    {
        return mMinXIndex;
    }

    int GraphLayouter::minYIndex() const
    {
        return mMinYIndex;
    }

    bool GraphLayouter::done() const
    {
        return mDone;
    }

    QVector<qreal> GraphLayouter::xValues() const
    {
        return mXValues;
    }

    QVector<qreal> GraphLayouter::yValues() const
    {
        return mYValues;
    }

    qreal GraphLayouter::maxNodeWidth() const
    {
        return mMaxNodeWidth;
    }

    qreal GraphLayouter::maxNodeHeight() const
    {
        return mMaxNodeHeight;
    }

    qreal GraphLayouter::defaultGridWidth() const
    {
        return mMaxNodeWidth + sMinimumVChannelWidth;
    }

    qreal GraphLayouter::defaultGridHeight() const
    {
        return mMaxNodeHeight + sMinimumHChannelHeight;
    }

    void GraphLayouter::alternateLayout()
    {
        getWireHash();

        findMaxBoxDimensions();
        alternateMaxChannelLanes();
        calculateJunctionMinDistance();
        alternateGateOffsets();
        alternatePlaceGates();
        mDone = true;
        alternateDrawNets();
        updateSceneRect();

        mScene->moveNetsToBackground();
        mScene->handleExternSelectionChanged(nullptr);

    #ifdef GUI_DEBUG_GRID
        mScene->debugSetLayouterGrid(xValues(), yValues(), defaultGridHeight(), defaultGridWidth());
    #endif
    }

    void GraphLayouter::layout()
    {
        QElapsedTimer timer;
        timer.start();
        mScene->deleteAllItems();
        clearLayoutData();

        createBoxes();
        if (mOptimizeNetLayout)
        {
            alternateLayout();
            qDebug() << "elapsed time (experimental new) layout [ms]" << timer.elapsed();
            return;
        }
        calculateNets();
        findMaxBoxDimensions();
        findMaxChannelLanes();
        resetRoadsAndJunctions();
        calculateMaxChannelDimensions();
        calculateGateOffsets();
        placeGates();
        mDone = true;
        drawNets();
        updateSceneRect();

        mScene->moveNetsToBackground();
        mScene->handleExternSelectionChanged(nullptr);

    #ifdef GUI_DEBUG_GRID
        mScene->debugSetLayouterGrid(xValues(), yValues(), defaultGridHeight(), defaultGridWidth());
    #endif
        qDebug() << "elapsed time (classic) layout [ms]" << timer.elapsed();
    }

    void GraphLayouter::clearLayoutData()
    {
        mDone = false;

        mBoxes.clearBoxes();

        for (const GraphLayouter::Road* r : mHRoads.values())
            delete r;
        mHRoads.clear();

        for (const GraphLayouter::Road* r : mVRoads.values())
            delete r;
        mVRoads.clear();

        for (const GraphLayouter::Junction* j : mJunctions.values())
            delete j;
        mJunctions.clear();

        mMaxNodeWidthForX.clear();
        mMaxNodeHeightForY.clear();

        mMaxVChannelLanesForX.clear();
        mMaxHChannelLanesForY.clear();

        mMaxVChannelLeftSpacingForX.clear();
        mMaxVChannelRightSpacingForX.clear();
        mMaxHChannelTopSpacingForY.clear();
        mMaxHChannelBottomSpacingForY.clear();

        mMaxVChannelWidthForX.clear();
        mMaxHChannelHeightForY.clear();

        mNodeOffsetForX.clear();
        mNodeOffsetForY.clear();

        mMaxLeftJunctionSpacingForX.clear();
        mMaxRightJunctionSpacingForX.clear();

        mMaxTopJunctionSpacingForY.clear();
        mMaxBottomJunctionSpacingForY.clear();

        mMaxLeftIoPaddingForChannelX.clear();
        mMaxRightIoPaddingForChannelX.clear();

        mMinXIndex = 0;
        mMinYIndex = 0;

        mMaxXIndex = 0;
        mMaxYIndex = 0;

        mXValues.clear();
        mYValues.clear();

        mMaxNodeWidth  = 0;
        mMaxNodeHeight = 0;

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
        mGlobalInputHash.clear();
        mGlobalOutputHash.clear();
        mNodeBoundingBox = QRect();
    }

    void GraphLayouter::createBoxes()
    {
        bool first = true;
        int xmin, xmax, ymin, ymax;
        xmin = ymin = xmax = ymax = 0;
        QMap<QPoint, Node>::const_iterator i = positionToNodeMap().constBegin();
        while (i != positionToNodeMap().constEnd())
        {
            int x = i.key().x();
            int y = i.key().y();
            if (first || x+1 > xmax) xmax = x+1;
            if (first || y+1 > ymax) ymax = y+1;
            if (first || x < xmin) xmin = x;
            if (first || y < ymin) ymin = y;
            first = false;
            mBoxes.addBox(i.value(), x, y);
            ++i;
        }
        mNodeBoundingBox = QRect(xmin, ymin, xmax-xmin, ymax-ymin);
    }

    bool GraphLayouter::verifyModulePort(const Net* n, const Node &modNode, bool isModInput)
    {
        // bypass test for gates
        if (modNode.type() != Node::Module) return true;

        Module* m = gNetlist->get_module_by_id(modNode.id());
        Q_ASSERT(m);
        std::vector<Net*> knownNets = isModInput ? m->get_input_nets() : m->get_output_nets();
        for (const Net* kn : knownNets)
            if (kn == n) return true;
        return false;
    }

    void GraphLayouter::getWireHash()
    {
        for (const u32 id : mContext->nets())
        {
            Net* n = gNetlist->get_net_by_id(id);
            if (!n) continue;

            QSet<NetLayoutPoint> srcPoints;
            QSet<NetLayoutPoint> dstPoints;

            mWireEndpoint[id] = EndpointList();

            for (const Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                const NodeBox* srcBox = mBoxes.boxForGate(src->get_gate());
                if (!srcBox)
                    continue;

                if (!verifyModulePort(n,srcBox->getNode(),false))
                    continue;

                NetLayoutPoint srcPnt(srcBox->x()+1,2*srcBox->y());
                srcPoints.insert(srcPnt);
                mWireEndpoint[id].addSource(srcPnt);
            }

            for (const Endpoint* dst : n->get_destinations())
            {
                // find dst box
                const NodeBox* dstBox = mBoxes.boxForGate(dst->get_gate());
                if (!dstBox)
                    continue;

                if (!verifyModulePort(n,dstBox->getNode(),true))
                    continue;

                NetLayoutPoint dstPnt(dstBox->x(),2*dstBox->y());
                dstPoints.insert(dstPnt);
                mWireEndpoint[id].addDestination(dstPnt);
            }

            if (isConstNet(n))
                mWireEndpoint[id].setNetType(EndpointList::ConstantLevel);

            // test for global inputs
            EndpointList::EndpointType nType = mWireEndpoint.value(id).netType();
            if ((nType == EndpointList::SingleDestination && dstPoints.size() > 1) ||
                    (nType == EndpointList::SourceAndDestination && n->is_global_input_net()))
            {
                // global input connects to multiple boxes
                int ypos = mGlobalInputHash.size();
                NetLayoutPoint srcPnt(mNodeBoundingBox.left(),2*ypos);
                srcPoints.insert(srcPnt);
                mWireEndpoint[id].addSource(srcPnt);
                mGlobalInputHash[id] = ypos;
                mWireEndpoint[id].setNetType(EndpointList::MultipleDestination);
            }

            if ((nType == EndpointList::SingleSource && srcPoints.size() > 1) ||
                    (nType == EndpointList::SourceAndDestination && n->is_global_output_net()))
            {
                // multi-driven global output or global output back coupled to net gate
                int ypos = mGlobalOutputHash.size();
                NetLayoutPoint dstPnt(mNodeBoundingBox.right()+1,2*ypos);
                dstPoints.insert(dstPnt);
                mWireEndpoint[id].addDestination(dstPnt);
                mGlobalOutputHash[id] = ypos;
                mWireEndpoint[id].setNetType(EndpointList::MultipleSource);
            }

            const EndpointList& epl = mWireEndpoint.value(id);
            switch (epl.netType()) {
            case EndpointList::SingleSource:
            case EndpointList::SingleDestination:
            case EndpointList::ConstantLevel:
            {
                int ipnt = 0;
                for (const NetLayoutPoint& pnt : epl)
                {
                    bool isInput = epl.isInput(ipnt++);
                    SeparatedGraphicsNet* net_item = epl.netType() == EndpointList::ConstantLevel
                            ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n,QString::fromStdString(n->get_name())))
                            : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));
                    if (isInput)
                        mSeparatedWidth[pnt].requireInputSpace(net_item->inputWidth()+sLaneSpacing);
                    else
                    {
                        const NodeBox* nb = mBoxes.boxForPoint(QPoint(pnt.x()-1,pnt.y()/2));
                        Q_ASSERT(nb);
                        mSeparatedWidth[pnt].requireOutputSpace(
                                    nb->item()->width() + net_item->outputWidth() + sLaneSpacing);
                    }
                    delete net_item;
                }
            }
                break;
            case EndpointList::SourceAndDestination:
            case EndpointList::MultipleDestination:
            case EndpointList::MultipleSource:
            {
                NetLayoutConnectionFactory nlcf(srcPoints.toList(),dstPoints.toList());
                // nlcf.dump(QString("wire %1").arg(id));
                mConnectionMetric.insert(NetLayoutMetric(id,nlcf.connection),nlcf.connection);
            }
                break;
            default:
                break;
            }
        }

        /// logic nets -> wire mLanes
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
                NetLayoutPoint pnt = iend
                        ? it.key().endPoint(NetLayoutWire::SourcePoint)
                        : it.key().endPoint(NetLayoutWire::DestinationPoint);
                int idirBase = it.key().isHorizontal() ? NetLayoutDirection::Left : NetLayoutDirection::Up;
                mJunctionEntries[pnt].setEntries(idirBase+iend, it.value());
            }
        }

        /// end points -> junction entries
        for (const NodeBox* nbox : mBoxes)
        {
            NetLayoutPoint inPnt(nbox->x(),nbox->y()*2);
            QList<u32> inpNets = nbox->item()->inputNets();
            mJunctionEntries[inPnt].setEntries(NetLayoutDirection::Right, nbox->item()->inputNets());
            mEndpointHash[inPnt].setInputPins(nbox->item()->inputNets(),
                                              nbox->item()->yTopPinDistance(),
                                              nbox->item()->yEndpointDistance());
            NetLayoutPoint outPnt(nbox->x()+1,nbox->y()*2);
            mJunctionEntries[outPnt].setEntries(NetLayoutDirection::Left, nbox->item()->outputNets());
            mEndpointHash[outPnt].setOutputPins(nbox->item()->outputNets(),
                                                nbox->item()->yTopPinDistance(),
                                                nbox->item()->yEndpointDistance());
        }

        for (auto itGlInp = mGlobalInputHash.constBegin(); itGlInp != mGlobalInputHash.constEnd(); ++itGlInp)
        {
            QList<u32> netIds;
            netIds.append(itGlInp.key());
            NetLayoutPoint pnt(mNodeBoundingBox.left(), 2*itGlInp.value());
            mJunctionEntries[pnt].setEntries(NetLayoutDirection::Left, netIds);
            if (!mEndpointHash.contains(pnt))
                mEndpointHash[pnt].setOutputPins(netIds,0,0);
        }

        for (auto itGlOut = mGlobalOutputHash.constBegin(); itGlOut != mGlobalOutputHash.constEnd(); ++itGlOut)
        {
            QList<u32> netIds;
            netIds.append(itGlOut.key());
            NetLayoutPoint pnt(mNodeBoundingBox.right()+1, 2*itGlOut.value());
            mJunctionEntries[pnt].setEntries(NetLayoutDirection::Right, netIds);
            if (!mEndpointHash.contains(pnt))
                mEndpointHash[pnt].setInputPins(netIds,0,0);
        }

        for (auto it = mJunctionEntries.constBegin(); it != mJunctionEntries.constEnd(); ++it)
        {
//            it.value().dumpFile(it.key());
//            qDebug() << "Junction at" << it.key().x() << it.key().y();
            NetLayoutJunction* nlj = new NetLayoutJunction(it.value());
            if (nlj->lastError() != NetLayoutJunction::Ok)
                qDebug() << "Junction route error" << nlj->lastError() << it.key();
            mJunctionHash.insert(it.key(),nlj);
        }
    }

    void GraphLayouter::calculateNets()
    {
        for (const u32 id : mContext->nets())
        {
            Net* n = gNetlist->get_net_by_id(id);
            assert(n);

            if (n->is_unrouted())
                continue;

            UsedPaths used;

            for (Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                Node node = mContext->nodeForGate(src->get_gate()->get_id());

                if (node.isNull())
                    continue;

                NodeBox* src_box = mBoxes.boxForNode(node);
                assert(src_box);

                // FOR EVERY DST
                for (Endpoint* dst : n->get_destinations())
                {
                    // FIND DST BOX
                    node = mContext->nodeForGate(dst->get_gate()->get_id());
                    if (node.isNull())
                        continue;

                    NodeBox* dst_box = mBoxes.boxForNode(node);
                    assert(dst_box);

                    // ROAD BASED DISTANCE (x_distance - 1)
                    const int x_distance = dst_box->x() - src_box->x() - 1;
                    const int y_distance = dst_box->y() - src_box->y();

                    if (!y_distance && vRoadJumpPossible(src_box->x() + 1, dst_box->x(), src_box->y()))
                    {
                        // SPECIAL CASE INDIRECT HORIZONTAL NEIGHBORS
                        Road* dst_v_road = getVRoad(dst_box->x(), dst_box->y());
                        used.mVRoads.insert(dst_v_road);
                        continue;
                    }

                    Road* src_v_road = getVRoad(src_box->x() + 1, src_box->y());

                    if (!(x_distance || y_distance))
                    {
                        // SPECIAL CASE DIRECT HORIZONTAL NEIGHBORS
                        used.mVRoads.insert(src_v_road);
                        continue;
                    }

                    // NORMAL CASE
                    // CONNECT SRC TO V ROAD, TRAVEL X DISTANCE, TRAVEL Y DISTANCE, CONNECT V ROAD TO DST
                    used.mVRoads.insert(src_v_road);

                    Junction* initial_junction = nullptr;
                    int remaining_y_distance   = y_distance;

                    if (y_distance < 0)
                    {
                        // TRAVEL UP
                        initial_junction = getJunction(src_v_road->x, src_v_road->y);

                        if (src_v_road->mLanes != initial_junction->mVLanes)
                        {
                            if (src_v_road->mLanes < initial_junction->mVLanes)
                                used.mCloseBottomJunctions.insert(initial_junction);
                            else
                                used.mFarBottomJunctions.insert(initial_junction);
                        }
                    }
                    else
                    {
                        // TRAVEL DOWN
                        initial_junction = getJunction(src_v_road->x, src_v_road->y + 1);

                        if (src_v_road->mLanes != initial_junction->mVLanes)
                        {
                            if (src_v_road->mLanes < initial_junction->mVLanes)
                                used.mCloseTopJunctions.insert(initial_junction);
                            else
                                used.mFarTopJunctions.insert(initial_junction);
                        }

                        if (!y_distance)
                            remaining_y_distance = -1;
                    }

                    used.mVJunctions.insert(initial_junction);

                    Junction* last_junction = initial_junction;

                    if (x_distance)
                    {
                        used.mHJunctions.insert(initial_junction);

                        int remaining_x_distance = x_distance;

                        // TRAVEL REMAINING X DISTANCE
                        while (remaining_x_distance)
                        {
                            Road* r     = nullptr;
                            Junction* j = nullptr;

                            if (x_distance > 0)
                            {
                                // TRAVEL RIGHT
                                r = getHRoad(last_junction->x, last_junction->y);

                                if (last_junction->mHLanes != r->mLanes)
                                {
                                    if (last_junction->mHLanes < r->mLanes)
                                        used.mFarRightJunctions.insert(last_junction);
                                    else
                                        used.mCloseRightJunctions.insert(last_junction);
                                }

                                j = getJunction(last_junction->x + 1, last_junction->y);

                                if (r->mLanes != j->mHLanes)
                                {
                                    if (r->mLanes < j->mHLanes)
                                        used.mCloseLeftJunctions.insert(j);
                                    else
                                        used.mFarLeftJunctions.insert(j);
                                }

                                --remaining_x_distance;
                            }
                            else
                            {
                                // TRAVEL LEFT
                                r = getHRoad(last_junction->x - 1, last_junction->y);

                                if (last_junction->mHLanes != r->mLanes)
                                {
                                    if (last_junction->mHLanes < r->mLanes)
                                        used.mFarLeftJunctions.insert(last_junction);
                                    else
                                        used.mCloseLeftJunctions.insert(last_junction);
                                }

                                j = getJunction(last_junction->x - 1, last_junction->y);

                                if (r->mLanes != j->mHLanes)
                                {
                                    if (r->mLanes < j->mHLanes)
                                        used.mCloseRightJunctions.insert(j);
                                    else
                                        used.mFarRightJunctions.insert(j);
                                }

                                ++remaining_x_distance;
                            }

                            used.mHRoads.insert(r);
                            used.mHJunctions.insert(j);

                            last_junction = j;
                        }

                        used.mVJunctions.insert(last_junction);
                    }

                    // TRAVEL REMAINING Y DISTANCE
                    if (remaining_y_distance > 0)
                    {
                        while (remaining_y_distance != 1)
                        {
                            // TRAVEL DOWN
                            Road* r = getVRoad(last_junction->x, last_junction->y);

                            if (last_junction->mVLanes != r->mLanes)
                            {
                                if (last_junction->mVLanes < r->mLanes)
                                    used.mFarBottomJunctions.insert(last_junction);
                                else
                                    used.mCloseBottomJunctions.insert(last_junction);
                            }

                            Junction* j = getJunction(last_junction->x, last_junction->y + 1);

                            if (r->mLanes != j->mVLanes)
                            {
                                if (r->mLanes < j->mVLanes)
                                    used.mCloseTopJunctions.insert(j);
                                else
                                    used.mFarTopJunctions.insert(j);
                            }

                            used.mVRoads.insert(r);
                            used.mVJunctions.insert(j);

                            last_junction = j;

                            --remaining_y_distance;
                        }
                    }
                    else
                    {
                        while (remaining_y_distance != -1)
                        {
                            // TRAVEL UP
                            Road* r = getVRoad(last_junction->x, last_junction->y - 1);

                            if (last_junction->mVLanes != r->mLanes)
                            {
                                if (last_junction->mVLanes < r->mLanes)
                                    used.mFarTopJunctions.insert(last_junction);
                                else
                                    used.mCloseTopJunctions.insert(last_junction);
                            }

                            Junction* j = getJunction(last_junction->x, last_junction->y - 1);

                            if (r->mLanes != j->mVLanes)
                            {
                                if (r->mLanes < j->mVLanes)
                                    used.mCloseBottomJunctions.insert(j);
                                else
                                    used.mFarBottomJunctions.insert(j);
                            }

                            used.mVRoads.insert(r);
                            used.mVJunctions.insert(j);

                            last_junction = j;

                            ++remaining_y_distance;
                        }
                    }

                    Road* dst_road = nullptr;

                    if (y_distance > 0)
                    {
                        // TRAVEL DOWN
                        dst_road = getVRoad(last_junction->x, last_junction->y);

                        if (last_junction->mVLanes != dst_road->mLanes)
                        {
                            if (last_junction->mVLanes < dst_road->mLanes)
                                used.mFarBottomJunctions.insert(last_junction);
                            else
                                used.mCloseBottomJunctions.insert(last_junction);
                        }
                    }
                    else
                    {
                        // TRAVEL UP
                        dst_road = getVRoad(last_junction->x, last_junction->y - 1);

                        if (last_junction->mVLanes != dst_road->mLanes)
                        {
                            if (last_junction->mVLanes < dst_road->mLanes)
                                used.mFarTopJunctions.insert(last_junction);
                            else
                                used.mCloseTopJunctions.insert(last_junction);
                        }
                    }

                    used.mVJunctions.insert(last_junction);
                    used.mVRoads.insert(dst_road);
                }
            }

            commitUsedPaths(used);
        }
    }

    void GraphLayouter::findMaxBoxDimensions()
    {
        for (const NodeBox* box : mBoxes)
        {
            if (box->x() < mMinXIndex)
                mMinXIndex = box->x();
            else if (box->x() > mMaxXIndex)
                mMaxXIndex = box->x();

            if (box->y() < mMinYIndex)
                mMinYIndex = box->y();
            else if (box->y() > mMaxYIndex)
                mMaxYIndex = box->y();

            if (mMaxNodeWidth < box->item()->width())
                mMaxNodeWidth = box->item()->width();

            if (mMaxNodeHeight < box->item()->height())
                mMaxNodeHeight = box->item()->height();

            storeMax(mMaxNodeWidthForX, box->x(), box->item()->width());
            storeMax(mMaxNodeHeightForY, box->y(), box->item()->height());

            storeMax(mMaxRightIoPaddingForChannelX, box->x(), box->inputPadding());
            storeMax(mMaxLeftIoPaddingForChannelX, box->x() + 1, box->outputPadding());
        }
    }

    void GraphLayouter::alternateMaxChannelLanes()
    {
        // maximum parallel wires for atomic network
        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            const NetLayoutPoint& pnt =
                    it.key().endPoint(NetLayoutWire::SourcePoint);
            unsigned int nw = it.value().size();
            if (it.key().isHorizontal())
                mCoordY[pnt.y()].testMinMax(nw);
            else
                mCoordX[pnt.x()].testMinMax(nw);
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

        // fill gaps in coordinate system if any
        if (!mCoordX.isEmpty())
        {
            auto itx0 = mCoordX.begin();
            for (auto itx1 = itx0+1; itx1 != mCoordX.end(); ++itx1)
            {
                for (int x = itx0.key()+1; x<itx1.key(); x++)
                    mCoordX[x].testMinMax(0);
                itx0 = itx1;
            }
        }
        if (!mCoordY.isEmpty())
        {
            auto ity0 = mCoordY.begin();
            for (auto ity1 = ity0+1; ity1 != mCoordY.end(); ++ity1)
            {
                for (int y = ity0.key()+1; y<ity1.key(); y++)
                    mCoordY[y].testMinMax(0);
                ity0 = ity1;
            }
        }
    }

    void GraphLayouter::findMaxChannelLanes()
    {
        for (const Road* r : mVRoads.values())
            storeMax(mMaxVChannelLanesForX, r->x, r->mLanes);

        for (const Road* r : mHRoads.values())
            storeMax(mMaxHChannelLanesForY, r->y, r->mLanes);

        for (const Junction* j : mJunctions.values())
        {
            storeMax(mMaxVChannelLanesForX, j->x, j->mVLanes);
            storeMax(mMaxHChannelLanesForY, j->y, j->mHLanes);
        }
    }

    void GraphLayouter::resetRoadsAndJunctions()
    {
        for (Road* r : mHRoads.values())
            r->mLanes = 0;

        for (Road* r : mVRoads.values())
            r->mLanes = 0;

        for (Junction* j : mJunctions.values())
        {
            // LEFT
            unsigned int combined_lane_changes = j->mCloseLeftLaneChanges + j->mFarLeftLaneChanges;
            qreal spacing                      = 0;

            if (combined_lane_changes)
                spacing = (combined_lane_changes - 1) * sLaneSpacing + sJunctionPadding;

            storeMax(mMaxLeftJunctionSpacingForX, j->x, spacing);

            // RIGHT
            combined_lane_changes = j->mCloseRightLaneChanges + j->mFarRightLaneChanges;
            spacing               = 0;

            if (combined_lane_changes)
                spacing = (combined_lane_changes - 1) * sLaneSpacing + sJunctionPadding;

            storeMax(mMaxRightJunctionSpacingForX, j->x, spacing);

            // TOP
            combined_lane_changes = j->mCloseTopLaneChanges + j->mFarTopLaneChanges;
            spacing               = 0;

            if (combined_lane_changes)
                spacing = (combined_lane_changes - 1) * sLaneSpacing + sJunctionPadding;

            storeMax(mMaxTopJunctionSpacingForY, j->y, spacing);

            // BOTTOM
            combined_lane_changes = j->mCloseBottomLaneChanges + j->mFarBottomLaneChanges;
            spacing               = 0;

            if (combined_lane_changes)
                spacing = (combined_lane_changes - 1) * sLaneSpacing + sJunctionPadding;

            storeMax(mMaxBottomJunctionSpacingForY, j->y, spacing);

            j->mHLanes = 0;
            j->mVLanes = 0;

            j->mCloseLeftLaneChanges   = 0;
            j->mCloseRightLaneChanges  = 0;
            j->mCloseTopLaneChanges    = 0;
            j->mCloseBottomLaneChanges = 0;

            j->mFarLeftLaneChanges   = 0;
            j->mFarRightLaneChanges  = 0;
            j->mFarTopLaneChanges    = 0;
            j->mFarBottomLaneChanges = 0;
        }
    }

    void GraphLayouter::calculateMaxChannelDimensions()
    {
        auto i = mMaxVChannelLanesForX.constBegin();
        while (i != mMaxVChannelLanesForX.constEnd())
        {
            qreal left_spacing = std::max(sVRoadPadding + mMaxLeftIoPaddingForChannelX.value(i.key()), mMaxLeftJunctionSpacingForX.value(i.key()));
            mMaxVChannelLeftSpacingForX.insert(i.key(), left_spacing);

            qreal right_spacing = std::max(sVRoadPadding + mMaxRightIoPaddingForChannelX.value(i.key()), mMaxRightJunctionSpacingForX.value(i.key()));
            mMaxVChannelRightSpacingForX.insert(i.key(), right_spacing);

            qreal width = left_spacing + right_spacing;

            if (i.value())
                width += (i.value() - 1) * sLaneSpacing;

            mMaxVChannelWidthForX.insert(i.key(), std::max(width, sMinimumVChannelWidth));

            ++i;
        }

        i = mMaxHChannelLanesForY.constBegin();
        while (i != mMaxHChannelLanesForY.constEnd())
        {
            qreal top_spacing = std::max(sHRoadPadding, mMaxTopJunctionSpacingForY.value(i.key()));
            mMaxHChannelTopSpacingForY.insert(i.key(), top_spacing);

            qreal bottom_spacing = std::max(sHRoadPadding, mMaxBottomJunctionSpacingForY.value(i.key()));
            mMaxHChannelBottomSpacingForY.insert(i.key(), bottom_spacing);

            qreal height = top_spacing + bottom_spacing;

            if (i.value())
                height += (i.value() - 1) * sLaneSpacing;

            mMaxHChannelHeightForY.insert(i.key(), std::max(height, sMinimumHChannelHeight));

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
                minDistance = (j0->rect().bottom() + 1) * sLaneSpacing
                        - itEdp.value().lanePosition(j1->rect().top(),false);
            }
            else
            {
                // endpoint -> net junction
                minDistance = itEdp.value().lanePosition(j0->rect().bottom(),false)
                        + (1-j1->rect().top()) * sLaneSpacing;
            }
            if (minDistance > mJunctionMinDistanceY[iy]) mJunctionMinDistanceY[iy] = minDistance;
        }
    }

    void GraphLayouter::alternateGateOffsets()
    {
        QHash<int,float> xInputPadding;
        QHash<int,float> xOutputPadding;
        for (auto itSep = mSeparatedWidth.constBegin(); itSep != mSeparatedWidth.constEnd(); itSep++)
        {
            NetLayoutJunction* jx = mJunctionHash.value(itSep.key());
            if (!jx) continue;
            int ix = itSep.key().x();
            float xinp = jx->rect().right() * sLaneSpacing + itSep.value().mInputSpace;
            float xout = itSep.value().mOutputSpace - jx->rect().left() * sLaneSpacing;
            if (xinp > xInputPadding[ix])  xInputPadding[ix] = xinp;
            if (xout > xOutputPadding[ix]) xOutputPadding[ix] = xout;
        }

        int ix0 = mNodeBoundingBox.x();

        float x0 = mCoordX[ix0].preLanes() * sLaneSpacing + sHRoadPadding;
        if (!mGlobalInputHash.isEmpty()) x0 += 50;
        mCoordX[ix0].setOffset(x0);
        mCoordX[ix0].setPadding(xInputPadding[ix0]);

        mXValues.append(mCoordX.value(ix0).xBoxOffset());

        auto itxLast = mCoordX.begin();
        for(auto itNext = itxLast + 1; itNext!= mCoordX.end(); ++itNext)
        {
            ix0 = itxLast.key();
            int ix1 = itNext.key();
            float xsum = 0;

            // loop in case that we span several columns
            for (int ix=ix0; ix<ix1;ix++)
            {
                auto xn = mMaxNodeWidthForX.find(ix);
                if (xn != mMaxNodeWidthForX.end())
                    xsum += xn.value();
            }
            itNext->setOffsetX(itxLast.value(), xsum + 2*sHRoadPadding, xOutputPadding[ix1], xInputPadding[ix1]);
            mXValues.append(itNext.value().xBoxOffset());
            itxLast = itNext;
        }

        int iy0 = mNodeBoundingBox.y();
        float y0 = mCoordY[iy0].preLanes() * sLaneSpacing + sVRoadPadding;
        mCoordY[iy0].setOffset(y0);
        mYValues.append(mCoordY.value(iy0).lanePosition(0));
        auto ityLast = mCoordY.begin();
        for(auto itNext = ityLast + 1; itNext!= mCoordY.end(); ++itNext)
        {
            iy0 = ityLast.key();
            int iy1 = itNext.key();
            Q_ASSERT(iy1 == iy0+1);
            if (iy0 % 2 != 0)
            {
                // netjunction -> endpoint
                itNext->setOffsetYje(ityLast.value(), mJunctionMinDistanceY.value(iy1));
                mYValues.append(itNext.value().lanePosition(0));
            }
            else
            {
                // endpoint -> netjunction
                float ydelta = sVRoadPadding;
                auto yn = mMaxNodeHeightForY.find(iy0/2);
                if (yn != mMaxNodeHeightForY.constEnd())
                    ydelta += yn.value();
                itNext->setOffsetYej(ityLast.value(), ydelta, mJunctionMinDistanceY.value(iy1));
            }
            ityLast = itNext;
        }
    }

    void GraphLayouter::calculateGateOffsets()
    {
        mNodeOffsetForX.insert(0, 0);
        mNodeOffsetForY.insert(0, 0);

        mXValues.append(0);
        mYValues.append(0);

        if (mMaxXIndex)
            for (int i = 1; i <= mMaxXIndex; ++i)
            {
                qreal offset = mNodeOffsetForX.value(i - 1) + mMaxNodeWidthForX.value(i - 1) + std::max(mMaxVChannelWidthForX.value(i), sMinimumVChannelWidth);
                mNodeOffsetForX.insert(i, offset);
                mXValues.append(offset);
            }

        if (mMinXIndex)
            for (int i = -1; i >= mMinXIndex; --i)
            {
                qreal offset = mNodeOffsetForX.value(i + 1) - mMaxNodeWidthForX.value(i) - std::max(mMaxVChannelWidthForX.value(i + 1), sMinimumVChannelWidth);
                mNodeOffsetForX.insert(i, offset);
                mXValues.prepend(offset);
            }

        if (mMaxYIndex)
            for (int i = 1; i <= mMaxYIndex; ++i)
            {
                qreal offset = mNodeOffsetForY.value(i - 1) + mMaxNodeHeightForY.value(i - 1) + std::max(mMaxHChannelHeightForY.value(i), sMinimumHChannelHeight);
                mNodeOffsetForY.insert(i, offset);
                mYValues.append(offset);
            }

        if (mMinYIndex)
            for (int i = -1; i >= mMinYIndex; --i)
            {
                qreal offset = mNodeOffsetForY.value(i + 1) - mMaxNodeHeightForY.value(i) - std::max(mMaxHChannelHeightForY.value(i + 1), sMinimumHChannelHeight);
                mNodeOffsetForY.insert(i, offset);
                mYValues.prepend(offset);
            }
    }

    void GraphLayouter::alternatePlaceGates()
    {
        for (const NodeBox* box : mBoxes)
        {
            box->item()->setPos(mCoordX[box->x()].xBoxOffset(),
                             mCoordY[box->y()*2].lanePosition(0));
            mScene->addGraphItem(box->item());

            NetLayoutPoint outPnt(box->x()+1,box->y()*2);
            QPointF outPos = box->item()->endpointPositionByIndex(0,false);
            mEndpointHash[outPnt].setOutputPosition(outPos);

            NetLayoutPoint inPnt(box->x(),box->y()*2);
            QPointF inPos = box->item()->endpointPositionByIndex(0,true);
            mEndpointHash[inPnt].setInputPosition(inPos);
        }

        /// place endpoints which are not connected to any box
        for (auto itEp = mEndpointHash.begin(); itEp != mEndpointHash.end(); ++itEp)
        {
            if (itEp.value().lanePosition(0,true) <= 0)
            {
                float px = mCoordX[itEp.key().x()].lanePosition(-1);
                float py = mCoordY[itEp.key().y()].lanePosition(0);
                itEp->setOutputPosition(QPointF(px,py));
            }
        }
    }

    void GraphLayouter::placeGates()
    {
        for (NodeBox* box : mBoxes)
        {
            box->setItemPosition(mNodeOffsetForX.value(box->x()), mNodeOffsetForY.value(box->y()));
            mScene->addGraphItem(box->item());
        }
    }

    void GraphLayouter::alternateDrawNets()
    {
        // lane for given wire and net id
        QHash<u32,QHash<NetLayoutWire,int>> laneMap;

        for (auto it=mWireHash.constBegin(); it!=mWireHash.constEnd(); ++it)
        {
            int ilane = 0;
            for(u32 id : it.value())
                laneMap[id].insert(it.key(),ilane++);
        }

        for (const u32 id : mContext->nets())
        {
            Net* n = gNetlist->get_net_by_id(id);
            if (!n) continue;

            const EndpointList& epl = mWireEndpoint.value(id);
            bool regularNet = false;

            switch(epl.netType())
            {
            case EndpointList::NoEndpoint:
                break;
            case EndpointList::SingleSource:
            case EndpointList::SingleDestination:
            case EndpointList::ConstantLevel:
                drawNetsIsolated(id,n,epl);
                break;;
            default:
                regularNet = true;
                break;
            }


            if (!regularNet)
                continue;

            StandardGraphicsNet::Lines lines;

            const QHash<NetLayoutWire,int>& wMap = laneMap.value(id);
            for (auto it=wMap.constBegin(); it!=wMap.constEnd(); ++it)
            {
                NetLayoutPoint wFromPoint = it.key().endPoint(NetLayoutWire::SourcePoint);
                NetLayoutPoint wToPoint   = it.key().endPoint(NetLayoutWire::DestinationPoint);
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
                    lines.appendHLine(x0,x1,yy);
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
//                        if (itEpc==mEndpointHash.constEnd())
//                            qDebug() << "xxx to endp" << wToPoint.x() << wToPoint.y() << y0 << y1;
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
//                        if (itEpc==mEndpointHash.constEnd())
//                            qDebug() << "xxx fr endp" << wFromPoint.x() << wFromPoint.y() << y0 << y1;
                    }
                    if (y1 > y0)
                        lines.appendVLine(xx,y0,y1);
                }
            }
            drawNetsJunction(lines,id);
            drawNetsEndpoint(lines,id);

            lines.mergeLines();

            GraphicsNet* graphicsNet = nullptr;
            switch (epl.netType()) {
            case EndpointList::MultipleDestination:
            {
                StandardArrowNet* san = new StandardArrowNet(n, lines);
                graphicsNet = san;
                int yGridPos = mGlobalInputHash.value(id,-1);
                Q_ASSERT(yGridPos >= 0);
                const EndpointCoordinate& epc = mEndpointHash.value(QPoint(mNodeBoundingBox.left(),yGridPos*2));
                san->setInputPosition(QPointF(mCoordX.value(mNodeBoundingBox.left()).lanePosition(-1),epc.lanePosition(0,true)));
            }
                break;
            case EndpointList::MultipleSource:
            {
                StandardArrowNet* san = new StandardArrowNet(n, lines);
                graphicsNet = san;
                int yGridPos = mGlobalOutputHash.value(id,-1);
                Q_ASSERT(yGridPos >= 0);
                QPoint pnt(mNodeBoundingBox.right()+1,yGridPos*2);
                const EndpointCoordinate& epc = mEndpointHash.value(pnt);
                const NetLayoutJunction* nlj = mJunctionHash.value(pnt);
                Q_ASSERT(nlj);
                san->setOutputPosition(QPointF(mCoordX.value(pnt.x()).lanePosition(nlj->rect().right()+1),
                                               epc.lanePosition(0,true)));
            }
                break;
            case EndpointList::SourceAndDestination:
                if (lines.nLines() > 0)
                    graphicsNet = new StandardGraphicsNet(n, lines);
                break;
            default:
                Q_ASSERT(0 > 1); // should never occur
                break;
            }

            if (graphicsNet)
                mScene->addGraphItem(graphicsNet);

        }
    }

    void GraphLayouter::drawNetsIsolated(u32 id, Net* n, const EndpointList &epl)
    {
        SeparatedGraphicsNet* net_item = epl.netType() == EndpointList::ConstantLevel
                ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n,QString::fromStdString(n->get_name())))
                : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));

        int ipnt = 0;
        for (const NetLayoutPoint& pnt : epl)
        {
            bool isInput = epl.isInput(ipnt++);
            auto itPnt = mEndpointHash.find(pnt);
            Q_ASSERT(itPnt != mEndpointHash.constEnd());
            if (isInput)
            {
                // gack hack : separated net might be connected to several ports
                const NodeBox* nbox = mBoxes.boxForPoint(pnt.gridPoint());
                Q_ASSERT(nbox);
                QList<u32> inpList = nbox->item()->inputNets();
                for (int jnx=0; jnx<inpList.size(); jnx++)
                {
                    u32 inpNetId = inpList.at(jnx);
                    if (inpNetId!=id) continue;
                    QPointF inpPnt(itPnt.value().xInput(),
                                   itPnt.value().lanePosition(jnx,true));
                    net_item->addInput(inpPnt);
                }
            }
            else
            {
                for (int inx : itPnt.value().outputPinIndex(id))
                {
                    QPointF outPnt(itPnt.value().xOutput(),
                                   itPnt.value().lanePosition(inx,true));
                    net_item->addOutput(outPnt);
                }
            }
        }
        net_item->finalize();
        mScene->addGraphItem(net_item);
    }

    void GraphLayouter::drawNetsEndpoint(StandardGraphicsNet::Lines &lines, u32 id)
    {
        for (auto it=mEndpointHash.constBegin(); it!=mEndpointHash.constEnd(); ++it)
        {
            const EndpointCoordinate& epc = it.value();

            QList<int> inputsById  = epc.inputPinIndex(id);
            QList<int> outputsById = epc.outputPinIndex(id);
            if (inputsById.isEmpty() && outputsById.isEmpty()) continue;

            const NetLayoutJunction* nlj = mJunctionHash.value(it.key());
            const SceneCoordinate& xScenePos = mCoordX.value(it.key().x());
            float xjLeft  = xScenePos.lanePosition(nlj->rect().left());
            float xjRight = xScenePos.lanePosition(nlj->rect().right());
            Q_ASSERT(nlj);

            for (int inpInx : inputsById)
            {
                if (xjRight >= epc.xInput())
                {
                    // don't complain if "input" is in fact global output pin
                    auto ityOut = mGlobalOutputHash.find(id);
                    if (ityOut == mGlobalOutputHash.constEnd() ||
                            QPoint(mNodeBoundingBox.right()+1, 2*ityOut.value()) != it.key())
                        qDebug() << "cannot connect input pin" << id << it.key().x() << it.key().y()/2 << xjRight << epc.xInput();
                }
                else
                    lines.appendHLine(xjRight, epc.xInput(), epc.lanePosition(inpInx,true));
            }
            for (int outInx : outputsById)
            {
                if (epc.xOutput() >= xjLeft)
                    qDebug() << "cannot connect output pin" << id << it.key().x() << it.key().y()/2 << xjLeft << epc.xOutput();
                else
                    lines.appendHLine(epc.xOutput(), xjLeft, epc.lanePosition(outInx,true));
            }
        }
    }

    void GraphLayouter::drawNetsJunction(StandardGraphicsNet::Lines& lines, u32 id)
    {
        for (auto jt = mJunctionHash.constBegin(); jt!=mJunctionHash.constEnd(); ++jt)
        {
            auto epcIt = mEndpointHash.find(jt.key());
            int x = jt.key().x();
            int y = jt.key().y();
            bool isEndpoint = (y%2 == 0);

            for (const NetLayoutJunctionWire& jw : jt.value()->netById(id).mWires)
            {
                if (jw.mHorizontal==0)
                {
                    Q_ASSERT(epcIt != mEndpointHash.constEnd() || !isEndpoint);
                    float x0 = mCoordX.value(x).lanePosition(jw.mFirst);
                    float x1 = mCoordX.value(x).lanePosition(jw.mLast);
                    float yy = isEndpoint
                            ? epcIt.value().lanePosition(jw.mRoad,true)
                            : mCoordY.value(y).lanePosition(jw.mRoad);
                    lines.appendHLine(x0,x1,yy);
                }
                else
                {
                    float y0, y1;
                    if (!isEndpoint)
                    {
                        y0 = mCoordY.value(y).lanePosition(jw.mFirst);
                        y1 = mCoordY.value(y).lanePosition(jw.mLast);
                    }
                    else if (epcIt != mEndpointHash.constEnd())
                    {
                        y0 = epcIt.value().lanePosition(jw.mFirst,true);
                        y1 = epcIt.value().lanePosition(jw.mLast,true);
                    }
                    else
                    {
                        y0 = mCoordY.value(y).junctionEntry();
                        y1 = mCoordY.value(y).junctionExit();
                        if (y1 <= y0) y1 = y0 + 1;
                    }
                    float xx = mCoordX.value(x).lanePosition(jw.mRoad);
                    lines.appendVLine(xx,y0,y1);
                }
            }
        }
    }

    void GraphLayouter::drawNets()
    {
        // ROADS AND JUNCTIONS FILLED LEFT TO RIGHT, TOP TO BOTTOM
        for (const u32 id : mContext->nets())
        {
            Net* n = gNetlist->get_net_by_id(id);
            assert(n);

            QSet<const NodeBox*> outputAssigned;

            if (n->is_unrouted())
            {
                // HANDLE GLOBAL NETS
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* src : n->get_sources())
                {
                    if (src->get_gate())
                    {
                        Node node = mContext->nodeForGate(src->get_gate()->get_id());

                        if (node.isNull())
                            continue;

                        const NodeBox* nb = mBoxes.boxForNode(node);
                        if (nb && !outputAssigned.contains(nb))
                        {
                            net_item->addOutput(nb->item()->getOutputScenePosition(n->get_id(),
                                                 QString::fromStdString(src->get_pin())));
                            outputAssigned.insert(nb);
                        }
                    }
                }

                QSet<const NodeBox*> inputAssigned;

                for (Endpoint* dst : n->get_destinations())
                {
                    if (dst->get_gate())
                    {
                        Node node = mContext->nodeForGate(dst->get_gate()->get_id());

                        if (node.isNull())
                            continue;

                        const NodeBox* nb = mBoxes.boxForNode(node);
                        if (nb && !inputAssigned.contains(nb))
                        {
                            net_item->addInput(nb->item()->getInputScenePosition(n->get_id(),
                                                QString::fromStdString(dst->get_pin())));
                            inputAssigned.insert(nb);
                        }
                    }
                }

                net_item->finalize();
                mScene->addGraphItem(net_item);
                continue;
            }

            bool use_label = isConstNet(n);

            if (use_label)
            {
                LabeledSeparatedNet* net_item = new LabeledSeparatedNet(n, QString::fromStdString(n->get_name()));

                for (Endpoint* src : n->get_sources())
                {
                    Node node = mContext->nodeForGate(src->get_gate()->get_id());

                    if (!node.isNull())
                    {
                        const NodeBox* nb = mBoxes.boxForNode(node);
                        if (nb)
                            net_item->addOutput(nb->item()->getOutputScenePosition(n->get_id(),
                                                 QString::fromStdString(src->get_pin())));
                    }
                }

                for (Endpoint* dst : n->get_destinations())
                {
                    Node node = mContext->nodeForGate(dst->get_gate()->get_id());

                    if (node.isNull())
                        continue;

                    const NodeBox* nb = mBoxes.boxForNode(node);
                    if (nb)
                        net_item->addInput(nb->item()->getInputScenePosition(n->get_id(),
                                            QString::fromStdString(dst->get_pin())));
                }

                net_item->finalize();
                mScene->addGraphItem(net_item);

                continue;
            }

            // incomplete_net: existing destination or source of net not visible
            // bool incomplete_net = false;

            bool src_found      = false;
            bool dst_found      = false;

            for (Endpoint* src : n->get_sources())
            {
                Node node = mContext->nodeForGate(src->get_gate()->get_id());

                if (!node.isNull()) src_found = true;
                 //   else incomplete_net = true;
            }

            for (Endpoint* dst : n->get_destinations())
            {
                Node node = mContext->nodeForGate(dst->get_gate()->get_id());

                if (!node.isNull()) dst_found = true;
                //    else incomplete_net = true;
            }

            if (src_found && !dst_found)
            {
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* src : n->get_sources())
                {
                    Node node = mContext->nodeForGate(src->get_gate()->get_id());

                    if (node.isNull())
                        continue;

                    const NodeBox* nb = mBoxes.boxForNode(node);
                    if (nb)
                        net_item->addOutput(nb->item()->getOutputScenePosition(n->get_id(),
                                             QString::fromStdString(src->get_pin())));
                }

                net_item->finalize();
                mScene->addGraphItem(net_item);

                continue;
            }

            if (!src_found && dst_found)
            {
                ArrowSeparatedNet* net_item = new ArrowSeparatedNet(n);

                for (Endpoint* dst : n->get_destinations())
                {
                    Node node = mContext->nodeForGate(dst->get_gate()->get_id());

                    if (node.isNull())
                        continue;

                    const NodeBox* nb = mBoxes.boxForNode(node);
                    if (nb)
                        net_item->addInput(nb->item()->getInputScenePosition(n->get_id(), QString::fromStdString(dst->get_pin())));
                }

                net_item->finalize();
                mScene->addGraphItem(net_item);

                continue;
            }

            // HANDLE NORMAL NETS
            UsedPaths used;
            StandardGraphicsNet::Lines lines;

            // FOR EVERY SRC
            for (Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                const NodeBox* src_box = nullptr;
                {
                    Node node = mContext->nodeForGate(src->get_gate()->get_id());

                    if (node.isNull())
                        continue;

                    const NodeBox* nb = mBoxes.boxForNode(node);
                    if (nb) src_box = nb;
                }
                assert(src_box);

                const QPointF src_pin_position = src_box->item()->getOutputScenePosition(n->get_id(), QString::fromStdString(src->get_pin()));

                // FOR EVERY DST
                for (Endpoint* dst : n->get_destinations())
                {
                    // FIND DST BOX
                    const NodeBox* dst_box = nullptr;

                    Node node = mContext->nodeForGate(dst->get_gate()->get_id());

                    if (node.isNull())
                        continue;

                    const NodeBox* nb = mBoxes.boxForNode(node);
                    if (nb) dst_box = nb;

                    assert(dst_box);

                    // don't attempt to loop back a module output into its input
                    // (if this triggers, we found the net because it also has
                    // destinations outside the module)
                    if (src_box == dst_box && src_box->type() == Node::Module)
                        continue;

                    QPointF dst_pin_position = dst_box->item()->getInputScenePosition(n->get_id(), QString::fromStdString(dst->get_pin()));

                    // ROAD BASED DISTANCE (x_distance - 1)
                    const int x_distance = dst_box->x() - src_box->x() - 1;
                    const int y_distance = dst_box->y() - src_box->y();

                    if (!y_distance && vRoadJumpPossible(src_box->x() + 1, dst_box->x(), src_box->y()))
                    {
                        // SPECIAL CASE INDIRECT HORIZONTAL NEIGHBORS
                        Road* dst_v_road = getVRoad(dst_box->x(), dst_box->y());

                        qreal x = sceneXForVChannelLane(dst_v_road->x, dst_v_road->mLanes);

                        lines.appendHLine(src_pin_position.x(), x, src_pin_position.y());

                        if (src_pin_position.y() < dst_pin_position.y())
                            lines.appendVLine(x, src_pin_position.y(), dst_pin_position.y());
                        else if (src_pin_position.y() > dst_pin_position.y())
                            lines.appendVLine(x, dst_pin_position.y(), src_pin_position.y());

                        lines.appendHLine(x, dst_pin_position.x(), dst_pin_position.y());

                        used.mVRoads.insert(dst_v_road);
                        continue;
                    }

                    Road* src_v_road = getVRoad(src_box->x() + 1, src_box->y());

                    if (!(x_distance || y_distance))
                    {
                        // SPECIAL CASE DIRECT HORIZONTAL NEIGHBORS
                        qreal x = sceneXForVChannelLane(src_v_road->x, src_v_road->mLanes);

                        lines.appendHLine(src_pin_position.x(), x, src_pin_position.y());

                        if (src_pin_position.y() < dst_pin_position.y())
                            lines.appendVLine(x, src_pin_position.y(), dst_pin_position.y());
                        else if (src_pin_position.y() > dst_pin_position.y())
                            lines.appendVLine(x, dst_pin_position.y(), src_pin_position.y());

                        lines.appendHLine(x, dst_pin_position.x(), dst_pin_position.y());

                        used.mVRoads.insert(src_v_road);
                        continue;
                    }

                    // NORMAL CASE
                    // CONNECT SRC TO V ROAD, TRAVEL X DISTANCE, TRAVEL Y DISTANCE, CONNECT V ROAD TO DST
                    QPointF current_position(src_pin_position);
                    current_position.setX(sceneXForVChannelLane(src_v_road->x, src_v_road->mLanes));
                    lines.appendHLine(src_pin_position.x(), current_position.x(), src_pin_position.y());
                    used.mVRoads.insert(src_v_road);

                    Junction* initial_junction = nullptr;
                    int remaining_y_distance   = y_distance;

                    if (y_distance < 0)
                    {
                        // TRAVEL UP
                        initial_junction = getJunction(src_v_road->x, src_v_road->y);

                        if (src_v_road->mLanes != initial_junction->mVLanes)
                        {
                            // R -> J
                            if (src_v_road->mLanes < initial_junction->mVLanes)
                            {
                                // POS
                                qreal y = sceneYForCloseBottomLaneChange(initial_junction->y, initial_junction->mCloseBottomLaneChanges);
                                lines.appendVLine(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.mCloseBottomJunctions.insert(initial_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = sceneYForFarBottomLaneChange(initial_junction->y, initial_junction->mFarBottomLaneChanges);
                                lines.appendVLine(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.mFarBottomJunctions.insert(initial_junction);
                            }

                            qreal x = sceneXForVChannelLane(initial_junction->x, initial_junction->mVLanes);

                            if (current_position.x() < x)
                                lines.appendHLine(current_position.x(), x, current_position.y());
                            else
                                lines.appendHLine(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }
                    }
                    else
                    {
                        // TRAVEL DOWN
                        initial_junction = getJunction(src_v_road->x, src_v_road->y + 1);

                        if (src_v_road->mLanes != initial_junction->mVLanes)
                        {
                            // R -> J
                            if (src_v_road->mLanes < initial_junction->mVLanes)
                            {
                                // POS
                                qreal y = sceneYForCloseTopLaneChange(initial_junction->y, initial_junction->mCloseTopLaneChanges);
                                lines.appendVLine(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.mCloseTopJunctions.insert(initial_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = sceneYForFarTopLaneChange(initial_junction->y, initial_junction->mFarTopLaneChanges);
                                lines.appendVLine(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.mFarTopJunctions.insert(initial_junction);
                            }

                            qreal x = sceneXForVChannelLane(initial_junction->x, initial_junction->mVLanes);

                            if (current_position.x() < x)
                                lines.appendHLine(current_position.x(), x, current_position.y());
                            else
                                lines.appendHLine(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }

                        if (!y_distance)
                            remaining_y_distance = -1;
                    }

                    used.mVJunctions.insert(initial_junction);

                    Junction* last_junction = initial_junction;

                    if (x_distance)
                    {
                        {
                            qreal y = sceneYForHChannelLane(initial_junction->y, initial_junction->mHLanes);

                            if (current_position.y() < y)
                                lines.appendVLine(current_position.x(), current_position.y(), y);
                            else
                                lines.appendVLine(current_position.x(), y, current_position.y());

                            current_position.setY(y);
                            used.mHJunctions.insert(initial_junction);
                        }

                        int remaining_x_distance = x_distance;

                        // TRAVEL REMAINING X DISTANCE
                        while (remaining_x_distance)
                        {
                            Road* r     = nullptr;
                            Junction* j = nullptr;

                            if (x_distance > 0)
                            {
                                // TRAVEL RIGHT
                                r = getHRoad(last_junction->x, last_junction->y);

                                if (last_junction->mHLanes != r->mLanes)
                                {
                                    // J -> R
                                    if (last_junction->mHLanes < r->mLanes)
                                    {
                                        // POS
                                        qreal x = sceneXForFarRightLaneChange(last_junction->x, last_junction->mFarRightLaneChanges);
                                        lines.appendHLine(current_position.x(), x, current_position.y());
                                        current_position.setX(x);
                                        used.mFarRightJunctions.insert(last_junction);
                                    }
                                    else
                                    {
                                        // NEG
                                        qreal x = sceneXForCloseRightLaneChange(last_junction->x, last_junction->mCloseRightLaneChanges);
                                        lines.appendHLine(current_position.x(), x, current_position.y());
                                        current_position.setX(x);
                                        used.mCloseRightJunctions.insert(last_junction);
                                    }

                                    qreal y = sceneYForHChannelLane(r->y, r->mLanes);

                                    if (current_position.y() < y)
                                        lines.appendVLine(current_position.x(), current_position.y(), y);
                                    else
                                        lines.appendVLine(current_position.x(), y, current_position.y());

                                    current_position.setY(y);
                                }

                                j = getJunction(last_junction->x + 1, last_junction->y);

                                if (r->mLanes != j->mHLanes)
                                {
                                    // R -> J
                                    if (r->mLanes < j->mHLanes)
                                    {
                                        // POS
                                        qreal x = sceneXForCloseLeftLaneChange(j->x, j->mCloseLeftLaneChanges);
                                        lines.appendHLine(current_position.x(), x, current_position.y());
                                        current_position.setX(x);
                                        used.mCloseLeftJunctions.insert(j);
                                    }
                                    else
                                    {
                                        // NEG
                                        qreal x = sceneXForFarLeftLaneChange(j->x, j->mFarLeftLaneChanges);
                                        lines.appendHLine(current_position.x(), x, current_position.y());
                                        current_position.setX(x);
                                        used.mFarLeftJunctions.insert(j);
                                    }

                                    qreal y = sceneYForHChannelLane(j->y, j->mHLanes);

                                    // DUPLICATE CODE ?
                                    if (current_position.y() < y)
                                        lines.appendVLine(current_position.x(), current_position.y(), y);
                                    else
                                        lines.appendVLine(current_position.x(), y, current_position.y());

                                    current_position.setY(y);
                                }

                                --remaining_x_distance;
                            }
                            else
                            {
                                // TRAVEL LEFT
                                r = getHRoad(last_junction->x - 1, last_junction->y);

                                if (last_junction->mHLanes != r->mLanes)
                                {
                                    // J -> R
                                    if (last_junction->mHLanes < r->mLanes)
                                    {
                                        // POS
                                        qreal x = sceneXForFarLeftLaneChange(last_junction->x, last_junction->mFarLeftLaneChanges);
                                        lines.appendHLine(x, current_position.x(), current_position.y());
                                        current_position.setX(x);
                                        used.mFarLeftJunctions.insert(last_junction);
                                    }
                                    else
                                    {
                                        // NEG
                                        qreal x = sceneXForCloseLeftLaneChange(last_junction->x, last_junction->mCloseLeftLaneChanges);
                                        lines.appendHLine(x, current_position.x(), current_position.y());
                                        current_position.setX(x);
                                        used.mCloseLeftJunctions.insert(last_junction);
                                    }

                                    qreal y = sceneYForHChannelLane(r->y, r->mLanes);

                                    // DUPLICATE CODE ?
                                    if (current_position.y() < y)
                                        lines.appendVLine(current_position.x(), current_position.y(), y);
                                    else
                                        lines.appendVLine(current_position.x(), y, current_position.y());

                                    current_position.setY(y);
                                }

                                j = getJunction(last_junction->x - 1, last_junction->y);

                                if (r->mLanes != j->mHLanes)
                                {
                                    // R -> J
                                    if (r->mLanes < j->mHLanes)
                                    {
                                        // POS
                                        qreal x = sceneXForCloseRightLaneChange(j->x, j->mCloseRightLaneChanges);
                                        lines.appendHLine(x, current_position.x(), current_position.y());
                                        current_position.setX(x);
                                        used.mCloseRightJunctions.insert(j);
                                    }
                                    else
                                    {
                                        // NEG
                                        qreal x = sceneXForFarRightLaneChange(j->x, j->mFarRightLaneChanges);
                                        lines.appendHLine(x, current_position.x(), current_position.y());
                                        current_position.setX(x);
                                        used.mFarRightJunctions.insert(j);
                                    }

                                    qreal y = sceneYForHChannelLane(j->y, j->mHLanes);

                                    // DUPLICATE CODE ?
                                    if (current_position.y() < y)
                                        lines.appendVLine(current_position.x(), current_position.y(), y);
                                    else
                                        lines.appendVLine(current_position.x(), y, current_position.y());

                                    current_position.setY(y);
                                }

                                ++remaining_x_distance;
                            }

                            used.mHRoads.insert(r);
                            used.mHJunctions.insert(j);

                            last_junction = j;
                        }

                        qreal x = sceneXForVChannelLane(last_junction->x, last_junction->mVLanes);

                        if (current_position.x() < x)
                            lines.appendHLine(current_position.x(), x, current_position.y());
                        else
                            lines.appendHLine(x, current_position.x(), current_position.y());

                        current_position.setX(x);
                        used.mVJunctions.insert(last_junction);
                    }

                    // TRAVEL REMAINING Y DISTANCE
                    if (remaining_y_distance > 0)
                    {
                        while (remaining_y_distance != 1)
                        {
                            // TRAVEL DOWN
                            Road* r = getVRoad(last_junction->x, last_junction->y);

                            if (last_junction->mVLanes != r->mLanes)
                            {
                                // J -> R
                                if (last_junction->mVLanes < r->mLanes)
                                {
                                    // POS
                                    qreal y = sceneYForFarBottomLaneChange(last_junction->y, last_junction->mFarBottomLaneChanges);
                                    lines.appendVLine(current_position.x(), current_position.y(), y);
                                    current_position.setY(y);
                                    used.mFarBottomJunctions.insert(last_junction);
                                }
                                else
                                {
                                    // NEG
                                    qreal y = sceneYForCloseBottomLaneChange(last_junction->y, last_junction->mCloseBottomLaneChanges);
                                    lines.appendVLine(current_position.x(), current_position.y(), y);
                                    current_position.setY(y);
                                    used.mCloseBottomJunctions.insert(last_junction);
                                }

                                qreal x = sceneXForVChannelLane(r->x, r->mLanes);

                                if (current_position.x() < x)
                                    lines.appendHLine(current_position.x(), x, current_position.y());
                                else
                                    lines.appendHLine(x, current_position.x(), current_position.y());

                                current_position.setX(x);
                            }

                            Junction* j = getJunction(last_junction->x, last_junction->y + 1);

                            if (r->mLanes != j->mVLanes)
                            {
                                // R -> J
                                if (r->mLanes < j->mVLanes)
                                {
                                    // POS
                                    qreal y = sceneYForCloseTopLaneChange(j->y, j->mCloseTopLaneChanges);
                                    lines.appendVLine(current_position.x(), current_position.y(), y);
                                    current_position.setY(y);
                                    used.mCloseTopJunctions.insert(j);
                                }
                                else
                                {
                                    // NEG
                                    qreal y = sceneYForFarTopLaneChange(j->y, j->mFarTopLaneChanges);
                                    lines.appendVLine(current_position.x(), current_position.y(), y);
                                    current_position.setY(y);
                                    used.mFarTopJunctions.insert(j);
                                }

                                qreal x = sceneXForVChannelLane(j->x, j->mVLanes);

                                if (current_position.x() < x)
                                    lines.appendHLine(current_position.x(), x, current_position.y());
                                else
                                    lines.appendHLine(x, current_position.x(), current_position.y());

                                current_position.setX(x);
                            }

                            used.mVRoads.insert(r);
                            used.mVJunctions.insert(j);

                            last_junction = j;

                            --remaining_y_distance;
                        }
                    }
                    else
                    {
                        while (remaining_y_distance != -1)
                        {
                            // TRAVEL UP
                            Road* r = getVRoad(last_junction->x, last_junction->y - 1);

                            if (last_junction->mVLanes != r->mLanes)
                            {
                                // J -> R
                                if (last_junction->mVLanes < r->mLanes)
                                {
                                    // POS
                                    qreal y = sceneYForFarTopLaneChange(last_junction->y, last_junction->mFarTopLaneChanges);
                                    lines.appendVLine(current_position.x(), y, current_position.y());
                                    current_position.setY(y);
                                    used.mFarTopJunctions.insert(last_junction);
                                }
                                else
                                {
                                    // NEG
                                    qreal y = sceneYForCloseTopLaneChange(last_junction->y, last_junction->mCloseTopLaneChanges);
                                    lines.appendVLine(current_position.x(), y, current_position.y());
                                    current_position.setY(y);
                                    used.mCloseTopJunctions.insert(last_junction);
                                }

                                qreal x = sceneXForVChannelLane(r->x, r->mLanes);

                                if (current_position.x() < x)
                                    lines.appendHLine(current_position.x(), x, current_position.y());
                                else
                                    lines.appendHLine(x, current_position.x(), current_position.y());

                                current_position.setX(x);
                            }

                            Junction* j = getJunction(last_junction->x, last_junction->y - 1);

                            if (r->mLanes != j->mVLanes)
                            {
                                // R -> J
                                if (r->mLanes < j->mVLanes)
                                {
                                    // POS
                                    qreal y = sceneYForCloseBottomLaneChange(j->y, j->mCloseBottomLaneChanges);
                                    lines.appendVLine(current_position.x(), y, current_position.y());
                                    current_position.setY(y);
                                    used.mCloseBottomJunctions.insert(j);
                                }
                                else
                                {
                                    // NEG
                                    qreal y = sceneYForFarBottomLaneChange(j->y, j->mFarBottomLaneChanges);
                                    lines.appendVLine(current_position.x(), y, current_position.y());
                                    current_position.setY(y);
                                    used.mFarBottomJunctions.insert(j);
                                }

                                qreal x = sceneXForVChannelLane(j->x, j->mVLanes);

                                if (current_position.x() < x)
                                    lines.appendHLine(current_position.x(), x, current_position.y());
                                else
                                    lines.appendHLine(x, current_position.x(), current_position.y());

                                current_position.setX(x);
                            }

                            used.mVRoads.insert(r);
                            used.mVJunctions.insert(j);

                            last_junction = j;

                            ++remaining_y_distance;
                        }
                    }

                    Road* dst_road = nullptr;

                    if (y_distance > 0)
                    {
                        // TRAVEL DOWN
                        dst_road = getVRoad(last_junction->x, last_junction->y);

                        if (last_junction->mVLanes != dst_road->mLanes)
                        {
                            // J -> R
                            if (last_junction->mVLanes < dst_road->mLanes)
                            {
                                // POS
                                qreal y = sceneYForFarBottomLaneChange(last_junction->y, last_junction->mFarBottomLaneChanges);
                                lines.appendVLine(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.mFarBottomJunctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = sceneYForCloseBottomLaneChange(last_junction->y, last_junction->mCloseBottomLaneChanges);
                                lines.appendVLine(current_position.x(), current_position.y(), y);
                                current_position.setY(y);
                                used.mCloseBottomJunctions.insert(last_junction);
                            }

                            qreal x = sceneXForVChannelLane(dst_road->x, dst_road->mLanes);

                            if (current_position.x() < x)
                                lines.appendHLine(current_position.x(), x, current_position.y());
                            else
                                lines.appendHLine(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }
                    }
                    else
                    {
                        // TRAVEL UP
                        dst_road = getVRoad(last_junction->x, last_junction->y - 1);

                        if (last_junction->mVLanes != dst_road->mLanes)
                        {
                            // J -> R
                            if (last_junction->mVLanes < dst_road->mLanes)
                            {
                                // POS
                                qreal y = sceneYForFarTopLaneChange(last_junction->y, last_junction->mFarTopLaneChanges);
                                lines.appendVLine(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.mFarTopJunctions.insert(last_junction);
                            }
                            else
                            {
                                // NEG
                                qreal y = sceneYForCloseTopLaneChange(last_junction->y, last_junction->mCloseTopLaneChanges);
                                lines.appendVLine(current_position.x(), y, current_position.y());
                                current_position.setY(y);
                                used.mCloseTopJunctions.insert(last_junction);
                            }

                            qreal x = sceneXForVChannelLane(dst_road->x, dst_road->mLanes);

                            if (current_position.x() < x)
                                lines.appendHLine(current_position.x(), x, current_position.y());
                            else
                                lines.appendHLine(x, current_position.x(), current_position.y());

                            current_position.setX(x);
                        }
                    }

                    used.mVJunctions.insert(last_junction);

                    if (current_position.y() < dst_pin_position.y())
                        lines.appendVLine(current_position.x(), current_position.y(), dst_pin_position.y());
                    else
                        lines.appendVLine(current_position.x(), dst_pin_position.y(), current_position.y());

                    current_position.setY(dst_pin_position.y());

                    used.mVRoads.insert(dst_road);

                    lines.appendHLine(current_position.x(), dst_pin_position.x(), current_position.y());

                    current_position = src_pin_position;
                }
            }

            lines.mergeLines();
            if (lines.nLines() > 0)
            {
                StandardGraphicsNet* GraphicsNet = new StandardGraphicsNet(n, lines);
                mScene->addGraphItem(GraphicsNet);
            }
            commitUsedPaths(used);
        }
    }

    void GraphLayouter::updateSceneRect()
    {
        // SCENE RECT STUFF BEHAVES WEIRDLY, FURTHER RESEARCH REQUIRED
        //QRectF rect = mScene->sceneRect();

        QRectF rect(mScene->itemsBoundingRect());
        rect.adjust(-200, -200, 200, 200);
        mScene->setSceneRect(rect);
    }

    bool GraphLayouter::boxExists(const int x, const int y) const
    {
        return mBoxes.boxForPoint(QPoint(x,y)) != nullptr;
    }

    bool GraphLayouter::hRoadJumpPossible(const int x, const int y1, const int y2) const
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
            if (boxExists(x, bottom_y - difference))
                return false;

            --difference;
        }

        return true;
    }

    bool GraphLayouter::hRoadJumpPossible(const GraphLayouter::Road* const r1, const GraphLayouter::Road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->x != r2->x);

        return hRoadJumpPossible(r1->x, r1->y, r2->y);
    }

    bool GraphLayouter::vRoadJumpPossible(const int x1, const int x2, const int y) const
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
            if (boxExists(right_x - difference, y))
                return false;

            --difference;
        }

        return true;
    }

    bool GraphLayouter::vRoadJumpPossible(const GraphLayouter::Road* const r1, const GraphLayouter::Road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->y != r2->y);

        return vRoadJumpPossible(r1->x, r2->x, r1->y);
    }

    GraphLayouter::Road* GraphLayouter::getHRoad(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = mHRoads.find(p);
        if (it != mHRoads.end())
            return it.value();

        GraphLayouter::Road* r = new Road(x, y);
        mHRoads.insert(p,r);
        return r;
    }

    GraphLayouter::Road* GraphLayouter::getVRoad(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = mVRoads.find(p);
        if (it != mVRoads.end())
            return it.value();

        GraphLayouter::Road* r = new Road(x, y);
        mVRoads.insert(p,r);
        return r;
    }

    GraphLayouter::Junction* GraphLayouter::getJunction(const int x, const int y)
    {
        QPoint p(x,y);
        auto it = mJunctions.find(p);
        if (it != mJunctions.end())
        {
            return it.value();
        }

        GraphLayouter::Junction* j = new Junction(x, y);
        mJunctions.insert(p,j);
        return j;
    }

    qreal GraphLayouter::hRoadHeight(const unsigned int mLanes) const
    {
        // LANES COUNTED FROM 1
        qreal height = sHRoadPadding * 2;

        if (mLanes > 1)
            height += (mLanes - 1) * sLaneSpacing;

        return height;
    }

    qreal GraphLayouter::vRoadWidth(const unsigned int mLanes) const
    {
        // LANES COUNTED FROM 1
        qreal width = sVRoadPadding * 2;

        if (mLanes > 1)
            width += (mLanes - 1) * sLaneSpacing;

        return width;
    }

    qreal GraphLayouter::sceneYForHChannelLane(const int y, const unsigned int lane) const
    {
        // LINES NUMBERED FROM 0
        assert(mNodeOffsetForY.contains(y) || mNodeOffsetForY.contains(y - 1));

        const qreal offset = lane * sLaneSpacing;

        if (y == 0)
            return mNodeOffsetForY.value(y) - mMaxHChannelHeightForY.value(y) + mMaxHChannelTopSpacingForY.value(y) + offset;
        else
            return mNodeOffsetForY.value(y - 1) + mMaxNodeHeightForY.value(y - 1) + mMaxHChannelTopSpacingForY.value(y) + offset;
    }

    qreal GraphLayouter::sceneXForVChannelLane(const int x, const unsigned int lane) const
    {
        // LINES NUMBERED FROM 0
        assert(mNodeOffsetForX.contains(x) || mNodeOffsetForX.contains(x - 1));

        const qreal offset = lane * sLaneSpacing;

        if (mNodeOffsetForX.contains(x))
            return mNodeOffsetForX.value(x) - mMaxVChannelWidthForX.value(x) + mMaxVChannelLeftSpacingForX.value(x) + offset;
        else
            return mNodeOffsetForX.value(x - 1) + mMaxNodeWidthForX.value(x - 1) + mMaxVChannelLeftSpacingForX.value(x) + offset;
    }

    qreal GraphLayouter::sceneXForCloseLeftLaneChange(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(mNodeOffsetForX.contains(channel_x) || mNodeOffsetForX.contains(channel_x - 1));

        if (mNodeOffsetForX.contains(channel_x))
            return mNodeOffsetForX.value(channel_x) - mMaxVChannelWidthForX.value(channel_x) + mMaxVChannelLeftSpacingForX.value(channel_x) - sJunctionPadding
                   - lane_change * sLaneSpacing;
        else
            return mNodeOffsetForX.value(channel_x - 1) + mMaxNodeWidthForX.value(channel_x - 1) + mMaxVChannelLeftSpacingForX.value(channel_x) - sJunctionPadding
                   - lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneXForFarLeftLaneChange(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(mNodeOffsetForX.contains(channel_x) || mNodeOffsetForX.contains(channel_x - 1));

        if (mNodeOffsetForX.contains(channel_x))
            return mNodeOffsetForX.value(channel_x) - mMaxVChannelWidthForX.value(channel_x) + mMaxVChannelLeftSpacingForX.value(channel_x)
                   - mMaxLeftJunctionSpacingForX.value(channel_x) + lane_change * sLaneSpacing;
        else
            return mNodeOffsetForX.value(channel_x - 1) + mMaxNodeWidthForX.value(channel_x - 1) + mMaxVChannelLeftSpacingForX.value(channel_x)
                   - mMaxLeftJunctionSpacingForX.value(channel_x) + lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneXForCloseRightLaneChange(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(mNodeOffsetForX.contains(channel_x) || mNodeOffsetForX.contains(channel_x - 1));

        if (mNodeOffsetForX.contains(channel_x))
            return mNodeOffsetForX.value(channel_x) - mMaxVChannelRightSpacingForX.value(channel_x) + sJunctionPadding + lane_change * sLaneSpacing;
        else
            return mNodeOffsetForX.value(channel_x - 1) + mMaxNodeWidthForX.value(channel_x - 1) + mMaxVChannelWidthForX.value(channel_x)
                   - mMaxVChannelRightSpacingForX.value(channel_x) + sJunctionPadding + lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneXForFarRightLaneChange(const int channel_x, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        assert(mNodeOffsetForX.contains(channel_x) || mNodeOffsetForX.contains(channel_x - 1));

        if (mNodeOffsetForX.contains(channel_x))
            return mNodeOffsetForX.value(channel_x) - mMaxVChannelRightSpacingForX.value(channel_x) + mMaxRightJunctionSpacingForX.value(channel_x) - lane_change * sLaneSpacing;
        else
            return mNodeOffsetForX.value(channel_x - 1) + mMaxNodeWidthForX.value(channel_x - 1) + mMaxVChannelWidthForX.value(channel_x)
                   - mMaxVChannelRightSpacingForX.value(channel_x) + mMaxRightJunctionSpacingForX.value(channel_x) - lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneYForCloseTopLaneChange(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return mNodeOffsetForY.value(channel_y) - mMaxHChannelHeightForY.value(channel_y) + mMaxHChannelTopSpacingForY.value(channel_y) - sJunctionPadding
                   - lane_change * sLaneSpacing;
        else
            return mNodeOffsetForY.value(channel_y - 1) + mMaxNodeHeightForY.value(channel_y - 1) + mMaxHChannelTopSpacingForY.value(channel_y) - sJunctionPadding
                   - lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneYForFarTopLaneChange(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return mNodeOffsetForY.value(channel_y) - mMaxHChannelHeightForY.value(channel_y) + mMaxHChannelTopSpacingForY.value(channel_y)
                   - mMaxTopJunctionSpacingForY.value(channel_y) + lane_change * sLaneSpacing;
        else
            return mNodeOffsetForY.value(channel_y - 1) + mMaxNodeHeightForY.value(channel_y - 1) + mMaxHChannelTopSpacingForY.value(channel_y)
                   - mMaxTopJunctionSpacingForY.value(channel_y) + lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneYForCloseBottomLaneChange(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return mNodeOffsetForY.value(channel_y) - mMaxHChannelBottomSpacingForY.value(channel_y) + sJunctionPadding + lane_change * sLaneSpacing;
        else
            return mNodeOffsetForY.value(channel_y - 1) + mMaxNodeHeightForY.value(channel_y - 1) + mMaxHChannelHeightForY.value(channel_y)
                   - mMaxHChannelBottomSpacingForY.value(channel_y) + sJunctionPadding + lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneYForFarBottomLaneChange(const int channel_y, unsigned int lane_change) const
    {
        // LANE CHANGES COUNTED FROM 0
        if (channel_y == 0)
            return mNodeOffsetForY.value(channel_y) - mMaxHChannelBottomSpacingForY.value(channel_y) + mMaxBottomJunctionSpacingForY.value(channel_y) - lane_change * sLaneSpacing;
        else
            return mNodeOffsetForY.value(channel_y - 1) + mMaxNodeHeightForY.value(channel_y - 1) + mMaxHChannelHeightForY.value(channel_y)
                   - mMaxHChannelBottomSpacingForY.value(channel_y) + mMaxBottomJunctionSpacingForY.value(channel_y) - lane_change * sLaneSpacing;
    }

    qreal GraphLayouter::sceneXForCloseLeftLaneChange(const Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneXForCloseLeftLaneChange(j->x, j->mCloseLeftLaneChanges);
    }

    qreal GraphLayouter::sceneXForFarLeftLaneChange(const GraphLayouter::Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneXForFarLeftLaneChange(j->x, j->mFarLeftLaneChanges);
    }

    qreal GraphLayouter::sceneXForCloseRightLaneChange(const Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneXForCloseRightLaneChange(j->x, j->mCloseRightLaneChanges);
    }

    qreal GraphLayouter::sceneXForFarRightLaneChange(const GraphLayouter::Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneXForFarRightLaneChange(j->x, j->mFarRightLaneChanges);
    }

    qreal GraphLayouter::sceneYForCloseTopLaneChange(const Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneYForCloseTopLaneChange(j->y, j->mCloseTopLaneChanges);
    }

    qreal GraphLayouter::sceneYForFarTopLaneChange(const GraphLayouter::Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneYForFarTopLaneChange(j->y, j->mFarTopLaneChanges);
    }

    qreal GraphLayouter::sceneYForCloseBottomLaneChange(const Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneYForCloseBottomLaneChange(j->y, j->mCloseBottomLaneChanges);
    }

    qreal GraphLayouter::sceneYForFarBottomLaneChange(const GraphLayouter::Junction* const j) const
    {
        // CONVENIENCE METHOD
        assert(j);

        return sceneYForFarBottomLaneChange(j->y, j->mFarBottomLaneChanges);
    }

    void GraphLayouter::commitUsedPaths(const UsedPaths &used)
    {
        for (Road* r : used.mHRoads)
            r->mLanes += 1;

        for (Road* r : used.mVRoads)
            r->mLanes += 1;

        for (Junction* j : used.mHJunctions)
            j->mHLanes += 1;

        for (Junction* j : used.mVJunctions)
            j->mVLanes += 1;

        for (Junction* j : used.mCloseLeftJunctions)
            j->mCloseLeftLaneChanges += 1;

        for (Junction* j : used.mCloseRightJunctions)
            j->mCloseRightLaneChanges += 1;

        for (Junction* j : used.mCloseTopJunctions)
            j->mCloseTopLaneChanges += 1;

        for (Junction* j : used.mCloseBottomJunctions)
            j->mCloseBottomLaneChanges += 1;

        for (Junction* j : used.mFarLeftJunctions)
            j->mFarLeftLaneChanges += 1;

        for (Junction* j : used.mFarRightJunctions)
            j->mFarRightLaneChanges += 1;

        for (Junction* j : used.mFarTopJunctions)
            j->mFarTopLaneChanges += 1;

        for (Junction* j : used.mFarBottomJunctions)
            j->mFarBottomLaneChanges += 1;
    }

    void GraphLayouter::SceneCoordinate::testMinMax(int ilane)
    {
        if (ilane  < minLane) minLane = ilane;
        if (ilane+1> maxLane) maxLane = ilane+1;
    }

    void GraphLayouter::SceneCoordinate::setOffsetX(const SceneCoordinate& previous, float maximumBlock, float sepOut, float sepInp)
    {
        float delta =  maximumBlock;
        if (delta < sepOut) delta = sepOut;
        mOffset = previous.xBoxOffset() + (1 - minLane) * sLaneSpacing  + delta;
        float xDefaultBoxPadding = maxLane * sLaneSpacing;
        if (xDefaultBoxPadding < sepInp)
            mPadding = sepInp - xDefaultBoxPadding;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYje(const SceneCoordinate& previous, float minimumJunction)
    {
        float delta = (previous.maxLane) * sLaneSpacing + sVRoadPadding;
        if (delta < minimumJunction) delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYej(const SceneCoordinate& previous, float maximumBlock, float minimumJunction)
    {
        float delta = (-minLane - 1) * sLaneSpacing + maximumBlock + sVRoadPadding;
        if (delta < minimumJunction) delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    float GraphLayouter::SceneCoordinate::lanePosition(int ilane) const
    {
        return mOffset + ilane * sLaneSpacing;
    }

    float GraphLayouter::SceneCoordinate::xBoxOffset() const
    {
        return junctionExit() + sHRoadPadding + mPadding;
    }

    float GraphLayouter::EndpointCoordinate::lanePosition(int ilane, bool absolute) const
    {
        float y0 = absolute ? mYoffset : mTopPin;
        if (ilane < 0) return y0 + ilane * sLaneSpacing;
        int n = numberPins() - 1;
        if (ilane <= n) return y0 + ilane * mPinDistance;
        return y0 + n * mPinDistance + (ilane-n) * sLaneSpacing;
    }

    GraphLayouter::EndpointCoordinate::EndpointCoordinate()
        : mYoffset(0), mXoutput(0), mXinput(0), mPinDistance(0), mTopPin(0), mNumberPins(0)
    {;}

    int GraphLayouter::EndpointCoordinate::numberPins() const
    {
        return mNumberPins;
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

    QList<int> GraphLayouter::EndpointCoordinate::inputPinIndex(u32 id) const
    {
        return mInputHash.values(id);
    }

    QList<int> GraphLayouter::EndpointCoordinate::outputPinIndex(u32 id) const
    {
        return mOutputHash.values(id);
    }

    void GraphLayouter::EndpointCoordinate::setInputPins(const QList<u32> &pinList, float p0dist, float pdist)
    {
        int n = pinList.size();
        if (n > mNumberPins) mNumberPins = n;
        for (int i=0; i<n; i++)
        {
            u32 id = pinList.at(i);
            if (id) mInputHash.insert(id,i);
        }
        if (p0dist > mTopPin) mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointCoordinate::setOutputPins(const QList<u32>& pinList, float p0dist, float pdist)
    {
        int n = pinList.size();
        if (n > mNumberPins) mNumberPins = n;
        for (int i=0; i<n; i++)
        {
            u32 id = pinList.at(i);
            if (id) mOutputHash.insert(id,i);
        }
        if (p0dist > mTopPin) mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointList::addSource(const NetLayoutPoint &pnt)
    {
        mNetType = static_cast<EndpointType>(mNetType | SingleSource);
        int existingIndex = indexOf(pnt);
        if (existingIndex >= 0 && !mPointIsInput.at(existingIndex)) return;
        append(pnt);
        mPointIsInput.append(false);
    }

    void GraphLayouter::EndpointList::addDestination(const NetLayoutPoint &pnt)
    {
        mNetType = static_cast<EndpointType>(mNetType | SingleDestination);
        int existingIndex = indexOf(pnt);
        if (existingIndex >= 0 && mPointIsInput.at(existingIndex)) return;
        append(pnt);
        mPointIsInput.append(true);
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

    bool GraphLayouter::optimizeNetLayoutEnabled()
    {
        return mOptimizeNetLayout;
    }

    void GraphLayouter::setOptimizeNetLayoutEnabled(bool enabled)
    {
        mOptimizeNetLayout = enabled;
    }
}
